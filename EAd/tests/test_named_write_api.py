import os
import time

import pytest
import requests


def _base_url():
    """Build the REST base URL for native runs or Docker Compose test runs."""
    explicit = os.getenv("EA_BASEURL")
    if explicit:
        return explicit.rstrip("/")
    proto = os.getenv("EA_PROTO", "http")
    host = os.getenv("EA_HOST", "127.0.0.1")
    port = os.getenv("EA_PORT", "9876")
    return f"{proto}://{host}:{port}"


@pytest.fixture(scope="session")
def api_base_url():
    """Resolve the REST server URL once for all tests in this module."""
    return _base_url()


@pytest.fixture(scope="session")
def session():
    """Share one HTTP client with common JSON headers across all API tests.

    requests.Session keeps default headers in one place and can reuse HTTP
    connections, which keeps the tests concise without changing API behavior.
    """
    session = requests.Session()
    session.headers.update({"Content-Type": "application/json"})
    return session


def _json(response):
    """Decode JSON responses and fail with useful HTTP context if decoding fails."""
    try:
        return response.json()
    except ValueError:
        pytest.fail(f"response was not JSON: HTTP {response.status_code}: {response.text}")


@pytest.fixture(scope="session")
def contracts_doc(session, api_base_url):
    """Fetch the self-describing write contract once and fail fast if unavailable."""
    response = session.get(f"{api_base_url}/contracts", timeout=10)
    if response.status_code != 200:
        pytest.fail(f"GET /contracts failed: HTTP {response.status_code}: {response.text}")
    return _json(response)


@pytest.fixture(scope="session")
def writable_subject(contracts_doc):
    """Pick one real subject whose referenced contract has named input points."""
    contracts_by_id = {
        contract["id"]: contract for contract in contracts_doc.get("contracts", [])
    }
    for subject in contracts_doc.get("subjects", []):
        contract = contracts_by_id.get(subject.get("contract"))
        if contract and contract.get("points"):
            return subject, contract
    pytest.fail("/contracts did not expose any subject with a writable point contract")


@pytest.fixture
def named_sample(writable_subject):
    """Create a fresh valid named sample body for one discovered subject.

    The fixture is function-scoped because validation tests mutate the values.
    """
    subject, contract = writable_subject
    return {
        "subject": subject["key"],
        "values": {point: float(index + 1) for index, point in enumerate(contract["points"])},
    }


def _put_named_sample(session, api_base_url, values_by_subject):
    """Submit one named timestep request using the current time."""
    response = session.put(
        f"{api_base_url}/ctrl/sampletimestep-named",
        json={"time": int(time.time()), "values_by_subject": values_by_subject},
        timeout=10,
    )
    return response, _json(response)


def test_contracts_document_is_self_describing(contracts_doc):
    """GET /contracts exposes contracts and subjects that can be joined by id."""
    assert contracts_doc["schema"] == "zandrea.subject-contracts.v1"
    assert isinstance(contracts_doc["contracts"], list)
    assert isinstance(contracts_doc["subjects"], list)
    assert contracts_doc["contracts"], "expected at least one contract"
    assert contracts_doc["subjects"], "expected at least one subject"

    contracts_by_id = {}
    for contract in contracts_doc["contracts"]:
        assert set(contract) >= {"id", "label", "points"}
        assert isinstance(contract["id"], str) and contract["id"]
        assert isinstance(contract["label"], str)
        assert isinstance(contract["points"], list) and contract["points"]
        assert all(isinstance(point, str) and point for point in contract["points"])
        contracts_by_id[contract["id"]] = contract

    for subject in contracts_doc["subjects"]:
        assert set(subject) >= {"key", "name", "contract"}
        assert isinstance(subject["key"], int)
        assert isinstance(subject["name"], str) and subject["name"]
        assert subject["contract"] in contracts_by_id


def test_named_sampletimestep_accepts_contract_point_names(session, api_base_url, named_sample):
    """PUT /ctrl/sampletimestep-named accepts values keyed by contract point names."""
    response, body = _put_named_sample(session, api_base_url, [named_sample])

    assert response.status_code == 200
    assert body["returncode"] in (0, "OKAY_allDone")
    assert "added named sample" in body["status"]
    assert str(named_sample["subject"]) in body["status"]


