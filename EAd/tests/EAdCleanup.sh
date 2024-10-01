#!/usr/bin/env bash

# Implement pre- and post-test functions to start up an ead
# instance for testing.

LOGFILE=ead.log
PIDFILE=ead.pid

function die {
  echo "$@" 1>&2
  exit 1
}

cmd="${0##*/}"
cmdbase="${cmd%.*}"
cmdextension="${cmd##.}"
cmdpath=$(dirname "$0")

#### POST:
####   Kill the ead
####   remove the pid file
####   remove the log file

cd "${cmdpath}" || die "${cmdpath}: unable to chdir"
[ -f $PIDFILE ] || die "Cleanup failed: no ead daemon running (missing pid file)"
eadpid=$(tail -1 $PIDFILE)
kill -INT $eadpid
/bin/rm -f Kbase.h5 $PIDFILE

exit 0
