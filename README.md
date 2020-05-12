# OBS (macOS) Virtual Camera 🎥

![Release](https://github.com/johnboiles/obs-mac-virtualcam/workflows/Release/badge.svg)

Creates a virtual webcam device from the output of [OBS Studio](https://obsproject.com/). Especially useful for streaming smooth, composited video into Zoom, Hangouts, Jitsi etc. Like [CatxFish/obs-virtual-cam](https://github.com/CatxFish/obs-virtual-cam) but for macOS.

![Mar-28-2020 01-55-07](https://user-images.githubusercontent.com/218876/77819715-279b8700-709a-11ea-8885-aa15051665ee.gif)

This code was spun out of [this OBS Project RFC](https://github.com/obsproject/rfcs/pull/15) which was itself spun out of [this issue](https://github.com/obsproject/obs-studio/issues/2568) from [@tobi](https://github.com/tobi). The goal is for this (or something with equivalent functionality) to eventually be merged into the core OBS codebase 🤞.

## Known Issues

* The virtual camera doesn't work with all programs due to a combination of [application restrictions](https://developer.apple.com/documentation/bundleresources/entitlements/com_apple_security_cs_disable-library-validation?language=objc) and OS restrictions (for built-in apps like Facetime). Check out [COMPATIBILITY.md](https://github.com/johnboiles/obs-mac-virtualcam/blob/master/COMPATIBILITY.md) to see if your app is supported. Please [edit that file](https://github.com/johnboiles/obs-mac-virtualcam/edit/master/COMPATIBILITY.md) and submit a Pull Request if you try other software that we should include in that list.
* OBS needs to be restarted after changing resolutions for the Virtual Camera to work. (#83)

## Installing

* Download and install the latest version of OBS from the [official website](https://obsproject.com).
* Download the latest `.pkg` installer on the [Releases page](https://github.com/johnboiles/obs-mac-virtualcam/releases)
* Run the `.pkg` installer (entering your password when required)
* If you're already running OBS, make sure to restart it.
* Restart any app that was running during the installation process that is supposed to pick up the camera.
* To start the virtual camera, go (in OBS) to `Tools`→`Start Virtual Camera`.

Your OBS video should now show up in the target app!

## Uninstalling

You can easily uninstall this plugin by deleting the OBS plugin (in `/Library/Application\ Support/obs-studio/plugins/`) and the DAL plugin (in `/Library/CoreMediaIO/Plug-Ins/DAL/`).

```bash
sudo rm -rf /Library/CoreMediaIO/Plug-Ins/DAL/obs-mac-virtualcam.plugin
sudo rm -rf /Library/Application\ Support/obs-studio/plugins/obs-mac-virtualcam
```

## Discussion / Support

The official place for discussion and chat related to this plugin is in the `#plugins-and-tools` channel in the [OBS Studio Discord](https://discord.gg/obsproject). For questions or troubleshooting, ping @gxalpha#3486 and attach the OBS log, screenshots, and/or crash logs (from Console.app).

## Reporting Issues / Bugs / Improvements

> 🚀 Wonder How to contribute? Have look at our [notes for contributors](./CONTRIBUTING.md). There are ways non-technical or minimally-technical folks can contribute too!

This plugin is still very much a work in progress. If you are having an issue there's a good chance someone has already run into the same thing. Please search through the [issues](https://github.com/johnboiles/obs-mac-virtualcam/issues) before reporting a new one.

Also, make sure you're running the most recent version of OBS. We're only officially supporting the most recent version of OBS at any given time.

If you still believe you have found an unreported issue related to this plugin, please open an issue! When you do, include any relevant terminal log, Console.app log, crash log, screen recording and/or screenshots. The more information you can provide, the better!

## Development

Please help me make this thing not janky! See the [DEVELOPING.md](./DEVELOPING.md) file for build instructions and tips & tricks for developing.

## License

As the goal of this repo is to eventually get merged into [obsproject/obs-studio](https://github.com/obsproject/obs-studio/) (or die because someone made a better implementation), the license for this code mirrors the GPLv2 license for that project.
