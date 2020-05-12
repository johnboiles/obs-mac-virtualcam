# Compatibility

A list of programs this is currently compatible with. Please note that this list is not complete.

If you find inaccuracies in this list or want to provide more detail, please [edit this file](https://github.com/johnboiles/obs-mac-virtualcam/edit/master/COMPATIBILITY.md) and then submit a Pull Request.

|Program|Compatiblity|Notes|
|---|---|---|
|Google Chrome|Yes||
|Zoom|No*|Possible to run by changing entitlements. See note on apps that don't allow DAL plugins.|
|Skype|Yes||
|Discord|Yes||
|QuickTime Player|Yes||
|Photo Booth|Maybe|Seems to work for 4:3 resolutions for some people 🤷‍♂️|
|FaceTime|No*|May work in older versions of macOS 🤷‍♂️|
|Safari|No|
|Slack|No*|Possible to run by changing entitlements. See note on apps that don't allow DAL plugins.||
|Amazon Chime|No*|Possible to run by changing entitlements. See note on apps that don't allow DAL plugins.|
|Microsoft Teams|Yes||
|RingCentral|Yes||
|Brave|Yes||
|Edge|Yes||
|Firefox|No|

## Why doesn't my app work

There are two known reasons this plugin does not work.

### Apps don't allow DAL plugins

As of macOS 10.15 (Catalina), apps need to explicitly allow Device Abstraction Layer (DAL) plugins to work by setting [`com.apple.security.cs.disable-library-validation`](https://developer.apple.com/documentation/bundleresources/entitlements/com_apple_security_cs_disable-library-validation?language=objc) in their Info.plist. For apps that don't set this key (e.g. Zoom and Slack), it's possible to manually add this key, then re-codesign the app. Take a look at [issue #4](https://github.com/johnboiles/obs-mac-virtualcam/issues/4) for details. Note that you'll need to redo this process every time the app is opened.

You can try to re-codesign the application to avoid this. Please be aware this could have security implications for the app. For example (for Zoom):

```bash
sudo codesign -f -s - /Applications/zoom.us.app
```

### macOS System Apps block DAL plugins

Recent versions of macOS seem to disallow DAL plugins for system apps (e.g. Safari, Facetime, Photobooth). More investigation is needed to understand if this is the same issue as above, just for Apple provided apps, or if there's some other mechnaism that is blocking the DAL plugin from running.

TODO: Which versions of macOS block DAL plugins in which apps?
