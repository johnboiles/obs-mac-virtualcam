/*
	    File: CMIO_DPA_Sample_Server_VCamDevice.cpp
	Abstract: n/a
	 Version: 1.2
 
*/

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DPA_Sample_Server_VCamDevice.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_VCamInputStream.h"
#include "CAHostTimeBase.h"


//#include <obs.h>
#include "CMIO_DPA_Sample_Server_Stream.h"


namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	#pragma mark -
	#pragma mark VCamDevice
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// VCamDevice()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	VCamDevice::VCamDevice() :
        Device()
	{
		CreateStreams();
        mFrameSize = getObsOutputWidth() * getObsOutputHeight() * 2;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~VCamDevice()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	VCamDevice::~VCamDevice()
	{
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  CreateStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void VCamDevice::CreateStreams()
	{
        UInt32 streamID = 0;
    

        CACFDictionary format;
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_CodecType), CMIO::DPA::Sample::kYUV422_ALL);
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_CodecFlags), kSampleCodecFlags_30fps | kSampleCodecFlags_1001_1000_adjust);  //TODO FPS
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_Width), getObsOutputWidth());
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_Height), getObsOutputHeight());

        CACFArray formats;
        formats.AppendDictionary(format.GetDict());

        CACFDictionary streamDict;
        streamDict.AddArray(CFSTR(kIOVideoStreamKey_AvailableFormats), formats.GetCFArray());
        streamDict.AddUInt32(CFSTR(kIOVideoStreamKey_StartingDeviceChannelNumber), 1);

        mInputStream = new VCamInputStream(this, streamDict.GetDict(), kCMIODevicePropertyScopeInput);
        mInputStreams[streamID] = mInputStream;
    }
}}}}