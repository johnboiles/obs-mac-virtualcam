project(dal-plugin)

find_library(AVFOUNDATION AVFoundation)
find_library(COCOA Cocoa)
find_library(COREFOUNDATION CoreFoundation)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COCOA Cocoa)
find_library(COREMEDIAIO CoreMediaIO)
find_library(IOSURFACE IOSurface)
find_library(IOKIT IOKit)

# Possible we could remove osme of these
include_directories(${AVFOUNDATION}
                    ${COCOA}
                    ${COREFOUNDATION}
                    ${COREMEDIA}
                    ${COREVIDEO}
                    ${COREMEDIAIO}
                    ${COCOA}
                    ${IOSURFACE}
                    ./
                    ../common)

set(dal-plugin_SOURCES
	Logging.h
	PlugInMain.mm
	PlugInInterface.h
	PlugInInterface.mm
	ObjectStore.h
	ObjectStore.mm
	PlugIn.h
	PlugIn.mm
	Device.h
	Device.mm
	Stream.h
	Stream.mm
	MachClient.h
	MachClient.mm
	../common/MachProtocol.h)

add_library(dal-plugin MODULE
	${dal-plugin_SOURCES}
	${dal-plugin_HEADERS})

set_target_properties(dal-plugin
	PROPERTIES
	BUNDLE TRUE
	OUTPUT_NAME "obs-mac-virtualcam"
	COMPILE_FLAGS "-std=gnu++14 -stdlib=libc++ -fobjc-arc -fobjc-weak")

target_link_libraries(dal-plugin
	${AVFOUNDATION}
	${COCOA}
	${COREFOUNDATION}
	${COREMEDIA}
	${COREVIDEO}
	${COREMEDIAIO}
	${IOSURFACE}
	${IOKIT})

# Workaround because building from Xcode likes to put things in Debug and Release directories
# TODO: This only supports Debug
if (XCODE)
	set(TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/Debug")
else (XCODE)
	set(TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif (XCODE)

add_custom_command(TARGET dal-plugin
	POST_BUILD
	COMMAND rm -rf ${TARGET_DIR}/obs-mac-virtualcam.plugin || true
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_DIR}/obs-mac-virtualcam.bundle ${TARGET_DIR}/obs-mac-virtualcam.plugin
	COMMENT "Rename bundle to plugin"
)

# Note: Xcode seems to run a command `builtin-infoPlistUtility` to generate the Info.plist, but I'm
# not sure where to find that binary. If we had access to it, the command would look something like:
# builtin-infoPlistUtility ${PROJECT_SOURCE_DIR}/../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/SampleVCam-Info.plist -producttype com.apple.product-type.bundle -expandbuildsettings -platform macosx -o mac-virtualcam.bundle/Contents/Info.plist
# Instead, just copy in one that was already generated from Xcode.
add_custom_command(TARGET dal-plugin
	POST_BUILD
	COMMAND cp ${PROJECT_SOURCE_DIR}/Info.plist ${TARGET_DIR}/obs-mac-virtualcam.plugin/Contents/Info.plist
	DEPENDS {PROJECT_SOURCE_DIR}/Info.plist
	COMMENT "Copy in Info.plist"
)

add_custom_command(TARGET dal-plugin
	POST_BUILD
	COMMAND /usr/bin/codesign --force --deep --sign - --timestamp=none ${TARGET_DIR}/obs-mac-virtualcam.plugin
	COMMENT "Codesign plugin"
)
