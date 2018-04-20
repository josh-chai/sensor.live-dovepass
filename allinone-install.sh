#!/bin/bash
###################################
# allinone-install.sh
#
# cd <your_project_folder>
# git clone https://github.com/SoftChef/sensor.live-dovepass
# cd sensor.live-dovepass/
# bash ./allinone-install.sh
# (more than 20 minutes ...)
#
###################################

SUDO_PASSWORD=bananapi
APP_HOME=`pwd`

echo "$SUDO_PASSWORD" | sudo -S apt-get update

echo -e "----- install curl lib -----"
echo "$SUDO_PASSWORD" | sudo -S rm -f /var/lib/dpkg/lock
echo "$SUDO_PASSWORD" | sudo -S apt-get -y install libcurl4-gnutls-dev
# echo "$SUDO_PASSWORD" | sudo -S apt-get -y install libcurl4-openssl-dev
echo -e "----- install curl lib done -----\n\n"

echo -e "----- install cmake -----"
echo "$SUDO_PASSWORD" | sudo -S rm -f /var/lib/dpkg/lock
echo "$SUDO_PASSWORD" | sudo -S apt-get -y install cmake
echo -e "----- install cmake done -----\n\n"

echo -e "----- install cJSON -----"
cd /tmp
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON/
mkdir build
cd build
cmake ..
make
echo "$SUDO_PASSWORD" | sudo -S make install
echo "$SUDO_PASSWORD" | sudo -S mv /usr/local/include/cjson /usr/include
echo "$SUDO_PASSWORD" | sudo -S mv  /usr/local/lib/libcjson* /usr/lib
echo "$SUDO_PASSWORD" | sudo -S mv  /usr/local/lib/cmake/ /usr/lib
cd /tmp
rm -fr cJSON
echo -e "----- install cJSON done -----\n\n"

# echo -e "----- clone application source code -----"
# cd $HOME
# git clone https://github.com/SoftChef/sensor.live-dovepass
# cd sensor.live-dovepass/src
# echo -e "----- clone application source code done -----\n\n"

echo -e "----- clone aws-iot-device-sdk -----"
cd $APP_HOME/src
git clone https://github.com/aws/aws-iot-device-sdk-embedded-C.git libs/aws-iot-device-sdk
echo -e "----- clone aws-iot-device-sdk done -----\n\n"

echo -e "----- clone mbedTLS -----"
git clone https://github.com/ARMmbed/mbedtls.git libs/mbedTLS
mv libs/mbedTLS/* libs/aws-iot-device-sdk/external_libs/mbedTLS; rm -fr libs/mbedTLS
echo -e "----- clone mbedTLS done -----\n\n"

echo -e "----- make dovepass -----"
make
echo "$SUDO_PASSWORD" | sudo -S make install
echo -e "----- make dovepass done -----"
