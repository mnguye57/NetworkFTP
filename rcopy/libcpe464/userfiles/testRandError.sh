#!/bin/bash

if [ $# -lt 3 ]; then
    echo "Usage: $0 SERVER_PORT FILE_IN ERR_RATE [WINDOW]"
    exit 2
fi

# ===============================
APP_SERVER=server
APP_CLIENT=rcopy
PIDOF=`which pidof`
# ===============================

function clear_defs {
    export CPE464_OVERRIDE_DEBUG=
    export CPE464_OVERRIDE_ERR_RATE=
    export CPE464_OVERRIDE_ERR_DROP=
    export CPE464_OVERRIDE_ERR_FLIP=
    export CPE464_OVERRIDE_PORT=
    export CPE464_OVERRIDE_SEEDRAND=
}

function clean_up {
    echo "Ensuring Apps are closed"
    kill -s KILL `$PIDOF $APP_CLIENT` &> /dev/null
    kill -s KILL `$PIDOF $APP_SERVER` &> /dev/null

    rm -f $APP_SERVER $APP_CLIENT

    clear_defs

    exit
}

trap clean_up SIGHUP SIGINT SIGTERM SIGQUIT

# ===============================

PORT=$1

FILE=$2
FILEOUT=`basename $FILE`.tmp
SIZE=1000
ERROR=0.9 # ignored
WIN=$4    # Leave blank if non-windowed
SERVER=localhost

export CPE464_OVERRIDE_DEBUG=4
export CPE464_OVERRIDE_ERR_RATE=$3
export CPE464_OVERRIDE_ERR_DROP=-1
export CPE464_OVERRIDE_ERR_FLIP=-1

export CPE464_OVERRIDE_PORT=$PORT
export CPE464_OVERRIDE_SEEDRAND=10

# Create special copies of server and client to include port number
cp -f $APP_CLIENT ${APP_CLIENT}_$PORT
cp -f $APP_SERVER ${APP_SERVER}_$PORT
APP_CLIENT=${APP_CLIENT}_$PORT
APP_SERVER=${APP_SERVER}_$PORT

echo "========== SERVER ==========="
./$APP_SERVER 0.9 2>&1 | sed 's/.*/                              &/' &

sleep 1

echo "========== CLIENT ==========="
export CPE464_OVERRIDE_PORT=
export CPE464_OVERRIDE_SEEDRAND=2
./$APP_CLIENT $FILE $FILEOUT $SIZE $ERROR $WIN $SERVER $PORT
APP_CLIENT_RES=$?

echo "========== RESULTS ==========="

if [ $APP_CLIENT_RES -ne 0 ]; then
    echo "- Client Returned $APP_CLIENT_RES"
fi

SERV_PID=`/$PIDOF $APP_SERVER`
if [ `echo $SERV_PID | wc -w` -eq 0 ]; then
	echo "- Server closed early"
elif [ `echo $SERV_PID | wc -w` -gt 1 ]; then
	echo "- Waiting for Server children to close"
    for i in {1..10}; do
        if [ `echo $SERV_PID | wc -w` -gt 1 ]; then
            echo "."
            sleep 1
            SERV_PID=`/$PIDOF $APP_SERVER`
        else
            break;
        fi
    done
    
    if [ `echo $SERV_PID | wc -w` -gt 1 ]; then
        echo "-- Children didn't close"
        ps -eaf | grep $APP_SERVER | grep -v grep | sed 's/.*/--- &/'
    fi
fi
   
if [ `echo $SERV_PID | wc -w` -gt 0 ]; then
    kill $SERV_PID
	echo "- Waiting for Server to close"
    for i in {1..3}; do
        if [ `echo $SERV_PID | wc -w` -gt 0 ]; then
            echo "."
            sleep 1
            SERV_PID=`$PIDOF $APP_SERVER`
        else
            break;
        fi
    done

    if [ `echo $SERV_PID | wc -w` -gt 0 ]; then
	    echo "- Server Didn't Successfully Close"
        kill -s KILL $SERV_PID
    fi
fi

echo "========== DIFF (IN | OUT) ==========="
diff -qs $FILE $FILEOUT
RETVAL=$?

rm -f $FILEOUT
rm -f $APP_SERVER $APP_CLIENT

clear_defs

exit $RETVAL
