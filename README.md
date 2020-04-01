# OBS (macOS) Virtual Camera

NOTE: You probably don't want to be using this repo just yet unless you know what you are doing. Right now the [instructions](https://github.com/obsproject/rfcs/pull/15#issuecomment-606201708) here are an easier way to try this code out.

## Building

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
export OBS_DIR=$PWD/..

# Build the plugin
cmake -DLIBOBS_INCLUDE_DIR:STRING=$OBS_DIR/libobs cmake -DLIBOBS_LIB:STRING=$OBS_DIR/build/libobs/libobs.dylib ..
make -j
```

# TODO: Copy the plugin into the right place
