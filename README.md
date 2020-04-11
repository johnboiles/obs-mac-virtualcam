# CoreMediaIO Device Abstraction Layer (DAL) Minimal Example

This example project is intended to present the simplest possible implementation of a CoreMediaIO DAL plugin for creating a virtual webcam on macOS. Apple provides [sample code](https://developer.apple.com/library/archive/samplecode/CoreMediaIO/Introduction/Intro.html) ([also modernized by @lvsti](https://github.com/lvsti/CoreMediaIO-DAL-Example)) but it's extremely painful to read and modify.

**This plugin currently does not work**, but I think it's close. Any help making it work would be much appreciated.

The goal is for this plugin to serve static frames as a virtual webcam to host software (QuickTime, OBS, Chrome, etc)

## Developing

To try this out:
* Build it in Xcode
* Find `CMIOMinimalSample.plugin` in Xcode's 'Products' folder
* Right click `CMIOMinimalSample.plugin` and choose 'Show in Finder'
* Copy the plugin bundle to `/Library/CoreMediaIO/Plug-Ins/DAL/`
* Open QuickTime
* Watch the logs in Console.app for any logs prefixed with `CMIOMS`

You may also need to change the codesigning to use your own personal developer identity.

Also take a look at [Cameo](https://github.com/lvsti/Cameo) by @lvsti. It allows you to inspect DAL plugins and see all their properties at a glance. It might be helpful to take a known-working plugin (perhaps [johnboiles/obs-mac-virtualcam](https://github.com/johnboiles/obs-mac-virtualcam) or [Snap Camera](https://snapcamera.snapchat.com/) and try to figure out the differences between those plugins and this plugin.

## License

This software is licensed as GPLv2 (as it may someday become a part of [obsproject/obs-studio](https://github.com/obsproject/obs-studio) via [johnboiles/obs-mac-virtualcam](https://github.com/johnboiles/obs-mac-virtualcam)).
