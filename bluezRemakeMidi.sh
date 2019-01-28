#!/bin/sh
#make sure to put this script under bluez folder
echo "Begin script"

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