def test_named_sampletimestep_rejects_missing_point(
    session, api_base_url, named_sample, writable_subject
):
    """A named sample must include every point listed by the subject contract."""
    _, contract = writable_subject
    missing_point = contract["points"][0]
    del named_sample["values"][missing_point]

    response, body = _put_named_sample(session, api_base_url, [named_sample])

    assert response.status_code == 400
    assert missing_point in body["missing"]
    assert body["unknown"] == []
    assert body["nonnumeric"] == []
    assert body["expected"] == contract["points"]
    assert "missing required point" in body["error"]


def test_named_sampletimestep_rejects_unknown_point(session, api_base_url, named_sample):
    """A named sample must not include point names outside the subject contract."""
    named_sample["values"]["NotARegisteredPoint"] = 12.0

    response, body = _put_named_sample(session, api_base_url, [named_sample])

    assert response.status_code == 400
    assert body["missing"] == []
    assert body["unknown"] == ["NotARegisteredPoint"]
    assert body["nonnumeric"] == []
    assert "unknown point name" in body["error"]


def test_named_sampletimestep_rejects_nonnumeric_point(
    session, api_base_url, named_sample, writable_subject
):
    """Each submitted point value must be numeric before conversion to libEA input."""
    _, contract = writable_subject
    nonnumeric_point = contract["points"][0]
    named_sample["values"][nonnumeric_point] = "not-a-number"

    response, body = _put_named_sample(session, api_base_url, [named_sample])

    assert response.status_code == 400
    assert body["missing"] == []
    assert body["unknown"] == []
    assert body["nonnumeric"] == [nonnumeric_point]
    assert "non-numeric point value" in body["error"]


def test_named_sampletimestep_reports_all_named_value_validation_errors(
    session, api_base_url, named_sample, writable_subject
):
    """Validation reports missing, nonnumeric, and unknown point errors together."""
    _, contract = writable_subject
    missing_point = contract["points"][0]
    nonnumeric_point = contract["points"][1]
    del named_sample["values"][missing_point]
    named_sample["values"][nonnumeric_point] = None
    named_sample["values"]["UnknownPoint"] = 1.0

    response, body = _put_named_sample(session, api_base_url, [named_sample])

    assert response.status_code == 400
    assert body["missing"] == [missing_point]
    assert body["nonnumeric"] == [nonnumeric_point]
    assert body["unknown"] == ["UnknownPoint"]
    assert body["expected"] == contract["points"]


@pytest.mark.parametrize(
    ("values_by_subject", "error_fragment"),
    [
        ([1], "values_by_subject entries must be objects"),
        ([{"subject": "bad", "values": {}}], "subject parameter missing or invalid"),
        ([{"subject": 999999999, "values": {}}], "invalid subject key"),
    ],
)
def test_named_sampletimestep_rejects_malformed_entries(
    session, api_base_url, values_by_subject, error_fragment
):
    """Each values_by_subject item must be an object with a valid subject key."""
    response, body = _put_named_sample(session, api_base_url, values_by_subject)

    assert response.status_code == 400
    assert error_fragment in body["error"]


def test_named_sampletimestep_rejects_missing_values_object(
    session, api_base_url, named_sample
):
    """A subject entry without its values object is rejected before stepping."""
    response, body = _put_named_sample(
        session, api_base_url, [{"subject": named_sample["subject"]}]
    )

    assert response.status_code == 400
    assert "values parameter missing" in body["error"]


def test_named_sampletimestep_rejects_values_array(
    session, api_base_url, named_sample
):
    """The named endpoint rejects legacy positional arrays for the values field."""
    response, body = _put_named_sample(
        session,
        api_base_url,
        [{"subject": named_sample["subject"], "values": [1.0, 2.0]}],
    )

    assert response.status_code == 400
    assert "must be an object keyed by point name" in body["error"]


@pytest.mark.parametrize(
    "payload",
    [
        {"time": int(time.time()), "values_by_subject": {}},
        {"time": int(time.time())},
        {"values_by_subject": []},
    ],
)
def test_named_sampletimestep_rejects_malformed_envelope(session, api_base_url, payload):
    """The top-level request must contain time and an array of subject values."""
    response = session.put(
        f"{api_base_url}/ctrl/sampletimestep-named",
        json=payload,
        timeout=10,
    )
    body = _json(response)

    assert response.status_code == 400
    assert "error" in body
    assert "returncode" in body
