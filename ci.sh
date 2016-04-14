#!/bin/bash

wget -q https://launchpad.net/gcc-arm-embedded/5.0/5-2015-q4-major/+download/gcc-arm-none-eabi-5_2-2015q4-20151219-linux.tar.bz2
bzip2 -q -dc gcc-arm-none-eabi-5_2-2015q4-20151219-linux.tar.bz2 | tar xf -
rm gcc-arm-none-eabi-5_2-2015q4-20151219-linux.tar.bz2
chmod +x $PWD/gcc-arm-none-eabi-5_2-2015q4/bin/*
ls -al $PWD/gcc-arm-none-eabi-5_2-2015q4/bin
mkdir -p sdk
wget -q  https://www.nordicsemi.com/eng/nordic/download_resource/54291/46/30914717 -O sdk.zip
unzip -qq sdk.zip -d ./sdk
rm sdk.zip
export NRF51_SDK_DIR=$PWD/sdk
echo "GNU_INSTALL_ROOT := `pwd`/gcc-arm-none-eabi-5_2-2015q4"$'\r\n'"GNU_VERSION := 5.2.1"$'\r\n'"GNU_PREFIX := arm-none-eabi"$'\r\n' > ${NRF51_SDK_DIR}/components/toolchain/gcc/Makefile.posix
mkdir -p $NRF51_SDK_DIR/components/drivers_ext/segger_rtt
git clone https://gist.github.com/25a566178766c7d0a7e04a18b341a732.git
cp ./25a566178766c7d0a7e04a18b341a732/* $NRF51_SDK_DIR/components/drivers_ext/segger_rtt/
echo $NRF51_SDK_DIR
echo $PATH
make