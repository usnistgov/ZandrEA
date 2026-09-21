#!/usr/bin/env python3
"""Push generic CSV samples to the ZandrEA REST server.

The driver reads CSV rows, maps CSV columns to ZandrEA subject point names,
validates the mapping against ``GET /contracts``, and uploads each row through
``PUT /ctrl/sampletimestep-named``.

CSV formatting mode 1: encoded column names

    Use one CSV column per subject point. Each sample column header must be
    encoded as ``subject:point`` where ``subject`` is the configured subject
    name from ``/contracts`` and ``point`` is one of the point names in that
    subject's contract.

    Example header:

        date,time,VAV-1:Temperature_air_zone,VAV-1:FlowRateVolume_air_vav

CSV formatting mode 2: mapping file

    Pass ``--mapping-file mapping.json``. The mapping file must be JSON with
    subject names at the top level, point names under each subject, and CSV
    column names as values:

        {
          "VAV-1": {
            "Temperature_air_zone": "zone_temp",
            "FlowRateVolume_air_vav": "airflow"
          }
        }

    In this mode CSV headers can be any source-specific names, as long as the
    mapping file points each required ZandrEA point to the correct CSV column.

Timestamps:

    By default each row must include ``date`` and ``time`` columns. Use
    ``--time now`` or ``--time TIMESTAMP`` to override the first timestamp and
    ``--timestep`` to increment subsequent rows.

Examples:

    uv run --with requests --with python-dateutil python \\
      EAd/tests/ead-push-csv-generic.py samples.csv --dry-run

    uv run --with requests --with python-dateutil python \\
      EAd/tests/ead-push-csv-generic.py samples.csv --mapping-file mapping.json
"""

import argparse
import csv
import json
import os
import sys
import time
from contextlib import nullcontext
from datetime import datetime


METADATA_COLUMNS = {"date", "time", "gtc"}


def parse_args():
    cli = argparse.ArgumentParser(
        description="Push generic CSV samples to the EA REST server.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    cli.add_argument(
        "filenames", help="Name of the CSV file(s) to load (- for stdin)", nargs="+"
    )
    cli.add_argument(
        "-m",
        "--mapping-file",
        help=(
            "JSON mapping file shaped as "
            "{subject: {point: csv_column_name}}. If omitted, CSV headers must "
            "be encoded as subject:point."
        ),
    )
    cli.add_argument(
        "--column-separator",
        default=":",
        help="Separator for encoded subject:point CSV headers (default ':')",
    )
    cli.add_argument("-l", "--loop", help="Repeat forever", action="store_true")
    cli.add_argument(
        "-i", "--interval", help="Time to pause between samples", type=int, default=0
    )
    cli.add_argument(
        "-t",
        "--time",
        help="Starting timestamp override; use 'now' for current time",
    )
    cli.add_argument(
        "-s",
        "--timestep",
        help="Seconds to increment timestamp between samples when --time is used",
        type=int,
        default=60,
    )
    cli.add_argument("-u", "--baseurl", help="Base URL to use for REST API")
    cli.add_argument(
        "-p",
        "--port",
        help="Port number to use for REST API URL (EA_PORT)",
        type=int,
        default=int(os.getenv("EA_PORT", "9876")),
    )
    cli.add_argument(
        "-H",
        "--host",
        help="Hostname to use for REST API URL (EA_HOST)",
        default=os.getenv("EA_HOST", "127.0.0.1"),
    )
    cli.add_argument(
        "-P",
        "--protocol",
        help="Protocol to use for REST API URL (EA_PROTO)",
        default=os.getenv("EA_PROTO", "http"),
    )
    cli.add_argument(
        "-A",
        "--api-path",
        help="Path prefix to use for REST API URL (EA_APIPATH)",
        default=os.getenv("EA_APIPATH", ""),
    )
    cli.add_argument(
        "--dry-run",
        action="store_true",
        help="Validate through /contracts and print payloads without uploading samples",
    )
    cli.add_argument(
        "--max-rows",
        type=int,
        help="Stop after this many CSV data rows per file",
    )
    return cli.parse_args()


def base_url(args):
    if args.baseurl:
        return args.baseurl.rstrip("/")
    apipath = args.api_path.strip().rstrip("/")
    default = f"{args.protocol}://{args.host}:{args.port}{apipath}"
    return os.getenv("EA_BASEURL", default).rstrip("/")


