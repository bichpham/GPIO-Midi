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

rm pigpio.zip
sudo rm -rf PIGPIO

wget abyz.me.uk/rpi/pigpio/pigpio.zip

if [ $? -eq 0 ]; then 
	echo "download pigpio success"
else
	echo "download pigpio failed"
	exit 1
fi

unzip pigpio.zip
if [ $? -eq 0 ]; then 
	echo "unzip pigpio success"
else
	echo "unzip pigpio failed"
	exit 1
fi

cd PIGPIO
if [ $? -eq 0 ]; then 
	echo "cd pigpio success"
else
	echo "cd pigpio failed"
	exit 1
fi

make
if [ $? -eq 0 ]; then 
	echo "make pigpio success"
else
	echo "make pigpio failed"
	exit 1
fi

sudo make install

if [ $? -eq 0 ]; then 
	echo "make install pigpio success"
else
	echo "make install pigpio failed"
	exit 1
fi

sudo pigpiod

if [ $? -eq 0 ]; then 
	echo "pigpiod success"
else
	echo "pigpiod failed"
	exit 1
fi

wget https://github.com/oxesoft/bluez.zip


echo 'Completed Dependencies Installation'
