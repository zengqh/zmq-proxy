echo "Installing cmake"
sudo apt-get -y install build-essential
sudo apt-get -y install cmake

echo "Installing BOOST libraries"
sudo apt-get -y install libboost-dev libboost-program-options-dev

mkdir build-Debug
cd build-Debug

cmake ../src -DCMAKE_BUILD_TYPE=Debug
make -j
