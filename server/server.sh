! /bin/bash
pidfile="./mmod.pid"
name="MikuMikuOnline Server"
prog="./server"
dir="./"
SSD=start-stop-daemon

start() {
        echo -n $"Starting $name: "
        $SSD -S --pidfile $pidfile --make-pidfile --background --exec $prog -d $dir
        RETVAL=$?
        echo
        return $RETVAL
}

stop() {
        echo -n $"Stopping $name: "
        $SSD -K --oknodo --pidfile $pidfile -d $dir
        RETVAL=$?
        echo
        return $RETVAL
}

restart() {
        stop
        start
}

case "$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  restart)
        restart
        ;;
  *)
        echo $"Usage: $0 {start|stop|restart}"
        exit 1
esac