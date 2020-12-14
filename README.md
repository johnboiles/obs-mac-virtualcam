# OBS (macOS) Virtual Camera (ARCHIVED) 🎥

![Build](https://github.com/johnboiles/obs-mac-virtualcam/workflows/Build%20and%20(maybe)%20Release/badge.svg)

**ATTENTION: STARTING WITH OBS Studio 26.1, THIS PLUGIN IS NOW A PART OF THE OFFICIAL OBS PACKAGE 🎉. Development will now happen on the [OBS Studio GitHub](https://github.com/obsproject/obs-studio). Running this plugin along-side the built-in distribution does not work. If you can, update to OBS 26.1!**

**ATTENTION: Before updating to OBS Studio 26.1, make sure to remove this plugin using the [uninstall instructions](https://github.com/johnboiles/obs-mac-virtualcam#uninstalling).** While it worked for most, some users have reported problems when updating to OBS Studio 26.1 with the plugin installed. You will likely also need to restart any host software (e.g. Chrome, Zoom, etc) after installing OBS Studio 26.1 and starting the virtual camera before the new plugin will work.

Creates a virtual webcam device from the output of [OBS Studio](https://obsproject.com/). Especially useful for streaming smooth, composited video into Zoom, Hangouts, Jitsi etc. Like [CatxFish/obs-virtual-cam](https://github.com/CatxFish/obs-virtual-cam) but for macOS.

![Mar-28-2020 01-55-07](https://user-images.githubusercontent.com/218876/77819715-279b8700-709a-11ea-8885-aa15051665ee.gif)

This code was spun out of [this OBS Project RFC](https://github.com/obsproject/rfcs/pull/15) which was itself spun out of [this issue](https://github.com/obsproject/obs-studio/issues/2568) from [@tobi](https://github.com/tobi). The goal for this, being merged into the core OBS codebase, has been reached 🤞.

## Donating 💸

Consider sending some money in the direction of the [OBS Project](https://obsproject.com/contribute) via [Open Collective](https://opencollective.com/obsproject/contribute), [Patreon](https://patreon.com/OBSProject), or [PayPal](https://www.paypal.me/obsproject). Obviously, without OBS, this plugin would not be very useful! Hugh "Jim" Bailey is OBS Project's full-time lead developer and project maintainer. This money helps him continue to work on OBS!

If, after you donate to the OBS Project, you also want to send some cash my way that's appreciated too! Feel free to [Buy Me a Coffee](https://www.buymeacoffee.com/johnboiles) or [PayPal me](https://paypal.me/johnboiles).

<a href="https://www.buymeacoffee.com/johnboiles" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 41px !important;width: 174px !important;box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;-webkit-box-shadow: 0px 3px 2px 0px rgba(190, 190, 190, 0.5) !important;" ></a>

## Known Issues

* Zoom prior to version 5.1.1 disabled virtual cameras by default. Please update to the latest (5.2.1 at time of writing) to re-enable virtual camera. Start the virtual camera before starting the Zoom application.
* Slack, Webex, Skype and probably some other applications have disabled virtual cameras by default via [application restrictions](https://developer.apple.com/documentation/bundleresources/entitlements/com_apple_security_cs_disable-library-validation?language=objc). Check out [the wiki](https://github.com/johnboiles/obs-mac-virtualcam/wiki/Compatibility) to see if your app is supported. Please [edit the wiki](https://github.com/johnboiles/obs-mac-virtualcam/wiki/Compatibility/_edit) if you try other software that we should include in that list. In most cases you can work around these restrictions by [re-codesigning those applications](https://github.com/johnboiles/obs-mac-virtualcam/wiki/Compatibility#apps-dont-allow-dal-plugins).
* Photo Booth and FaceTime do not support virtual cameras as of macOS 10.14 Mojave since they disallow loading any plugin that's not provided by Apple. Photo Booth can simply be duplicated and renamed (e.g. `Photo Booth copy`) and it will work. There is no known workaroud for FaceTime.
* You _may_ need to restart your computer after installing new versions of this plugin (not sure why 🤷‍♂️).

See also the open [issues](https://github.com/johnboiles/obs-mac-virtualcam/issues) for other reported issues. In case you need help or think you found a bug, see [this](https://github.com/johnboiles/obs-mac-virtualcam#Discussion--Support).

## Installing
*If you are using OBS Studio 26.1 or newer, the virtual camera is already part of OBS Studio. In that case, **DO NOT** install this plugin!*

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

If you are using the version the virtual camera that comes shipped with OBS Studio 26.1, the official place for questions is the `#macos-support` channel in the [OBS Studio Discord](https://discord.gg/obsproject).
If you are still using this plugin, the official place for discussion and chat is in the `#plugins-and-tools` channel in the [OBS Studio Discord](https://discord.gg/obsproject). For questions or troubleshooting, ping @gxalpha#3486 and attach the OBS log, screenshots, and/or crash logs (from Console.app).

## Reporting Issues / Bugs / Improvements

> 🚀 Wonder How to contribute? Have look at our [notes for contributors](https://github.com/johnboiles/obs-mac-virtualcam/wiki/Contributing). There are ways non-technical or minimally-technical folks can contribute too!

This plugin is now archived. If you are having an issue there's a good chance someone has already run into the same thing. Please search through the [issues](https://github.com/johnboiles/obs-mac-virtualcam/issues) before reporting a new one.
If you are using the version the virtual camera that comes shipped with OBS Studio 26.1, also see the issues on the [OBS Studio GitHub](https://github.com/obsproject/obs-studio/issues) and create new issues there.

If you still believe you have found an unreported issue related to this plugin, please open an issue! When you do, include any relevant terminal log, Console.app log, crash log, screen recording and/or screenshots. The more information you can provide, the better!

## Development

Please help me make this thing not janky! See the [this wiki page](https://github.com/johnboiles/obs-mac-virtualcam/wiki/Developing) for build instructions and tips & tricks for developing.

## License

As the goal of this repo was to get merged into [obsproject/obs-studio](https://github.com/obsproject/obs-studio/), the license for this code mirrors the GPLv2 license for that project.
