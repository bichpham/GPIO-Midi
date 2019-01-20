#!/bin/sh
echo "Begin script"
sudo apt-get install libasound2-dev
if [ $? -eq 0 ]; then 
	echo "libasound2-dev success"
else
	echo "libasound2-dev failed"
	exit 1
fi

sudo apt-get install libglib2.0-dev -y

if [ $? -eq 0 ]; then 
	echo "libglib2.0-dev success"
else
	echo "libglib2.0-dev failed"
	exit 1
fi

sudo apt-get install udev

if [ $? -eq 0 ]; then 
	echo "udev success"
else
	echo "udev failed"
	exit 1
fi

sudo apt-get install libreadline-dev -y

if [ $? -eq 0 ]; then 
	echo "libreadline success"
else
	echo "libreadline failed"
	exit 1
fi

sudo apt-get install libtool -y

if [ $? -eq 0 ]; then 
	echo "libtool success"
else
	echo "libtool failed"
	exit 1
fi

sudo apt-get install intltool -y

if [ $? -eq 0 ]; then 
	echo "intltool success"
else
	echo "intltool failed"
	exit 1
fi

sudo apt-get install libdbus-1-dev

if [ $? -eq 0 ]; then 
	echo "libdbus success"
else
	echo "libdbus failed"
	exit 1
fi

sudo apt-get install libical-dev -y

if [ $? -eq 0 ]; then 
	echo "libical-dev success"
else
	echo "libical-dev failed"
	exit 1
fi


echo 'Completed Dependencies Installation'
