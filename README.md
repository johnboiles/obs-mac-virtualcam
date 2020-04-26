# CoreMediaIO Device Abstraction Layer (DAL) Minimal Example

This example project is intended to present the simplest possible implementation of a CoreMediaIO DAL plugin for creating a virtual webcam on macOS. Apple provides [sample code](https://developer.apple.com/library/archive/samplecode/CoreMediaIO/Introduction/Intro.html) ([also modernized by @lvsti](https://github.com/lvsti/CoreMediaIO-DAL-Example)) but it's extremely painful to read and modify.

The goal is for this plugin to serve static frames as a virtual webcam to host software (QuickTime, OBS, Chrome, etc)

## Other Examples

Other projects that have implemented a DAL plugin are probably going to give the most insight as to what is missing here.

These projects are direct copies of [Apple's own sample code](https://developer.apple.com/library/archive/samplecode/CoreMediaIO/Introduction/Intro.html).

* [lvsti/CoreMediaIO-DAL-Example](https://github.com/lvsti/CoreMediaIO-DAL-Example)
* [johnboiles/obs-mac-virtualcam](https://github.com/johnboiles/obs-mac-virtualcam) - A fork of @lvsti's repo but adapted to work with OBS.

This project seems significantly different than the Apple sample code in its implementation.

* [webcamoid/webcamoid](https://github.com/webcamoid/webcamoid)

Other examples would be helpful!

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

This software is licensed as MIT. Do what you want with it! But please, if you find ways to improve this software, or you find bugs, please open a [Pull Request](https://github.com/johnboiles/coremediaio-dal-minimal-example/pulls) so others can benefit from it!
