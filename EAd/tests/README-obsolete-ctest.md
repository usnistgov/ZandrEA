# EAd Testing

Testing is done via CMake and its built-in CTest capability.

Tests are controlled by the CMakeLists.txt file in this directory.

Testing is done using CTest's `FIXTURE` property, which enables a way to group commands
together and run setup and cleanup operations before and after the group.  This means
we have a clean way to start up a test `ead` daemon and run the remainder of the tests
against that single daemon, which saves a lot of startup time.

Be aware that CTest apparently may tests in parallel (when?) so if you have tests
that may interfere with one another, you should use some combination of the `DEPENDS`
and `RESOURCE_LOCK` properties as described here: 

https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/

## Node requirements

I've been using Node 9.x.  Required modules that must be installed:
* requests

## CMake requirements

Note that CMake >= 3.7 must be installed in order to use the fixtures!
Older versions don't honor the dependencies so you'll potentially see cleanup being
run before other tests, resulting in failures.

On Mac, homebrew's version is fine.  On Ubuntu, I had to install the latest
version via `pip install --upgrade cmake` (and make sure it was in my path - it installed
into a non-system directory.)

## How to add a new unit test

First create a node.js (javascript) test script in this ( `EAd/tests` ) directory
with an appropriate name.  (e.g. `ead-something.js` )  It looks like all tests
for all components use the same CMake namespace so I used the ead- prefix.  It must
return 0 on success, non-zero on failure.  You should be able to test your script by
running `node ead-something.js` directly on the command line.  Note that all the
tests so far assume a test `ead` daemon is running listening on localhost port 9876.

Add the file to git.

Lastly, add your test at the end of the main EAd `CMakeLists.txt` file (referencing
the symlink file, NOT the javascript file).  It will need its own `add_test` line
and it will also need to have `FIXTURE` and possibly other properties assigned to it.

Now, from the project root directory you can run:
```
make test
```

If you want debugging (showing stdout/stderr from your test script) you can run:
```
cd build && ctest --verbose
```

