#!/bin/bash

sudo apt update
sudo apt install build-essential libglfw3-wayland libwayland-dev libfreetype-dev libfreetype6 libfreetype6-dev libcapstone-dev libwayland-egl++0 libwayland-egl1-mesa libwayland-egl1 pkg-config libtbb2 libegl-dev libdbus-1-dev libxkbcommon-dev -y

git clone https://github.com/wolfpld/tracy.git
cd tracy
git checkout v0.10
cd ./profiler/build/unix

make -j $(nproc)

echo "Finished building Tracy. You can run it directly $(pwd)/Tracy-Release"
echo "or run the following command to install it:"

echo "sudo cp ./Tracy-Release /usr/local/bin/tracy"
echo "sudo chmod +x /usr/local/bin/tracy"
