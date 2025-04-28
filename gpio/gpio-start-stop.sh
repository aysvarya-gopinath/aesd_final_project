#!/bin/sh

case "$1" in 
    start)
        echo "STARTED GPIO FUNCTIONALITY"
        start-stop-daemon -S -n gpio_button -a /usr/bin/gpio_button -- -d
        ;;
    stop)
        echo "STOPPED GPIO FUNCTIONALITY"
        start-stop-daemon -K -n gpio_button
        ;;
    *)
        echo "USAGE :$0 {start|stop}"
    exit 1
esac
exit 0