class RestClient:
    def __init__(self, baseurl):
        self.baseurl = baseurl
        self.headers = {"Content-Type": "application/json"}

    def request(self, method, path, **kwargs):
        import requests

        response = requests.request(
            method, f"{self.baseurl}{path}", headers=self.headers, **kwargs
        )
        if not 200 <= response.status_code <= 299:
            raise RuntimeError(
                f"{method} {path} failed with HTTP {response.status_code}: {response.text}"
            )
        return response

    def get_contracts(self):
        return self.request("GET", "/contracts").json()

    def sample_time_step(self, timestamp, values_by_subject):
        body = {"time": timestamp, "values_by_subject": values_by_subject}
        return self.request("PUT", "/ctrl/sampletimestep-named", json=body).json()

    def new_alerts_then_clear(self):
        return self.request("GET", "/alerts").json().get("alerts", [])

    def case_keys(self, subject):
        return (
            self.request("GET", "/casekeys", json={"subject": subject})
            .json()
            .get("casekeys", [])
        )

    def case(self, case_key, subject):
        return self.request(
            "GET", "/case", json={"key": case_key, "subject": subject}
        ).json()


def load_mapping_file(filename):
    if filename is None:
        return None
    with open(filename) as mapping_file:
        mapping = json.load(mapping_file)
    if not isinstance(mapping, dict):
        raise ValueError("mapping file must contain an object")
    for subject, point_map in mapping.items():
        if not isinstance(point_map, dict):
            raise ValueError(f"mapping for subject {subject!r} must be an object")
        for point, column in point_map.items():
            if not isinstance(point, str) or not isinstance(column, str):
                raise ValueError(
                    f"mapping for subject {subject!r} must map point names to CSV column names"
                )
    return mapping


def discover_subjects_from_contracts(contract_doc):
    subjects = {}
    contracts_by_id = {
        contract.get("id"): contract for contract in contract_doc.get("contracts", [])
    }
    for subject in contract_doc.get("subjects", []):
        contract_id = subject.get("contract")
        contract = contracts_by_id.get(contract_id)
        if contract is None:
            print(
                f"WARNING: subject {subject.get('key')} ({subject.get('name')}) "
                f"uses unsupported contract {contract_id}"
            )
            continue
        name = subject.get("name")
        if name:
            subjects[name] = {"subject": subject, "contract": contract}
        print(
            f"Subject {subject.get('key')} ({subject.get('name')}) uses contract {contract_id}"
        )
    if not subjects:
        raise RuntimeError("/contracts did not provide any subjects")
    return subjects


def build_mapping_from_encoded_headers(fieldnames, subjects_by_name, separator):
    mapping = {}
    for column in fieldnames:
        if column in METADATA_COLUMNS:
            continue
        if separator not in column:
            raise ValueError(
                f"CSV column {column!r} is not metadata and is not encoded as subject{separator}point"
            )
        subject_name, point = column.split(separator, 1)
        if not subject_name or not point:
            raise ValueError(
                f"CSV column {column!r} must be encoded as subject{separator}point"
            )
        mapping.setdefault(subject_name, {})[point] = column
    return mapping


def build_column_map(fieldnames, subjects_by_name, mapping):
    if not fieldnames:
        raise ValueError("CSV file has no header")
    fieldnames = set(fieldnames)

    column_map = {}
    for subject_name, point_map in mapping.items():
        subject_contract = subjects_by_name.get(subject_name)
        if subject_contract is None:
            raise ValueError(
                f"mapping references subject {subject_name!r}, "
                "but /contracts does not list that subject"
            )

        subject = subject_contract["subject"]
        contract = subject_contract["contract"]
        expected_points = set(contract.get("points", []))
        mapped_points = set(point_map)

        unknown_points = sorted(mapped_points - expected_points)
        if unknown_points:
            raise ValueError(
                f"mapping for subject {subject_name!r} references points not listed "
                f"in /contracts: {', '.join(unknown_points)}"
            )

        missing_points = sorted(expected_points - mapped_points)
        if missing_points:
            raise ValueError(
                f"mapping for subject {subject_name!r} is missing required points "
                f"from /contracts: {', '.join(missing_points)}"
            )

        for point, column in point_map.items():
            if column not in fieldnames:
                raise ValueError(
                    f"mapping for subject {subject_name!r} point {point!r} uses "
                    f"CSV column {column!r}, but that column is not in the file"
                )
            if column in column_map:
                previous = column_map[column]
                raise ValueError(
                    f"CSV column {column!r} is mapped more than once "
                    f"({previous['subject']['name']}:{previous['point']} and "
                    f"{subject_name}:{point})"
                )
            column_map[column] = {"subject": subject, "point": point}

    if not column_map:
        raise ValueError("CSV mapping did not produce any sample columns")

    return column_map


def row_timestamp(row, override_timestamp):
    if override_timestamp is not None:
        return override_timestamp
    return parse_timestamp(f"{row['date']} {row['time']}")


