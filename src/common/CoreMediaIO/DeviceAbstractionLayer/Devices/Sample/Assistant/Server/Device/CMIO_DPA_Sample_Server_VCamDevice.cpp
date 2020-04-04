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


//Probably too many includes, I'm sorry -- gxalpha
#include <iostream>
#include <obs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
using namespace std;


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
        
        obs_video_info ovi;
        obs_get_video_info(&ovi);
        mFrameSize = ovi.output_width * ovi.output_height * 2;
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
        
        obs_video_info ovi;
        obs_get_video_info(&ovi);
        stringstream stream;
        stream << ovi.output_width << "x" << ovi.output_height;
        string res = stream.str();
        
        FrameType frametype;
        
        //If-Ladder, yay!
        if (strcmp("720x480", res.c_str())==0) {
            frametype = kYUV422_720x480;
        } else if (strcmp("720x486", res.c_str())==0) {
            frametype = kYUV422_720x486;
        } else if (strcmp("720x576", res.c_str())==0) {
            frametype = kYUV422_720x576;
        } else if (strcmp("1280x720", res.c_str())==0) {
            frametype = kYUV422_1280x720;
        } else if (strcmp("1920x1080", res.c_str())==0) {
            frametype = kYUV422_1920x1080;
        } else {
            //ERROR
        }
        
        
        CACFDictionary format;
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_CodecType), frametype);
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_CodecFlags), kSampleCodecFlags_30fps | kSampleCodecFlags_1001_1000_adjust);  //TODO FPS
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_Width), ovi.output_width);
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_Height), ovi.output_height);

        CACFArray formats;
        formats.AppendDictionary(format.GetDict());

        CACFDictionary streamDict;
        streamDict.AddArray(CFSTR(kIOVideoStreamKey_AvailableFormats), formats.GetCFArray());
        streamDict.AddUInt32(CFSTR(kIOVideoStreamKey_StartingDeviceChannelNumber), 1);

        mInputStream = new VCamInputStream(this, streamDict.GetDict(), kCMIODevicePropertyScopeInput);
        mInputStreams[streamID] = mInputStream;
    }
}}}}
