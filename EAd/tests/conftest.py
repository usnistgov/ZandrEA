"""Pytest configuration and shared fixtures for EAd REST API tests."""

import os

import pytest
import requests


def _base_url():
    """Return the REST API base URL, matching existing script conventions."""
    if "EA_BASEURL" in os.environ:
        return os.environ["EA_BASEURL"]
    proto = os.getenv("EA_PROTO", "http")
    host = os.getenv("EA_HOST", "127.0.0.1")
    port = os.getenv("EA_PORT", "9876")
    return f"{proto}://{host}:{port}"


@pytest.fixture(scope="session")
def base_url():
    """Base URL for the REST API, resolved once per session.

    Reads EA_BASEURL (or EA_PROTO/EA_HOST/EA_PORT) from the environment so
    tests can target a native daemon, a Docker container, or any other server.
    Use this fixture as the starting point for every request in a test.
    """
    return _base_url()


@pytest.fixture(scope="session")
def ead_ready(base_url):
    """Guard fixture that fails fast if the ead daemon is not running.

    Hits GET /noop once at the start of the session.  Any test that depends
    on a live server should list ead_ready (or another fixture that
    transitively depends on it) so pytest aborts the whole run with a clear
    message instead of flailing through dozens of connection errors.
    """
    try:
        resp = requests.get(f"{base_url}/noop", timeout=5)
        resp.raise_for_status()
    except requests.RequestException as exc:
        raise RuntimeError(
            f"ead daemon not reachable at {base_url}/noop — "
            f"start it first or set EA_BASEURL: {exc}"
        ) from exc
    return True


@pytest.fixture
def subject_keys(base_url, ead_ready):
    """List of subject keys currently configured in the libEA domain.

    Fetched from GET /subjectkeys on every use (function-scoped) so tests
    that mutate the domain see the latest state.  Use this fixture when a
    test needs a real subject key to exercise write endpoints or to skip
    gracefully when the domain has no subjects configured.
    """
    resp = requests.get(f"{base_url}/subjectkeys", timeout=10)
    resp.raise_for_status()
    data = resp.json()
    return data.get("subjectkeys", [])
