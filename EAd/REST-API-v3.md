# EA REST API

This document describes the REST API interface to libEA as implemented by ead (in `handler.cpp`).

The REST API is a mapping from HTTP REST-style queries over a web interface to the libEA back end engine that
does the fault analysis. Therefore, to understand the terminology here you will first want to familiarize yourself
with the libEA documentation.

There is some minor extended functionality, but it is more or less a 1:1 mapping to the libEA C++ API. Most
of the extended functionality involves coalescing multiple low-level C++ API calls into higher-level REST
calls that return a lot more information at once, in order to reduce the effect of the much- higher REST API
latency due to network round trip times (RTT).

There are a number of libEA API calls that are not represented in the REST API, however the implemwentation
documented were is what was sufficient to write a REST client.

To further assist with improving network performance, the REST server will compress (using normal HTTP Gzip
compression) certain types of reponses that are likely to return large amounts of data. The overhead from
compression is far outshadowed by the improvement in RTT latency.

All HTTP response bodies will be in JSON format.

API testing is easy using varations on `curl -X METHOD -i URL`.  For example:
```
curl --compress http://localhost/api/apiver
curl --compress -H "Content-Type: application/json" -d '{"time": "10"}' -X PUT http://localhost/api/ctrl/tm
```

ALL replies for any type of HTTP request will include the API version number used in the response.

SOME replies for some types of requests may include status or error information returned from the library call.

| *Parameter* | *Description* | *Note* |
| ----------- | ------------- | ------ |
| `apiver:INT` | Integer version number of the API implementation spec that may affect how the returned data is presented | Always provided |
| `seq:INT` | Integer sequence number that increments any time information changes and the client needs to be pull fresh data |
| `error:STR` | Human-readable error from libEA related to the request | Optional. SEE ALSO the HTTP status code (200, etc.) in the response |
| `status:STR` | Human-readable status from libEA related to the request | Optional |

Note some optional querystrings are accepted for some HTTP request types to modify the generated response, as described in each section below.

## How to read this document

### Request vs. Response

HTTP is a request/response protocol. A client (user) sends a request to the HTTP server, and the server replies with
a response. At the REST level there is no persistent connection so the server cannot initiate an exchange or
independently trigger any client action.

### Sequence numbers

Because the server can't arbitrarily contact the client to tell it to update its display, sequence numbers (`seq`)
have been added to all HTTP responses from the server. This allows the client to periodically poll the server
to see if anything has changed without having to refresh everything in the client every time.

The sequence number in incremented any time one of the PUT or POST endpoints are called that can cause a change
in the back end.  The sequence number is managed inside the REST code so it is not part of libEA itself.

