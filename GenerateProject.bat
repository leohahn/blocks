@echo off
pushd build
cmake .. -G"Visual Studio 15 2017 Win64"
popd
PAUSE
