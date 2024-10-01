The default.conf file here is used only in the production build of the webapp docker
container. It is needed to force nginx to listen on port 3000 the same way the
test/dev nodejs server does when running in development mode.

Note that in production mode, the webapp image is a simple nginx web server serving
only static files that were pre-compiled and bundled to minimize size and load
time. The container itself is very very simple.

In development mode, the web server is a nodejs test server that compiles the
React application on the fly. It watches for files that change and recompiles them
immediately so that changes can be seen in the web page immediately; this works
well when the code is bind-mounted into the container. However it is much slower
to start up, and slower to load pages into the browser.
