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
                    ../common/CoreMediaIO/PublicUtility
                    ../common/CoreAudio/PublicUtility
                    ../common/CoreMediaIO/PublicUtility/IOSurfaceAssistant
                    ../common/CoreMediaIO/PublicUtility/POSIXThreadAssistant
                    ../common/CoreMediaIO/PublicUtility/IOVideoAssistant
                    ../common/CoreMediaIO/PublicUtility/IOStreamAssistant
                    ../common/CoreMediaIO/PublicUtility/IOKitAssistant
                    ../common/CoreMediaIO/PublicUtility/CoreMediaAssistant
                    ../common/CoreMediaIO/PublicUtility/CoreVideoAssistant
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device
                    # Verified to be necessary
                    ../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Client
                    ../common/CoreMediaIO/PublicUtility/DALAssistant
                    ./)

set(mac-virtualcam-plugin_SOURCES
	../common/CoreMediaIO/PublicUtility/IOSurfaceAssistant/CMIO_SA_Assistance.cpp
	../common/CoreMediaIO/PublicUtility/POSIXThreadAssistant/CMIO_PTA_NotificationPortThread.cpp
	../common/CoreMediaIO/PublicUtility/IOStreamAssistant/CMIO_IOSA_Assistance.cpp
	../common/CoreMediaIO/PublicUtility/IOKitAssistant/CMIO_IOKA_NotificationPort.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_ScheduledOutputNotificationProc.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Deck.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_FirstOutputPresentationTimeStamp.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_ControlDictionary.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_ClientStream.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Frame.cpp
	../common/CoreAudio/PublicUtility/CACFDistributedNotification.cpp
	../common/CoreAudio/PublicUtility/CAGuard.cpp
	../common/CoreMediaIO/PublicUtility/CMIO_SMPTETimeBase.cpp
	../common/CoreAudio/PublicUtility/CAPThread.cpp
	../common/CoreMediaIO/PublicUtility/POSIXThreadAssistant/CMIO_PTA_CFMachPortThread.cpp
	../common/CoreMediaIO/PublicUtility/IOVideoAssistant/CMIO_IOVA_Assistance.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_UserInfo.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Stream.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_VirtualDevice.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_Stream.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/Properties/CMIO_DP_Sample_Property_HogMode.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_PlugIn.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_IOBackedDevice.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_Sample_Device.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/CMIO_DP_SampleVCam_PlugInInterface.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_SuspendedByUser.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_SMPTETimeCallback.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_OutputBuffers.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_NoData.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_HogMode.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_FormatList.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_EndOfData.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_DeviceMaster.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Clock.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_ClientSyncDiscontinuity.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Base.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_PlugIn.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Object.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn/CMIO_DP_IOV_ControlDictionary.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn/CMIO_DP_IOV_Control.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_HardwarePlugInInterface.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_DeviceSettings.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_DeviceCommands.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Device.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Control.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_Command.cpp
	../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Client/CMIO_DPA_Sample_ClientExtras.cpp
	../common/CoreMediaIO/PublicUtility/DALAssistant/CMIO_DALA_System.cpp
	../common/CoreMediaIO/PublicUtility/CMIODebugPrintf.cpp
	../common/CoreMediaIO/PublicUtility/CMIODebugMacros.cpp
	../common/CoreAudio/PublicUtility/CAVolumeCurve.cpp
	../common/CoreAudio/PublicUtility/CAProcess.cpp
	../common/CoreAudio/PublicUtility/CAMutex.cpp
	../common/CoreAudio/PublicUtility/CAHostTimeBase.cpp
	../common/CoreAudio/PublicUtility/CACFString.cpp
	../common/CoreAudio/PublicUtility/CACFPreferences.cpp
	../common/CoreAudio/PublicUtility/CACFNumber.cpp
	../common/CoreAudio/PublicUtility/CACFMachPort.cpp
	../common/CoreAudio/PublicUtility/CACFDictionary.cpp
	../common/CoreAudio/PublicUtility/CACFArray.cpp
	CMIODPASampleClient.cpp)

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
	PUBLIC -exported_symbols_list ${PROJECT_SOURCE_DIR}/../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/SampleVCam.exp
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
# builtin-infoPlistUtility ${PROJECT_SOURCE_DIR}/../common/CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/PlugIn/SampleVCam-Info.plist -producttype com.apple.product-type.bundle -expandbuildsettings -platform macosx -o mac-virtualcam-plugin.bundle/Contents/Info.plist
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
