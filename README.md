# OBS (macOS) Virtual Camera

Creates a virtual webcam device from the output of OBS. Especially useful for streaming smooth, composited video into Zoom, Hangouts, Jitsi etc. Like [CatxFish/obs-virtual-cam](https://github.com/CatxFish/obs-virtual-cam) but for macOS.

![Mar-28-2020 01-55-07](https://user-images.githubusercontent.com/218876/77819715-279b8700-709a-11ea-8885-aa15051665ee.gif)

This code was spun out of this OBS Project [RFC](https://github.com/obsproject/rfcs/pull/15) which was itself spun out of [this issue](https://github.com/obsproject/obs-studio/issues/2568) from @tobi. This was intended as a proof of concept to inform technical decisions in that RFC, but who knows, maybe it will be useful to you in its current state. The goal is for this (or something with equivalent functionality) to eventually be merged into the core OBS codebase 🤞.

## Building

To use this plugin, you'll need to clone and build OBS locally, build this plugin, copy it to the right places, then run your local build of OBS:

```
# Clone and build OBS
git clone --recursive https://github.com/johnboiles/obs-studio.git
cd obs-studio

# Follow normal OBS build steps
brew install FFmpeg x264 Qt5 cmake mbedtls swig
mkdir build
cd build
export QTDIR=/usr/local/opt/qt
cmake .. && make -j

# Clone this repo
cd ../..
git clone https://github.com/johnboiles/obs-mac-virtualcam.git
cd obs-mac-virtualcam

# Set an environment variable that points to the directory for your OBS clone
export OBS_DIR=$PWD/../obs-studio

# Build the plugin
mkdir build
cd build
cmake -DLIBOBS_INCLUDE_DIR:STRING=$OBS_DIR/libobs cmake -DLIBOBS_LIB:STRING=$OBS_DIR/build/libobs/libobs.dylib ..
make -j

# Copy the OBS plugin to your local OBS build
cp src/obs-plugin/obs-mac-virtualcam.so $OBS_DIR/build/rundir/RelWithDebInfo/obs-plugins/

# Copy the DAL plugin to the right place
sudo cp -r src/dal-plugin/obs-mac-virtualcam.plugin /Library/CoreMediaIO/Plug-Ins/DAL

# Run your build of OBS
cd $OBS_DIR/build/rundir/RelWithDebInfo/bin
./obs
```

## Known Issues

* OBS crashes when an app using the virtual camera is closed ([#1](https://github.com/johnboiles/obs-mac-virtualcam/issues/1))
* Resolution is hardcoded to 720x480
* If OBS is closed when an app is opened, the virtual camera may not show up
