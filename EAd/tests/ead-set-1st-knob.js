#!/usr/bin/env node

const bent = require('bent');

const baseurl = "http://127.0.0.1:9876";

const get = bent(baseurl, 'GET', 'json');
const put = bent(baseurl, 'PUT');

process.exitCode = 0;

async function testget(r, uri, f) {
  try {
    const response = await r(uri);
    await f(response, uri);
  } catch (e) {
    console.log("ERROR testing %s: %o", uri, e.message);
    process.exitCode++;
  }
}
async function testput(r, uri, body, f) {
  try {
    const response = await r(uri, body);
    await f(response, uri);
  } catch (e) {
    console.log("ERROR testing %s: %o", uri, e.message);
    process.exitCode++;
  }
}

testget(get, '/subjects', (subjects, uri) => {
  let subject = subjects.subjects[0];
  let rulekit = subject.rulekits[0];
  let histogram = rulekit.histogram;
  let knob = histogram.knobs[0];
  let value = knob.value;
  console.log("INFO: testing %s knob %d (%s) of histogram %d of rulekit %d (%s) of subject %d (%s)", knob.type, knob.key, knob.idtext, histogram.key, rulekit.key, rulekit.idtext, subject.key, subject.idtext);
  
  let newvalue = value;
  // switch statement fails here because it uses === comparisons and string primitives weren't matching
  if (knob.type == 'Knob_takesGuiFpnAsBoolean') {
    // This is what is normally expected as the first knob in a histogram. The other types aren't supported yet.
    // The type in json is simple boolean, so just invert it.
    newvalue = value ? 0 : 1;
  } else if (knob.type == 'Knob_takesGuiFpnAsInteger' || knob.type == 'Knob_takesGuiFpnAsFloat' || knob.type == 'Knob_takesGuiUin' || knob.type == 'Knob_selectsGuiFpnFromList') {
    console.log('WARNING: Test not implemented for knob type "%s" in knob id %d', knob.type, knob.key);
  } else {
    console.log('ERROR: Found unknown knob type "%s" in knob id %d', knob.type, knob.key);
    process.exitCode++;
  }
  if (newvalue != value) {
    testput(put, '/set/knob', {'key': knob.key, 'value': newvalue}, async (r, uri) => {
      if (r.status != 200) {
        console.log("ERROR: PUT value %s (was %s) to %s returned bad status code %s", newvalue, value, uri, r.status);
        process.exitCode++;
      } else {
	console.log("INFO: PUT succeeded");
      }
    });
  } else {
    console.log("Value didn't change so not testing");
  }
});
