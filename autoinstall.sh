#!/bin/sh
echo "Begin script"

echo "Teresa Testing"


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

sudo apt-get install libudev-dev
if [ $? -eq 0 ]; then 
	echo "libudev-dev success"
else
	echo "libudev-dev failed"
	exit 1
fi

sudo apt-get install libical-dev -y

if [ $? -eq 0 ]; then 
	echo "libical-dev success"
else
	echo "libical-dev failed"
	exit 1
fi

echo "done with dependencies installation"

echo "begin setting up pi"

sudo apt-get install git

if [ $? -eq 0 ]; then 
	echo "instal git success"
else
	echo "install git failed"
	exit 1
fi


rm pigpio.zip
sudo rm -rf PIGPIO

cd /home/pi/Downloads/GPIO-Midi-master

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

cd /home/pi/Downloads/GPIO-Midi-master

rm bluez
sudo rm -rf bluez

git clone https://github.com/oxesoft/bluez.git

if [ $? -eq 0 ]; then 
	echo "git clone bluez success"
else
	echo "git clone bluez failed"
	exit 1
fi

cd bluez
if [ $? -eq 0 ]; then 
	echo "cd bluez success"
else
	echo "cd bluez failed"
	exit 1
fi

autoreconf -i

if [ $? -eq 0 ]; then 
	echo "autoreconf -i success"
else
	echo "autoreconf -i failed"
	exit 1
fi

autoconf

if [ $? -eq 0 ]; then 
	echo "autoreconf success"
else
	echo "autoreconf failed"
	exit 1
fi

./configure --prefix=/usr --mandir=/usr/share/man \
				--sysconfdir=/etc --localstatedir=/var
				
if [ $? -eq 0 ]; then 
	echo "./configure --prefix=/usr --mandir=/usr/share/man \
				--sysconfdir=/etc --localstatedir=/var success"
else
	echo "./configure --prefix=/usr --mandir=/usr/share/man \
				--sysconfdir=/etc --localstatedir=/var failed"
	exit 1
fi				
				
make

if [ $? -eq 0 ]; then 
	echo "make bluez success"
else
	echo "make bluez failed"
	exit 1
fi

./configure --enable-midi

if [ $? -eq 0 ]; then 
	echo "./configure --enable-midi success"
else
	echo "./configure --enable-midi failed"
	exit 1
fi

sudo make install

if [ $? -eq 0 ]; then 
	echo "make install bluez success"
else
	echo "make install bluez failed"
	exit 1
fi


cd /home/pi/Downloads/GPIO-Midi-master

rm rpi-midi-ble
sudo rm -rf rpi-midi-ble

git clone https://github.com/oxesoft/rpi-midi-ble

if [ $? -eq 0 ]; then 
	echo "git https://github.com/oxesoft/rpi-midi-ble success"
else
	echo "git https://github.com/oxesoft/rpi-midi-ble failed"
	exit 1
fi

cd rpi-midi-ble/alsa-seq-autoconnect/
if [ $? -eq 0 ]; then 
	echo "cd rpi-midi-ble/alsa-seq-autoconnect/ success"
else
	echo "cd rpi-midi-ble/alsa-seq-autoconnect/ failed"
	exit 1
fi

make

if [ $? -eq 0 ]; then 
	echo "make alsa-seq-autoconnect/ success"
else
	echo "make alsa-seq-autoconnect/ failed"
	exit 1
fi





echo 'Completed Dependencies Installation'
