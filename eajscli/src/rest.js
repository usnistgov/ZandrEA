//
// CONFIGURE THESE IF NEEDED
//
// Default configuration is that the REST server is on the same host
// as the web service - if so, no change is needed.
//
const api_port_number = process.env.REACT_APP_DOCKERENV ? "80" : "9876";
const api_prefix = process.env.REACT_APP_DOCKERENV ? "/api" : "";
const apibase = "http://" + window.location.hostname + ":" + api_port_number + api_prefix;
//
// END OF CONFIGURATION
//


  // this code courtesy of https://www.lowmess.com/blog/fetch-with-timeout/
  // it implements a normal javascript fetch but with a working timeout option
export const fetchJsonWithTimeout = (uri, options = {}, time = 500) => {
    console.log("fetchJsonWithTimeout: %s", apibase + uri);
    // Lets set up our `AbortController`, and create a request options object
    // that includes the controller's `signal` to pass to `fetch`.
    const controller = new AbortController();
    let headers = {'Accept': 'application/json'};
    if ('method' in options && (options.method === 'PUT' || options.method === 'POST')) {
      headers['Content-Type'] = 'application/json';
    }
    const config = { ...options, headers: headers, signal: controller.signal };
    // Set a timeout limit for the request using `setTimeout`. If the body
    // of this timeout is reached before the request is completed, it will
    // be cancelled.
    // eslint-disable-next-line
    const timeout = setTimeout(() => {
      controller.abort()
    }, time)
    return fetch(apibase + uri, config)
      .then((response) => {
        // Because _any_ response is considered a success to `fetch`, we
        // need to manually check that the response is in the 200 range.
        // This is typically how I handle that.
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.statusText}`)
        }
        return response
      })
      .then((response) => response.json())
      .catch((error) => {
        // When we abort our `fetch`, the controller conveniently throws
        // a named error, allowing us to handle them separately from
        // other errors.
        if (error.name === 'AbortError') {
          throw new Error('Response timed out')
        }
        throw new Error(error.message)
      })
  }
  //end of fetchWithTimeout
