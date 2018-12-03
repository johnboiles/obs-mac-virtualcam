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
        
        mSequenceFile = fopen("/Library/CoreMediaIO/Plug-Ins/DAL/SampleVCam.plugin/Contents/Resources/ntsc2vuy720x480.yuv", "rb");
        mFrameSize = 720 * 480 * 2;

        fseek(mSequenceFile, 0, SEEK_END);
        mFrameCount = ftell(mSequenceFile) / mFrameSize;
        
        pthread_create(&mThread, NULL, &VCamDevice::EmitFrame, this);
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~VCamDevice()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	VCamDevice::~VCamDevice()
	{
        fclose(mSequenceFile);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//  CreateStreams()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void VCamDevice::CreateStreams()
	{
        UInt32 streamID = 0;
        
        CACFDictionary format;
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_CodecType), kYUV422_720x480);
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_CodecFlags), kSampleCodecFlags_30fps | kSampleCodecFlags_1001_1000_adjust);
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_Width), 720);
        format.AddUInt32(CFSTR(kIOVideoStreamFormatKey_Height), 480);

        CACFArray formats;
        formats.AppendDictionary(format.GetDict());

        CACFDictionary streamDict;
        streamDict.AddArray(CFSTR(kIOVideoStreamKey_AvailableFormats), formats.GetCFArray());

        mInputStream = new VCamInputStream(this, streamDict.GetDict(), kCMIODevicePropertyScopeInput);
        mInputStreams[streamID] = mInputStream;
    }
    
    #pragma mark -
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    //  EmitFrame()
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void* VCamDevice::EmitFrame(void* device) {
        VCamDevice* vcamDevice = (VCamDevice*)device;
        uint8_t* framebuffer = new uint8_t[vcamDevice->mFrameSize];
        
        while (true) {
            usleep(1000 * 1000 / 120);
            
            fseek(vcamDevice->mSequenceFile, (vcamDevice->mFrameIndex % vcamDevice->mFrameCount) * vcamDevice->mFrameSize, SEEK_SET);
            fread(framebuffer, 1, vcamDevice->mFrameSize, vcamDevice->mSequenceFile);
            ++vcamDevice->mFrameIndex;

            UInt64 vbiTime = vcamDevice->mInputStream->GetTimecode() * 1000000000.0;
            vcamDevice->mInputStream->FrameArrived(vcamDevice->mFrameSize, framebuffer, vbiTime);
        }
        
        return NULL;
    }

}}}}
