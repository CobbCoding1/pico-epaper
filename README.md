# Pico ePaper

## Build Instructions
## Windows

Comprehensive guide: https://www.waveshare.com/wiki/Pico-Get-Start-Windows

### Install dependendencies
[Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
[CMake](https://cmake.org/download/)
[VSCode 2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
[Python 3.10](https://www.python.org/downloads/windows/)
[Git](https://git-scm.com/download/win)

### Install pico-sdk
```sh
git clone -b master https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
cd ..
git clone -b master https://github.com/raspberrypi/pico-examples.git
```

### Set up the environment
From VSCode, press Tools -> Command Line -> Developer Powershell
Get the address to the pico-sdk you installed, and type this command
```sh
setx PICO_SDK_PATH <path_to_sdk>
```
Navigate to pico-examples and execute the following commands
```sh
mkdir build
cd build
cmake -G "NMake Makefiles" ..
nmake
```

Now you can update the firmware on the device, according to the instructions from this 
[site](https://www.waveshare.com/wiki/Pico-Get-Start-Windows#Download_the_Firmware)

## Raspbery Pi
Execute the following commands to setup pico-sdk
```sh
cd ~
wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh 
chmod +x pico_setup.sh
./pico_setup.sh
sudo reboot
```

Now install the required libraries
```sh
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential -y
sudo apt install automake autoconf build-essential texinfo libtool libftdi-dev libusb-1.0-0-dev -y
```
Install Pico-lib and Openocd
```sh
cd ~
sudo apt-get install p7zip-full -y
wget https://github.com/EngineerWill/Pico-lib/releases/download/v1.0/Pico-lib.7z
7z x ./Pico-lib.7z

cd ~/pico/openocd/
./bootstrap
./configure --enable-ftdi --enable-sysfsgpio --enable-bcm2835gpio
make -j4
sudo make install
```