def parse_timestamp(value):
    value = value.strip()
    for fmt in (
        "%Y/%m/%d %H:%M",
        "%Y/%m/%d %H:%M:%S",
        "%Y-%m-%d %H:%M",
        "%Y-%m-%d %H:%M:%S",
        "%Y-%m-%dT%H:%M",
        "%Y-%m-%dT%H:%M:%S",
    ):
        try:
            return int(time.mktime(datetime.strptime(value, fmt).timetuple()))
        except ValueError:
            pass
    try:
        from dateutil.parser import parse
    except ImportError as err:
        raise ValueError(
            f"could not parse timestamp {value!r}; install python-dateutil for "
            "non-standard timestamp formats"
        ) from err
    return int(time.mktime(parse(value).timetuple()))


def row_values_by_subject(row, column_map):
    values_by_subject_key = {}
    for column, mapping in column_map.items():
        raw = row.get(column)
        if raw is None or raw.strip() == "":
            raise ValueError(f"missing value in column {column}")
        subject = mapping["subject"]
        subject_key = subject["key"]
        values_by_subject_key.setdefault(
            subject_key, {"subject": subject, "values": {}}
        )
        values_by_subject_key[subject_key]["values"][mapping["point"]] = float(raw)

    values_by_subject = []
    for subject_key in sorted(values_by_subject_key):
        subject_values = values_by_subject_key[subject_key]
        values_by_subject.append(
            {"subject": subject_key, "values": subject_values["values"]}
        )
    return values_by_subject


def open_csv(filename):
    if filename == "-":
        return nullcontext(sys.stdin)
    if not os.access(filename, os.R_OK):
        raise FileNotFoundError(f"datafile {filename!r} is not readable")
    return open(filename, newline="")


def read_csv_rows(filename):
    with open_csv(filename) as csvfile:
        reader = csv.DictReader(csvfile)
        return reader.fieldnames, list(reader)


def mapping_for_file(fieldnames, args, subjects_by_name, file_mapping):
    if file_mapping is not None:
        return file_mapping
    return build_mapping_from_encoded_headers(
        fieldnames, subjects_by_name, args.column_separator
    )


def run_rows(filename, rows, column_map, args, client, override_timestamp):
    rows_processed = 0
    for linenum, row in enumerate(rows, start=2):
        if args.max_rows is not None and rows_processed >= args.max_rows:
            break

        ts = row_timestamp(row, override_timestamp)
        try:
            values_by_subject = row_values_by_subject(row, column_map)
        except ValueError as err:
            raise ValueError(f"{filename}:{linenum}: {err}") from err

        if args.dry_run:
            print(json.dumps({"time": ts, "values_by_subject": values_by_subject}))
        else:
            client.sample_time_step(ts, values_by_subject)

        rows_processed += 1
        print(f"Single step {rows_processed} taken for timestamp {ts}")

        if override_timestamp is not None:
            override_timestamp += args.timestep

        if args.interval > 0:
            time.sleep(args.interval)

        if not args.dry_run:
            for alert in client.new_alerts_then_clear():
                print(alert.get("message", alert))

    return override_timestamp


def report_cases(client, subjectkeys):
    for subject in subjectkeys:
        for case_key in client.case_keys(subject):
            print("")
            case = client.case(case_key, subject)
            if case.get("error") is None:
                print(
                    "WARNING: case {} from subject {}: {}: {}".format(
                        case_key, subject, case.get("label"), case.get("report")
                    )
                )
            else:
                print(f"ERROR: case {case_key} from subject {subject}: SayCase failed")


def main():
    args = parse_args()
    client = RestClient(base_url(args))
    file_mapping = load_mapping_file(args.mapping_file)

    contract_doc = client.get_contracts()
    subjectkeys = [
        subject["key"]
        for subject in contract_doc.get("subjects", [])
        if "key" in subject
    ]
    subjects_by_name = discover_subjects_from_contracts(contract_doc)

    override_timestamp = None
    if args.time == "now":
        override_timestamp = int(time.time())
    elif args.time is not None:
        override_timestamp = parse_timestamp(args.time)

    while True:
        for filename in args.filenames:
            fieldnames, rows = read_csv_rows(filename)
            mapping = mapping_for_file(fieldnames, args, subjects_by_name, file_mapping)
            column_map = build_column_map(fieldnames, subjects_by_name, mapping)

            override_timestamp = run_rows(
                filename,
                rows,
                column_map,
                args,
                client,
                override_timestamp,
            )

        if not args.loop:
            break

    if not args.dry_run:
        report_cases(client, subjectkeys)


if __name__ == "__main__":
    try:
        main()
    except Exception as err:
        print(f"ERROR: {err}", file=sys.stderr)
        sys.exit(1)
