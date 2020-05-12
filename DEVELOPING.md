# Developing 🛠

How to modify, build, and debug potential fixes or improvements to this codebase. Please help me make this thing not janky!

## Known Issues

* Can't click menu bar after program start [(OBS Issue #2678)](https://github.com/obsproject/obs-studio/issues/2678).  **NOTE:** This is not an issue with this plugin, but it does make it more annoying to turn on the Virtual Camera created by this plugin after building locally.
* DAL plugin unable to run because of codesigning issues (for example unable to see Virtual Camera in latest version of Zoom). See [issue #4](https://github.com/johnboiles/obs-mac-virtualcam/issues/4) for history and some workarounds.


## Common Problems (with local builds)

* `Failed to load 'en-US' text for module: 'obs-mac-virtualcam.so'`: If the virtual camera doesn't work, you might see this error. However, it is expected and not the problem. There might be another problem you didn't see.
* Build fails at `cmake .. && make -j`: In this case, simply repeat `cmake .. && make -j`.
* Error 2: At the end of the build, the terminal will say `make: *** [all] Error 2`. This is expected and not an error. If it says `Built target obs` a few lines above, your build has succeeded.

_If you still have problems running the virtual camera, see below._

Please confirm that your issue is related to this plugin and not a general OBS problem before opening an issue – try to build OBS locally (but not this plugin) and see if the issue persists. If it does, please don't open an issue here; Instead go open an issue on [obsproject/obs-studio](https://github.com/johnboiles/obs-mac-virtualcam/issues). The same goes for supporting other plugins; it's outside of the scope of this project to tell you how to build other OBS plugins locally (e.g. Browser Source, Speex, etc).

## Building

If you want to clone this plugin (and OBS) locally, follow these steps.

```bash
# Clone and build OBS
git clone --recursive https://github.com/obsproject/obs-studio.git
cd obs-studio

# Follow normal OBS build steps
brew install FFmpeg x264 Qt5 cmake mbedtls swig
mkdir build
cd build
# Note: if you installed homebrew to a custom location, this will be $BREW_INSTALL_PATH/opt/qt
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
cmake -DLIBOBS_INCLUDE_DIR:STRING=$OBS_DIR/libobs -DLIBOBS_LIB:STRING=$OBS_DIR/build/libobs/libobs.dylib -DOBS_FRONTEND_LIB:STRING=$OBS_DIR/build/UI/obs-frontend-api/libobs-frontend-api.dylib -DQTDIR:STRING=$QTDIR ..
make -j

# Copy the OBS plugin to your local OBS build
cp src/obs-plugin/obs-mac-virtualcam.so $OBS_DIR/build/rundir/RelWithDebInfo/obs-plugins/

# Remove any existing plugin and copy the DAL plugin to the right place
sudo rm -rf /Library/CoreMediaIO/Plug-Ins/DAL/obs-mac-virtualcam.plugin && sudo cp -r src/dal-plugin/obs-mac-virtualcam.plugin /Library/CoreMediaIO/Plug-Ins/DAL

# Run your build of OBS
cd $OBS_DIR/build/rundir/RelWithDebInfo/bin
./obs
```

Now in OBS go to `Tools`→`Start Virtual Camera`, then open your favorite video streaming app (or fully close it then re-open it to make sure it reloads any loaded video plugins). You _should_ be able to pick `OBS Virtual Camera` as a source.

## Using the cmake Xcode generator

You can use cmake to generate an `xcodeproj` file to open all the files in Xcode. This is my preferred way of developing code on this plugin.

```bash
# Set an environment variable that points to the directory for your OBS clone
export OBS_DIR=$PWD/../obs-studio
# Set an environment variable pointing to QT
export QTDIR=/usr/local/opt/qt

mkdir xcode
cd xcode
cmake -DLIBOBS_INCLUDE_DIR:STRING=$OBS_DIR/libobs -DLIBOBS_LIB:STRING=$OBS_DIR/build/libobs/libobs.dylib -DOBS_FRONTEND_LIB:STRING=$OBS_DIR/build/UI/obs-frontend-api/libobs-frontend-api.dylib -DQTDIR:STRING=$QTDIR -G Xcode ..
```

You can then use Xcode to build your binaries (which will include debug symbols). To copy them into the right place you need a slightly different command. From the `xcode` directory created previously:

```bash
# Copy over the OBS plugin
cp src/obs-plugin/Debug/obs-mac-virtualcam.so $OBS_DIR/xcode/rundir/Debug/obs-plugins
# Remove any existing plugin and copy the DAL plugin to the right place
sudo rm -rf /Library/CoreMediaIO/Plug-Ins/DAL/obs-mac-virtualcam.plugin && sudo cp -r src/dal-plugin/Debug/obs-mac-virtualcam.plugin /Library/CoreMediaIO/Plug-Ins/DAL
```

## Debugging the OBS plugin

To debug the OBS plugin (`obs-mac-virtualcam.so`), you can attach the Xcode debugger to the OBS process. This allows you to step through the OBS plugin code In Xcode, go to `Debug` → `Attach to Process by PID or Name...`, then enter `obs` and debug the process as `root`. Then run OBS and you should be able to hit breakpoints set inside the plugin code.

## Debugging the DAL plugin

Debugging the DAL plugin (`obs-mac-virtualcam.plugin`) is a little trickier. You need to attach the Xcode debugger to a host process that loads the plugin. One option is to build another copy of OBS (without `obs-mac-virtualcam.so`), attach the debugger to that process, then add the virtual camera as a Video Capture Device there. You should then be able to step through code on the DAL plugin.
