#!/bin/bash

tar xvf linux-5.7.0-rc5+-x86.tar.gz
rm -rf /lib.bak
mv /lib /lib.bak
mv ./lib /lib
