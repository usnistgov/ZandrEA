#!/usr/bin/env node

const bent = require('bent');

const baseurl = "http://127.0.0.1:9876";

const request = bent(baseurl, 'GET', 'json');

process.exitCode = 0;


async function test(r, uri, f) {
  try {
    const json = await r(uri);
    await f(json, uri);
  } catch (e) {
    console.log("ERROR testing %s: %o", uri, e.message);
    process.exitCode++;
  }
}

test(request, '/domain', async (r, uri) => {
  if (r.apiver <= 0) {
    process.exitCode++;
    console.log('ERROR: GET %s returned invalid apiver of %s', uri, apiver);
  } else {
    if (! (r.label && r.label.length >= 0)) {
      console.log('ERROR: Get %s did not return a label string: %o', r);
      process.exitCode++;
    }
    if (! (r.subjectkeys && r.subjectkeys.constructor === Array && r.subjectkeys.length >= 0)) {
      console.log('ERROR: GET %s did not return a viable list of subjectkeys: %o', uri, r);
    }
  }
});
