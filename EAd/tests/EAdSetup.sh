#!/usr/bin/env bash

# Implement pre- and post-test functions to start up an ead
# instance for testing.

LOGFILE=ead.log
PIDFILE=ead.pid

function die {
  echo "$@" 1>&2
  /bin/rm -f Kbase.h5 $PIDFILE
  exit 1
}

cmd="${0##*/}"
cmdbase="${cmd%.*}"
cmdextension="${cmd##.}"
cmdpath=$(dirname "$0")

# Locate the ead executable to use
eadexe=$(which ead)
if [ -n "$1" ]; then
  eadexe="$1"
elif [ "x$eadexe" = "x" ]; then
  eadexe="../../bin/ead"
else
  die "Unable to find the ead executable"
fi

#### PRE:
####   Change to tests directory
####   Check if there's already an ead running
####   Remove KBase file
####   Start up private ead instance, saving the pid

cd ${cmdpath} || die "${cmdpath}: could not chdir"
if [ -f $PIDFILE ]; then
  pid=$(tail -1 $PIDFILE)
  kill -0 ${pid} >/dev/null 2>&1 && echo "EAd daemon was already running" && exit 0
fi
/bin/rm -f Kbase.h5
echo "--- startup ---" $(date) >>${LOGFILE}
nohup ${eadexe} -p 9876 -f </dev/null >>${LOGFILE} 2>&1 &
eadpid=$!
echo "Started ead with pid $eadpid"
echo $eadpid >${PIDFILE}

sleep 1	  # give it a second to get going
kill -0 ${eadpid} >/dev/null 2>&1 || die "EAd process died after startup -- check ead.log"

# Need to wait for the daemon to finish starting up and start listening before continuing
i=0
while [ $i -lt 30 ]; do
  netstat -an | egrep -q '[:\.]9876.*LISTEN'
  [ $? -eq 0 ] && exit 0
  i=$(( $i + 1 ))
  sleep 1
done

die "Warning: ${eadexe} listener did not finish starting after $i seconds... aborting"

exit 1