This is not a fine-grained approach (the client doesn't know what changed so must refresh everything) but it has
sufficed for the amount of data that needs to be exchanged so far.

There is also an event-wait REST call with a timeout that allows the client to just wait synchronously until
either the sequence number updates or a timeout is reached.

### Thread safety

This is not directly related to the API but it is worth noting that libEA is not guaranteed to be thread-safe. The
REST server works around this by implementing a global lock around libEA API calls that can change data in the libEA
back end or might cause a crash if data changes while a response is being generated in another thread.  It is
likely that there are additional calls that need locking but so far we have not observed any unexpected crashes.

If you have a lot of clients hitting the REST server AND have a lot of bacnet data streaming in, you will be
much more likely to find problems with this implementation.

### Data types used in this document

LibEA was written in C++ and uses strong typing, however JSON, which is used in this API, does not. This
document mainly shows the base types passed via JSON. You should refer to the libEA documentation and code
for clarification about specific data types.

* `STR`: a string value - must be quoted
* `BOOL`: a boolean value (value is true or false, no quotes)
* `INT`: an integer, commonly used as an object identifier (key), index into option lists, and other numeric values
* `FLT`: a floating point value, mainly used for some types of knob values
* `DBL`: a higher-precision floating point value, used for point data values

A data type abbreviation with an empty bracket suffix, i.e. `INT[]` means that the type is an array (list) of the
specified type.

For a description of the available data types in JSON and how to represent them or parse them, see for
example: https://www.w3schools.com/js/js_json_datatypes.asp

### Object identifiers (keys)

LibEA uses different object types to contain information about specific objects - each object type contains
different attributes, or values specific to each instance of that object. (See: libEA/exportTypes.hpp)

In order to refer to specific objects inside the libEA back end, libEA assigns objects (Cases, Subjects,
Features, etc.) a unique integer value so that the client has a way to refer back to specific objects.

Some effort has been made in the REST API to standardize on the term "key" however there are instances where
attribute names refer to an object type (e.g. "case") rather than using the term "key". Just be aware that
API request parameter names that refer to objects are always object keys, since an object cannot be referenced any
other way. However in REST API responses, a "case" for example is likely to be a JSON object describing
the libEA Case object. Response values that are keys are usually clear in naming them such.

### Alerts

The EA library has a concept of Alerts which are generated when e.g. a new Case is created, or other systemic
information is provided by the library. Note that reading these alerts from the library clears them, so the REST
handler keeps a ring buffer of all the alerts internally, and the individual web client instances are responsible
for keeping track of and hiding the alerts that have been closed by the user.

## HTTP GET endpoints

These are used to obtain information.

### Mandatory GET response parameters

In addition to the JSON return values listed below, every HTTP GET request will also return the following as part
of the response's JSON payload:

| *Parameter* | *Description* |
| ----------- | ------------- |
| `alertseq:INT` | Integer sequence number that increments for every new alert that is generated |

### Accepted GET querystrings

Querystrings are appended by the client to the HTTP request path per the HTTP standard to modify the format of the response.

| *Querystring* | *Description* |
| ------------- | ------------- |
| `compact` | If specified, only return top-level information rather than recursively returning a tree of objects. False by default. |

### GET Endpoints

The REST endpoints for HTTP GET are as follows:


| *URI* | *JSON params* | *Compress* | *JSON return* | *Description* |
| ----- | ------------- | ---------- | ------------- | ------------- |
| `/apiver` | | | | |
| `/api` | | | | |
| `/noop` | | | | |
| `/ctrl/eventwait` | `timeout:INT` (opt; def 30)<br>`seq:INT` (required) | | `timedout:BOOL` (if true) | Waits up to `timeout` seconds for `seq` to update. Returns `timedout` if timeout was reached (no changes detected). |
| `/domain` | | | `label:STRING`<br>`subjectkeys:INT[]`<br>`subjects:OBJ[]` (unless compact) | Returns top-level domain information that will probably never change. |
| `/casekeys` | | | `label:STRING`<br>`subjectkeys:INT[]`<br>`subjects:OBJ[]` (unless compact) | Returns top-level domain information that will probably never change. |
| `/casecounts` | | | `casecounts:OBJ[]` | Returns a list of objects with attributes `subject:INT` and `cases:INT` where cases is the number of cases for that subject |
| `/case` | `key:INT` | * | `key:INT`<br>`label:STR`<br>`krono:INT` (if snapshot)<br>`report:STR[]`<br>`prompt:STR[]`<br>`options:STR[]`<br>`error:STR` (ONLY if error) | Returns the attributes from the corresponding case key (always recurses) OR the error message |
| `/cases` | `subject:INT` | * | `cases:OBJ[]` | Same as above but returns a list of case objects for a given subject |
| `/featurekeys` | `subject:INT` | | `featurekeys:INT[]` | Return a list of feature keys for the specified subject |
| `/feature` | `key:INT` | | `key:INT`<br>`type:INT`<br>`uai:INT`<br>`label:STR`<br>`units:STR`<br>`message:STR`<br>`state:STR`<br>`knobs:INT[]`<br>`histogram:INT`<br>`error:STR` (only if error) | Return all the feature attributes for the specified feature key OR an error |
| `/features` | `subject:INT` | * | `features:OBJ[]` | Same as above but returns a list of feature objects for a given subject |
| `/krono` | `key:INT` | * | `key:INT`<br>`reply:STR`<br>`type:INT`<br>`caption:STR`<br>`panes:INT[]`<br>`timestamps:INT[]`<br>`knobs:INT[]`<br>`error:STR` (ONLY if error) | Returns the attributes from the corresponding krono key OR the error message |
| `/subjectkeys` | | | `subjectkeys:INT[]` | Return a list of all the configured subject keys |
| `/subject` | `subject:INT` | * | `key:INT`<br>`idtext:STR`<br>`reply:STR`<br>`domain:INT`<br>`label:STR`<br>`info:STR[]`<br>`name:STR`<br>`featurekeys:INT[]`<br>`knobkeys:INT[]`<br>`rulekitkeys:INT[]`<br>`casekeys:INT[]`<br>`points:STR[]`<br>`features:OBJ[]` (not compact)<br>`knobs:OBJ[]` (not compact)<br>`rulekits:OBJ[]` (not compact)<br>`cases:OBJ[]` (not compact)<br>`error:STR` | Returns the attributes from the corresponding subject key OR the error message |
| `/subjects` | | * | `subjects:OBJ[]` | Same as above but returns a list of all the subject objects |
| `/alerts` | | * | `alerts:OBJ[]` | Returns a list of alert objects (consisting of `id:INT` and `message:STR` attributes) for all alerts in the ring buffer. Each client instance is reponsible for keeping track of events the user no longer wishes to see. |

## HTTP POST endpoints

These are used to cause an action.

### Mandatory POST response parameters

There are no additional mandatory parameters for POST besides the global ones shown above.

### Accepted POST querystrings

Querystrings are not neede with the POST endpoints.

### POST Endpoints

The REST endpoints for HTTP POST are as follows:

| *URI* | *JSON params* | *Compress* | *JSON return* | *Description* |
| ----- | ------------- | ---------- | ------------- | ------------- |
| `/ctrl/singlestep` | | | | Triggers libEA to process a set of submitted data. This updates `seq` |
| `/ctrl/shutdown` | | | | Triggers libEA to close any open files and release memory for a clean shutdown |

## HTTP PUT endpoints

These are used to submit data or change UI settings.

Note that ALL successful PUT requests will update the `seq` sequence number.

### Mandatory PUT response parameters

There are no additional mandatory parameters for PUT besides the global ones shown above.

| *Parameter* | *Description* |
| ----------- | ------------- |
| `returncode:INT` | libEA enumerated return code type as defined in libEA/exportTypes.hpp |

### Accepted PUT querystrings

Querystrings are not neede with the PUT endpoints.

### PUT Endpoints

The REST endpoints for HTTP PUT are as follows:

| *URI* | *JSON params* | *Compress* | *JSON return* | *Description* |
| ----- | ------------- | ---------- | ------------- | ------------- |
| `/ctrl/time` | `time:INT`| | | Submits a new timestamp to libEA to be associated with the next batch(es) of data |
| `/ctrl/sample` | `subject:INT`<br>`values:DBL[]`| | `returncode:INT` | Submits an array containing all the point data for the specified subject key using the last set timestamp (above) |
| `/ctrl/sampletimestamp` | `time:INT`<br>`values_by_subject:OBJ[]`| | `returncode:INT` | This function combines the 3 steps of setting a timestamp, submitting data to one or more subjects corresponding to that timestamp, and then calling the SingleStep function to process it in a single atomic operation. `values_by_subject` is an array of objects with attributes `subject:INT` and `values:DBL[]` like the calls above |
| `/ctrl/answercase` | `case:INT`<br>`answer:INT` | | `success:BOOL` | Reply to the specified case key with the answer of zero-based option `answer` (from the list of options in the case) |
| `/set/knob` | `key:INT`<br>`value:INT\|FLT` | | `success:BOOL` | Sets the knob specified by `key` to an integer or float value |
| `/set/histogram/mode` | `key:INT`<br>`value:INT` | | `success:BOOL` | Sets the mode of the histogram specified by `key` to `value` |
| `/set/histogram/span` | `key:INT`<br>`value:INT` | | `success:BOOL` | Sets the span of the histogram specified by `key` to `value` |

## HTTP DELETE endpoints

There are currently no DELETE endpoints.
