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
		split -b 4096 -d -a 4 $1 $3$DATE.

		TOTAL_FILE=`ls $3$DATE.* | wc -l`
		echo "The number of files is $TOTAL_FILE"

		for i in $(seq 1 99)
		do
		    j=$(($i * 10))
		    if [ $i -lt 11 ];then
			if [ $TOTAL_FILE -ge $j ];then
			    if [ ! -f $i$3$DATE.tar.gz ];then
				tar -czvf $i$3$DATE.tar.gz $3$DATE.00$(($i - 1))*
			    fi
			fi
		    elif [ $i -ge 11 ];then
			if [ $TOTAL_FILE -ge $j ];then
			    if [ ! -f $i$3$DATE.tar.gz ];then
				tar -czvf $i$3$DATE.tar.gz $3$DATE.0$(($i - 1))*
			    fi
			fi
		    fi
		done
	     			   
		cd -
		sleep 60
	fi 
done
