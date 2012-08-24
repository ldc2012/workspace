#!/bin/bash


ulimit -c 1000000
workpath="/home/lidengchao/working/cloud_evaluation_center"
program="cloud_evaluation_center"
prog="$workpath/$program"
configfile="$workpath/config/${program}.conf"
logpath="$workpath/log/"
args="-c $configfile"
start()
{
    	ps -ef|grep "$prog $args"|grep -v grep 2>&1 >>/dev/null
	if [ $? -eq 0 ]
	   then
		echo "$program already runing..."
		exit 1
	   fi
	echo -n "Start $program..."
	$prog $args
	if [ $? -eq 0 ]
	   then
		echo -e "\t\\033[32m[OK]\\033[0m"
	else
		echo -e "\t\\033[0;31m[FAILED]\\033[0;39m"
	   fi
}
stop()
{
	echo -n "Stopping $program..."
	ps -ef | grep "$prog $args"| grep -v "grep" > /dev/null 2>&1
	if [ $? -ne 0 ]
	   then
		echo -e "\t\\033[32m[OK]\\033[0m"
		return 0
		exit 0
	fi
	ps -ef | grep "$prog $args" |grep -v "grep" | awk '{print $2}' | xargs kill -9 > /dev/null 2>&1
	RETVAL=$?
	if [ $RETVAL -eq 0 ]
	then
		echo -e "\t\\033[32m[OK]\\033[0m"
	else
		echo -e "\t\\033[0;31m[FAILED]\\033[0;39m"
	fi
	return $RETVAL
}
status()
{
	echo -n "Check $program status .."
	ps -ef | grep "$prog $args"| grep -v "grep" > /dev/null 2>&1
        if [ $? -eq 0 ]
           then
                echo -e "\t\\033[32m[OK]\\033[0m"
        else
                echo -e "\t\\033[0;31m[FAILED]\\033[0;39m"
           fi
}
case $1 in
 	start)
		$1
		;;
	stop)
		$1
		;;
	status)
		$1
		;;
	restart)
		stop
		start
		;;
	*)
		echo "Usage $0 {start|stop|status|restart}"
		exit 1
		;;
esac
