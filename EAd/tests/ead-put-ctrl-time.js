#!/usr/bin/env node

const bent = require('bent');

const baseurl = "http://127.0.0.1:9876";

const request = bent(baseurl, 'PUT');

process.exitCode = 0;

async function test(r, uri, body, f) {
  try {
    const response = await r(uri, body);
    f(response, uri);
  } catch (e) {
    console.log("ERROR testing %s: %o", uri, e.message);
    process.exitCode++;
  }
}

test(request, '/ctrl/time', {'time': Date.now() / 1000},  (r, uri) => {
  if (r.status != 200) {
    console.log("ERROR: PUT %s returned bad status code %s", uri, r.status);
    process.exitCode++;
  }
});
