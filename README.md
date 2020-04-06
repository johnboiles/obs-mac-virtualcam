# OBS (macOS) Virtual Camera

Creates a virtual webcam device from the output of OBS. Especially useful for streaming smooth, composited video into Zoom, Hangouts, Jitsi etc. Like [CatxFish/obs-virtual-cam](https://github.com/CatxFish/obs-virtual-cam) but for macOS.

![Mar-28-2020 01-55-07](https://user-images.githubusercontent.com/218876/77819715-279b8700-709a-11ea-8885-aa15051665ee.gif)

This code was spun out of this OBS Project [RFC](https://github.com/obsproject/rfcs/pull/15) which was itself spun out of [this issue](https://github.com/obsproject/obs-studio/issues/2568) from @tobi. This was intended as a proof of concept to inform technical decisions in that RFC, but who knows, maybe it will be useful to you in its current state. The goal is for this (or something with equivalent functionality) to eventually be merged into the core OBS codebase 🤞.

This is heavily based on [Apple's CoreMediaIO sample code](https://developer.apple.com/library/archive/samplecode/CoreMediaIO/Introduction/Intro.html) which has been [modernized by @lvsti](https://github.com/lvsti/CoreMediaIO-DAL-Example)

## Known Issues

* OBS crashes when an app using the virtual camera is closed ([#1](https://github.com/johnboiles/obs-mac-virtualcam/issues/1))
* Resolution is hardcoded to 1280x720
* If OBS is closed when an app is opened, the virtual camera may not show up

## Building

To use this plugin, you'll need to clone and build OBS locally, build this plugin, copy it to the right places, then run your local build of OBS:

    # Clone and build OBS
    git clone --recursive https://github.com/obsproject/obs-studio.git
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
    cmake -DLIBOBS_INCLUDE_DIR:STRING=$OBS_DIR/libobs -DLIBOBS_LIB:STRING=$OBS_DIR/build/libobs/libobs.dylib -DOBS_FRONTEND_LIB:STRING=$OBS_DIR/build/UI/obs-frontend-api/libobs-frontend-api.dylib -DQTDIR:STRING=/usr/local/opt/qt ..
    make -j

    # Copy the OBS plugin to your local OBS build
    cp src/obs-plugin/obs-mac-virtualcam.so $OBS_DIR/build/rundir/RelWithDebInfo/obs-plugins/

    # Remove any existing plugin and copy the DAL plugin to the right place
    sudo rm -rf /Library/CoreMediaIO/Plug-Ins/DAL/obs-mac-virtualcam.plugin && sudo cp -r src/dal-plugin/obs-mac-virtualcam.plugin /Library/CoreMediaIO/Plug-Ins/DAL

    # Run your build of OBS
    cd $OBS_DIR/build/rundir/RelWithDebInfo/bin
    ./obs

Now in OBS go to `Tools`->`Start Virtual Camera`, then open your favorite video streaming app (or fully close it then re-open it to make sure it reloads any loaded video plugins). You _should_ be able to pick `OBS Virtual Camera` as a source.

## Development

Please help me make this thing not janky!

### Using the cmake Xcode generator

You can use cmake to generate an `xcodeproj` file to open all the files in Xcode:

    # Set an environment variable that points to the directory for your OBS clone
    export OBS_DIR=$PWD/../obs-studio
    # Set an environment variable pointing to QT
    export QTDIR=/usr/local/opt/qt

    mkdir xcode
    cd xcode
    cmake -DLIBOBS_INCLUDE_DIR:STRING=$OBS_DIR/libobs -DLIBOBS_LIB:STRING=$OBS_DIR/build/libobs/libobs.dylib -DOBS_FRONTEND_LIB:STRING=$OBS_DIR/build/UI/obs-frontend-api/libobs-frontend-api.dylib -DQTDIR:STRING=/usr/local/opt/qt -G Xcode ..

You can then use Xcode to build your binaries (which will include debug symbols). To copy them into the right place you need a slightly different command. From the `xcode` directory created previously:

    # Copy over the OBS plugin
    cp src/obs-plugin/Debug/obs-mac-virtualcam.so $OBS_DIR/xcode/rundir/Debug/obs-plugins
    # Remove any existing plugin and copy the DAL plugin to the right place
    sudo rm -rf /Library/CoreMediaIO/Plug-Ins/DAL/obs-mac-virtualcam.plugin && sudo cp -r src/dal-plugin/Debug/obs-mac-virtualcam.plugin /Library/CoreMediaIO/Plug-Ins/DAL

### Debugging the OBS plugin

To debug the OBS plugin (`obs-mac-virtualcam.so`), you can attach the Xcode debugger to the OBS process. This allows you to step through the OBS plugin code In Xcode, go to `Debug` -> `Attach to Process by PID or Name...`, then enter `obs` and debug the process as `root`. Then run OBS and you should be able to hit breakpoints set inside the plugin code.

### Debugging the DAL plugin

Debugging the DAL plugin (`obs-mac-virtualcam.plugin`) is a little trickier. You need to attach the Xcode debugger to a host process that loads the plugin. One option is to build another copy of OBS (without `obs-mac-virtualcam.so`), attach the debugger to that process, then add the virtual camera as a Video Capture Device there. You should then be able to step through code on the DAL plugin.

## License

As the goal of this repo is to eventually get merged into [obsproject/obs-studio](https://github.com/obsproject/obs-studio/) (or die because someone made a better implementation), the license for this code mirrors the GPLv2 license for that project.
