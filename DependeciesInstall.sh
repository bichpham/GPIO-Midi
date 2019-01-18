#!/bin/bash
sudo apt-get install libasound2-dev || (echo "libasound2-dev failed"; exit 1)
sudo apt-get install libglib2.0-dev -y || (echo "libglib2.0-dev failed"; exit 1)
sudo apt-get install udev || (echo "udev failed"; exit 1)
sudo apt-get install libreadline-dev -y || (echo "libreadline-dev failed"; exit 1)
sudo apt-get install libtool -y || (echo "libtool failed"; exit 1)
sudo apt-get install intltool -y || (echo "intltool failed"; exit 1)
sudo apt-get install libdbus-1-dev || (echo "libdbus-1-dev failed"; exit 1)
sudo apt-get install libudev-dev || (echo "libudev-dev failed"; exit 1)
sudo apt-get install libical-dev -y || (echo "libical-dev failed"; exit 1)
echo 'Completed Dependencies'
exit 0