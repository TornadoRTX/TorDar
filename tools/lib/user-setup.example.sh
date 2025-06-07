#!/bin/bash

# Example user setup script. Copy as user-setup.sh and modify as required.

# gcc-13 is not the default gcc version
export CC=/usr/bin/gcc-13
export CXX=/usr/bin/c++-13

# Override conan profile to be gcc-13
export conan_profile=scwx-linux_gcc-13
