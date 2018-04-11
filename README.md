# sensor.live-dovepass

## Installation

Steps:

Clone this repository
Change directory to src folder
```
cd <your_project_folder>/src
```
Clone aws-iot-device-sdk into libs
```
git clone https://github.com/aws/aws-iot-device-sdk-embedded-C.git libs/aws-iot-device-sdk
```
Clone mbedTLS into aws-iot-device-sdk external_libs
```
git clone https://github.com/ARMmbed/mbedtls.git libs/mbedTLS
mv libs/mbedTLS libs/aws-iot-device-sdk/external_libs
```
Start complier source code
```
make
```
