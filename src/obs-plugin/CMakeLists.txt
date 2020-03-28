project(mac-virtualcam)

find_library(AVFOUNDATION AVFoundation)
find_library(COCOA Cocoa)
find_library(COREFOUNDATION CoreFoundation)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COCOA Cocoa)
find_library(COREMEDIAIO CoreMediaIO)
find_library(IOSURFACE IOSurface)
find_library(IOKIT IOKit)

include_directories(${AVFOUNDATION}
                    ${COCOA}
                    ${COREFOUNDATION}
                    ${COREMEDIA}
                    ${COREVIDEO}
                    ${COREMEDIAIO}
                    ${COCOA}
                    ${IOSURFACE}
                    CoreMediaIO/PublicUtility
                    CoreAudio/PublicUtility
                    CoreMediaIO/PublicUtility/IOSurfaceAssistant
                    CoreMediaIO/PublicUtility/POSIXThreadAssistant
                    CoreMediaIO/PublicUtility/IOVideoAssistant
                    CoreMediaIO/PublicUtility/IOStreamAssistant
                    CoreMediaIO/PublicUtility/IOKitAssistant
                    CoreMediaIO/PublicUtility/CoreMediaAssistant
                    CoreMediaIO/PublicUtility/CoreVideoAssistant
                    CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties
                    CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base
                    CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn
                    CoreMediaIO/DeviceAbstractionLayer/Devices/Sample
                    CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant
                    CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server
                    CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device
                    generated)

set(mac-virtualcam_HEADERS
	CoreMediaIO/PublicUtility/IOSurfaceAssistant/CMIO_SA_Assistance.h
	CoreMediaIO/PublicUtility/POSIXThreadAssistant/CMIO_PTA_NotificationPortThread.h
	CoreMediaIO/PublicUtility/IOVideoAssistant/CMIO_IOVA_Assistance.h
	CoreMediaIO/PublicUtility/IOStreamAssistant/CMIO_IOSA_Assistance.h
	CoreMediaIO/PublicUtility/IOKitAssistant/CMIO_IOKA_NotificationPort.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_FirstOutputPresentationTimeStamp.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Deck.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Base.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_ControlDictionary.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn/CMIO_DP_IOV_ControlDictionary.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_VCamInputStream.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Stream.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_Server_VCamAssistant.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_VCamDevice.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_Server_MIGInterface.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Frame.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Deck.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Device.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_ClientStream.h
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_Server_Assistant.h
	CoreMediaIO/PublicUtility/DALAssistant/CMIO_DALA_Object.h
	CoreMediaIO/PublicUtility/CMIODebugMacros.h
	CoreAudio/PublicUtility/CAProcess.h
	CoreAudio/PublicUtility/CAPThread.h
	CoreAudio/PublicUtility/CAMutex.h
	CoreAudio/PublicUtility/CAHostTimeBase.h
	CoreAudio/PublicUtility/CAGuard.h
	CoreAudio/PublicUtility/CACFString.h
	CoreAudio/PublicUtility/CACFNumber.h
	CoreAudio/PublicUtility/CACFDictionary.h
	CoreAudio/PublicUtility/CACFArray.h)

set(mac-virtualcam_SOURCES
	CoreMediaIO/PublicUtility/IOSurfaceAssistant/CMIO_SA_Assistance.cpp
	CoreMediaIO/PublicUtility/POSIXThreadAssistant/CMIO_PTA_NotificationPortThread.cpp
	CoreMediaIO/PublicUtility/IOVideoAssistant/CMIO_IOVA_Assistance.cpp
	CoreMediaIO/PublicUtility/IOStreamAssistant/CMIO_IOSA_Assistance.cpp
	CoreMediaIO/PublicUtility/IOKitAssistant/CMIO_IOKA_NotificationPort.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_FirstOutputPresentationTimeStamp.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Deck.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_VCamServer.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Properties/CMIO_DP_Property_Base.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/DP/Base/CMIO_DP_ControlDictionary.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/IOV/PlugIn/CMIO_DP_IOV_ControlDictionary.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_VCamInputStream.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Stream.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_Server_VCamAssistant.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_VCamDevice.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_Server_MIGInterface.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Frame.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Deck.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_Device.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/Device/CMIO_DPA_Sample_Server_ClientStream.cpp
	CoreMediaIO/DeviceAbstractionLayer/Devices/Sample/Assistant/Server/CMIO_DPA_Sample_Server_Assistant.cpp
	CoreMediaIO/PublicUtility/DALAssistant/CMIO_DALA_Object.cpp
	CoreMediaIO/PublicUtility/CMIODebugMacros.cpp
	CoreAudio/PublicUtility/CAProcess.cpp
	CoreAudio/PublicUtility/CAPThread.cpp
	CoreAudio/PublicUtility/CAMutex.cpp
	CoreAudio/PublicUtility/CAHostTimeBase.cpp
	CoreAudio/PublicUtility/CAGuard.cpp
	CoreAudio/PublicUtility/CACFString.cpp
	CoreAudio/PublicUtility/CACFNumber.cpp
	CoreAudio/PublicUtility/CACFDictionary.cpp
	CoreAudio/PublicUtility/CACFArray.cpp
	generated/CMIODPASampleServer.cpp
    plugin-main.cpp)


#set_source_files_properties(av-virtualcam.mm
#	PROPERTIES COMPILE_FLAGS "-fobjc-arc")
	
add_library(mac-virtualcam MODULE
	${mac-virtualcam_SOURCES}
	${mac-virtualcam_HEADERS})

target_link_libraries(mac-virtualcam
	libobs
	${AVFOUNDATION}
	${COCOA}
	${COREFOUNDATION}
	${COREMEDIA}
	${COREVIDEO}
	${COREMEDIAIO}
	${IOSURFACE}
	${IOKIT})

install_obs_plugin_with_data(mac-virtualcam data)
