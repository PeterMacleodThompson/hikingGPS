#!/bin/bash

set -e

# Create the source directory
mkdir -p /usr/local/src && cd /usr/local/src

# Clone the latest version
git clone https://github.com/eballetbo/hikingGPS.git

# Go to the application directory and build
cd hikingGPS
meson build/ && ninja -C build/

# Install the application to the default directory
ninja -C build/ install

