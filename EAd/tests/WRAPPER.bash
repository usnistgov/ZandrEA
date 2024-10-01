#!/usr/bin/env bash

# Implement pre- and post-test functions to start up an ead
# instance for testing.

function die {
  echo "$@" 1>&2
  exit 1
}

TMPDIR=$(mktemp -d -t ea.XXXXXXXX)
[ $? -ne 0 ] && die "mktemp failed"

function cleanup {
  /bin/rm -rf "$TMPDIR"
}
trap cleanup EXIT

# Determine the name of the javascript file we're going to run

cmd="${0##*/}"
cmdbase="${cmd%.*}"
cmdextension="${cmd##.}"
cmdpath=$(dirname "$0")
jscmd="${cmdpath}/${cmdbase}.js"

[ -e "${jscmd}" ] || die "${jscmd}: not found"

# Locate the ead executable to use
eadexe=$(which ead)
if [ -n "$1" ]; then
  eadexe="$1"
elif [ "x$eadexe" != "x" ]; then
  eadexe="../../build/EAd/ead"
else
  die "Unable to find the ead executable"
fi

# Locate the node executable to use
nodeexe=$(which node)
if [ -z "$nodeexe" ]; then
  die "Unable to find the ead executable"
fi

#### PRE:
####   Create and cd to temp directory
####   Start up private ead instance, saving the pid

${eadexe} -w "${TMPDIR}" -p 9876 -f 1 -i 0 </dev/null &
eadpid=$!
echo "Started ead with pid $eadpid"
sleep 5	  # give it a few seconds to get initialized

#### TEST:
####   Run the test, saving the exit code
${nodeexe} ${jscmd}
retval=$?

#### POST:
####   Kill the ead
####   Clean up the temp directory
####   Return the test exit code
kill -INT $eadpid
#wait $eadpid  # helps suppress termination message?

exit $retval
