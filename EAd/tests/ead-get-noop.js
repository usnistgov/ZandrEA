#!/usr/bin/env node

//const wrapper = require('./WRAPPER.js');
//const request = require('request');
const bent = require('bent')

const baseurl = "http://127.0.0.1:9876";

//wrapper.pre();

process.exitCode = 0;

const request = bent(baseurl, 'GET', 'json')

async function test(r, uri, f) {
  try {
    const json = await r(uri);
    f(json, uri);
  } catch (e) {
    console.log("ERROR testing %s: %o", uri, e.message);
    process.exitCode++;
  }
}

test(request, '/noop', (r, uri) => {
  if (r.apiver <= 0) {
    process.exitCode++;
    console.log('ERROR: GET %s returned invalid apiver of %s', uri, apiver);
  }
});

test(request, '/apiver', async (r, uri) => {
  if (r.apiver <= 0) {
    process.exitCode++;
    console.log('ERROR: GET %s returned invalid apiver of %s', uri, apiver);
  }
});

test(request, '/api', (r, uri) => {
  if (request.apiver <= 0) {
    process.exitCode++;
    console.log('ERROR: GET %s returned invalid apiver of %s', uri, apiver);
  }
});

//wrapper.post();
