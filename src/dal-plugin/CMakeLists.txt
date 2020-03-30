project(mac-virtualcam-plugin)

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
                    ../CoreMediaIO/PublicUtility
                    ../CoreAudio/PublicUtility
                    ../CoreMediaIO/PublicUtility/IOSurfaceAssistant
                    ../CoreMediaIO/PublicUtility/POSIXThreadAssistant
                    ../CoreMediaIO/PublicUtility/IOVideoAssistant
                    ../CoreMediaIO/PublicUtility/IOStreamAssistant
                    ../CoreMediaIO/PublicUtility/IOKitAssistant
                    ../CoreMediaIO/PublicUtility/CoreMediaAssistant
                    ../CoreMediaIO/PublicUtility/CoreVideoAssistant
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device
                    # Verified to be necessary
                    ../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Client
                    ../CoreMediaIO/PublicUtility/DALAssistant
                    ../generated)

set(mac-virtualcam-plugin_SOURCES
	../CoreMediaIO/PublicUtility/IOSurfaceAssistant/CMIO_SA_Assistance.cpp
	../CoreMediaIO/PublicUtility/POSIXThreadAssistant/CMIO_PTA_NotificationPortThread.cpp
	../CoreMediaIO/PublicUtility/IOStreamAssistant/CMIO_IOSA_Assistance.cpp
	../CoreMediaIO/PublicUtility/IOKitAssistant/CMIO_IOKA_NotificationPort.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_ScheduledOutputNotificationProc.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Deck.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_FirstOutputPresentationTimeStamp.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_ControlDictionary.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_ClientStream.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Frame.cpp
	../CoreAudio/PublicUtility/CACFDistributedNotification.cpp
	../CoreAudio/PublicUtility/CAGuard.cpp
	../CoreMediaIO/PublicUtility/CMIO_SMPTETimeBase.cpp
	../CoreAudio/PublicUtility/CAPThread.cpp
	../CoreMediaIO/PublicUtility/POSIXThreadAssistant/CMIO_PTA_CFMachPortThread.cpp
	../CoreMediaIO/PublicUtility/IOVideoAssistant/CMIO_IOVA_Assistance.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_UserInfo.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Stream.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_VirtualDevice.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_Stream.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/Properties/CMIO_DP_Sample_Property_HogMode.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_PlugIn.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_IOBackedDevice.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_Device.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_SampleVCam_PlugInInterface.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_SuspendedByUser.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_SMPTETimeCallback.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_OutputBuffers.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_NoData.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_HogMode.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_FormatList.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_EndOfData.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_DeviceMaster.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Clock.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_ClientSyncDiscontinuity.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Base.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_PlugIn.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Object.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn/CMIO_DP_IOV_ControlDictionary.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn/CMIO_DP_IOV_Control.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_HardwarePlugInInterface.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_DeviceSettings.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_DeviceCommands.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Device.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Control.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Command.cpp
	../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Client/CMIO_DPA_Sample_ClientExtras.cpp
	../CoreMediaIO/PublicUtility/DALAssistant/CMIO_DALA_System.cpp
	../CoreMediaIO/PublicUtility/CMIODebugPrintf.cpp
	../CoreMediaIO/PublicUtility/CMIODebugMacros.cpp
	../CoreAudio/PublicUtility/CAVolumeCurve.cpp
	../CoreAudio/PublicUtility/CAProcess.cpp
	../CoreAudio/PublicUtility/CAMutex.cpp
	../CoreAudio/PublicUtility/CAHostTimeBase.cpp
	../CoreAudio/PublicUtility/CACFString.cpp
	../CoreAudio/PublicUtility/CACFPreferences.cpp
	../CoreAudio/PublicUtility/CACFNumber.cpp
	../CoreAudio/PublicUtility/CACFMachPort.cpp
	../CoreAudio/PublicUtility/CACFDictionary.cpp
	../generated/CMIODPASampleClient.cpp
	../CoreAudio/PublicUtility/CACFArray.cpp)

add_library(mac-virtualcam-plugin MODULE
	${mac-virtualcam-plugin_SOURCES}
	${mac-virtualcam-plugin_HEADERS})

set_target_properties(mac-virtualcam-plugin
	PROPERTIES
	BUNDLE TRUE)

target_link_libraries(mac-virtualcam-plugin
	${AVFOUNDATION}
	${COCOA}
	${COREFOUNDATION}
	${COREMEDIA}
	${COREVIDEO}
	${COREMEDIAIO}
	${IOSURFACE}
	${IOKIT})

# Note: May not be necessary
target_link_options(mac-virtualcam-plugin
	PUBLIC -exported_symbols_list ${PROJECT_SOURCE_DIR}/../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/SampleVCam.exp
)

# Workaround because building from Xcode likes to put things in Debug and Release directories
# TODO: This only supports Debug
if (CPACK_CMAKE_GENERATOR STREQUAL "Xcode")
	set(TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/Debug")
else (CPACK_CMAKE_GENERATOR STREQUAL "Xcode")
	set(TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif (CPACK_CMAKE_GENERATOR STREQUAL "Xcode")

add_custom_command(TARGET mac-virtualcam-plugin
	POST_BUILD
	COMMAND rm -rf ${TARGET_DIR}/mac-virtualcam-plugin.plugin || true
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_DIR}/mac-virtualcam-plugin.bundle ${TARGET_DIR}/mac-virtualcam-plugin.plugin
	COMMENT "Rename bundle to plugin"
)

# Note: Xcode seems to run a command `builtin-infoPlistUtility` to generate the Info.plist, but I'm
# not sure where to find that binary. If we had access to it, the command would look something like:
# builtin-infoPlistUtility ${PROJECT_SOURCE_DIR}/../CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/SampleVCam-Info.plist -producttype com.apple.product-type.bundle -expandbuildsettings -platform macosx -o mac-virtualcam-plugin.bundle/Contents/Info.plist
# Instead, just copy in one that was already generated from Xcode.
add_custom_command(TARGET mac-virtualcam-plugin
	POST_BUILD
	COMMAND cp ${PROJECT_SOURCE_DIR}/Info.plist ${TARGET_DIR}/mac-virtualcam-plugin.plugin/Contents/Info.plist
	DEPENDS {PROJECT_SOURCE_DIR}/Info.plist
	COMMENT "Copy in Info.plist"
)

add_custom_command(TARGET mac-virtualcam-plugin
	POST_BUILD
	COMMAND /usr/bin/codesign --force --deep --sign - --timestamp=none ${TARGET_DIR}/mac-virtualcam-plugin.plugin
	COMMENT "Codesign plugin"
)
