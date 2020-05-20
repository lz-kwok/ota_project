#!/bin/sh
#
#
# $1 file name :/nfs/filename
# $2 dest path
# $3 split file name
while true;do
	if [ ! -f /home/leon/Downloads/develop/$1 ]; then
		echo "no log file"
		sleep 1
		exit
	else
		echo "update log file"
		rsync -av /home/leon/Downloads/develop/$1 $2
		DATE=$(date +%Y%m%d)
		cd $2
		split -b 1024 -d -a 4 $1 $3$DATE.
		cd -
		sleep 60
	fi 
done
