#!/usr/bin/env node

const bent = require('bent');

const baseurl = "http://127.0.0.1:9876";

const request = bent(baseurl, 'GET', 'json');

process.exitCode = 1;

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
    let subject = r.subjectkeys[0];
    const request2 = bent(baseurl, 'GET', 'json')
    await test(request2, '/casekeys?subject='+subject, async (r, uri) => {
      if (r.casekeys && r.casekeys.constructor === Array && r.casekeys.length >= 0) {
	process.exitCode = 0;
      } else {
	console.log('ERROR: GET %s did not return an array of casekeys', uri);
      }
    });
  }
});
