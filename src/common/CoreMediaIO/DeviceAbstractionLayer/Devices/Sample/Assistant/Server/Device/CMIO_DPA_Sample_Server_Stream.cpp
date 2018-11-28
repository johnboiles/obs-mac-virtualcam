/*
	    File: CMIO_DPA_Sample_Server_Stream.cpp
	Abstract: n/a
	 Version: 1.2
	
	Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
	Inc. ("Apple") in consideration of your agreement to the following
	terms, and your use, installation, modification or redistribution of
	this Apple software constitutes acceptance of these terms.  If you do
	not agree with these terms, please do not use, install, modify or
	redistribute this Apple software.
	
	In consideration of your agreement to abide by the following terms, and
	subject to these terms, Apple grants you a personal, non-exclusive
	license, under Apple's copyrights in this original Apple software (the
	"Apple Software"), to use, reproduce, modify and redistribute the Apple
	Software, with or without modifications, in source and/or binary forms;
	provided that if you redistribute the Apple Software in its entirety and
	without modifications, you must retain this notice and the following
	text and disclaimers in all such redistributions of the Apple Software.
	Neither the name, trademarks, service marks or logos of Apple Inc. may
	be used to endorse or promote products derived from the Apple Software
	without specific prior written permission from Apple.  Except as
	expressly stated in this notice, no other rights or licenses, express or
	implied, are granted by Apple herein, including but not limited to any
	patent rights that may be infringed by your derivative works or by other
	works in which the Apple Software may be incorporated.
	
	The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
	MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
	THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
	OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
	
	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
	OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
	AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
	STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
	
	Copyright (C) 2012 Apple Inc. All Rights Reserved.
	
*/

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DPA_Sample_Server_Stream.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_ClientStream.h"
#include "CMIO_DPA_Sample_Server_Device.h"
#include "CMIO_DPA_Sample_Server_Frame.h"
#include "CMIO_DPA_Sample_Server_Deck.h"
#include "CMIO_DPA_Sample_Shared.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_PTA_NotificationPortThread.h"

// CA Public Utility Includes
#include "CACFNumber.h"
#include "CAHostTimeBase.h"

// System Includes
#include <IOKit/video/IOVideoTypes.h>
#include <CoreMediaIO/CMIOHardware.h>
#include "CMIO_CVA_Pixel_Buffer.h"
#include "CMIO_SA_Assistance.h"
#include <mach/mach.h>

#define kMaxRequestsPerCallback 4

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	#pragma mark -
	#pragma mark Stream
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::Stream(Device* device, IOKA::Object& registryEntry, CFDictionaryRef streamDictionary, CMIOObjectPropertyScope scope) :
		mDevice(device),
		mRegistryEntry(registryEntry),
		mStreamDictionary(static_cast<CFDictionaryRef>(NULL), true),
        mIOSPAlugIn(IOSA::AllocatePlugIn(mRegistryEntry)),
        mIOSAStream(IOSA::AllocateStream(mIOSPAlugIn)),
		mStateMutex("CMIO::DPA::Sample::Stream state mutex"),
		mIsInput(kCMIODevicePropertyScopeInput == scope),
		mStreaming(false),
		mInOutputCallBack(false),
		mDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities),
		mOutputBuffer(),
		mOutputBufferRequestPort(MACH_PORT_NULL),
		mOutputClient(MACH_PORT_NULL),
		mProperties(),
		mNoDataTimeout(250),
		mNoDataEventCount(0),
		mNoDataNotificationHasOccured(false),
		mDeviceSyncTimeout(0),
		mDeviceSyncTimeoutChanged(false),
		mDeviceSyncCount(0),
		mEndOfData(false),
		mUnderrunCount(0),
		mFrameFormats(),
		mFrameType(kYUV422_10_720x486),
		mFrameRatesMap(),
		mFrameRate(30000.0 / 1001.0),
		mNominalFrameDuration(CMTimeMake(1001, 30000)),
		mClientStreams(),
		mClientStreamsMutex("CMIO::DPA::Sample::Server::Stream client streams mutex"),
		mFrameAvailableGuard("frame available guard"),
		mDeck(*this)
	{
		mStreamDictionary = streamDictionary;
		// Specify the stream's callback
		mIOSAStream.SetOutputCallback(reinterpret_cast<IOStreamOutputCallback>(StreamOutputCallback), this);
				
		// Add the stream's run loop source to the notification thread's run loop
		mIOSAStream.AddToRunLoop(GetOwningDevice().GetNotificationThread().GetRunLoop());

		UInt32 element = GetStartingDeviceChannelNumber();
		UInt64 shadowTime = CAHostTimeBase::GetTheCurrentTime();

		if (IsInput())
		{
			// This stream only has a single format that provides frames @ 29.97 fps.
			// (Note:  this should really be extracted from the the stream dictionary)
			
			// Get the available format list from the the dictionary
			CACFArray formatList = CACFArray(static_cast<CFArrayRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_AvailableFormats))), false);
			UInt32 formatCount = formatList.GetNumberItems();
			
			for (UInt32 index = 0 ; index < formatCount ; ++index)
			{
				// Get the control dictionary
				CFDictionaryRef	formatDictionary = NULL;
				
				if (formatList.GetDictionary(index, formatDictionary))
				{
					CACFDictionary formatCACFDictionary = CACFDictionary(formatDictionary, false);
					if (formatCACFDictionary.IsValid())
					{
						UInt32 codecType, codecFlags, formatWidth, formatHeight;
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_CodecType), codecType);
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_CodecFlags), codecFlags);
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_Width), formatWidth);
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_Height), formatHeight);
						
						switch (codecType)
						{
							case kYUV422_720x480:
								{
									mFrameFormats.insert(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMVideoCodecType_422YpCbCr8, formatWidth, formatHeight));
									mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1001.0)] = CMTimeMake(1000, 30001);
								}
								break;
							case kYUV422_1280x720:
							{
								mFrameFormats.insert(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMVideoCodecType_422YpCbCr8, formatWidth, formatHeight));
								mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1001.0)] = CMTimeMake(1000, 30001);
							}
								break;
							case kYUV422_1920x1080:
							{
								mFrameFormats.insert(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMVideoCodecType_422YpCbCr8, formatWidth, formatHeight));
								mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1001.0)] = CMTimeMake(1000, 30001);
							}
								break;
						}
						
					}
				}
			}
			
			mProperties[PropertyAddress(kCMIOStreamPropertyFormatDescriptions, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyFormatDescription, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyFrameRates, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyNoDataTimeoutInMSec, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyNoDataEventCount, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyDeviceSyncTimeoutInMSec, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			
			mProperties[PropertyAddress(kCMIOStreamPropertyDeckFrameNumber, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyDeck, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyDeckCueing, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
		}
		else
		{
			// (Note:  this should really be extracted from the the stream dictionary)
			CACFArray formatList = CACFArray(static_cast<CFArrayRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_AvailableFormats))), false);
			UInt32 formatCount = formatList.GetNumberItems();
			
			for (UInt32 index = 0 ; index < formatCount ; ++index)
			{
				// Get the control dictionary
				CFDictionaryRef	formatDictionary = NULL;
				
				if (formatList.GetDictionary(index, formatDictionary))
				{
					CACFDictionary formatCACFDictionary = CACFDictionary(formatDictionary, false);
					if (formatCACFDictionary.IsValid())
					{
						UInt32 codecType, codecFlags, formatWidth, formatHeight;
                        std::set<FrameFormat>::iterator it;
                        
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_CodecType), codecType);
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_CodecFlags), codecFlags);
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_Width), formatWidth);
						formatCACFDictionary.GetUInt32(CFSTR(kIOVideoStreamFormatKey_Height), formatHeight);
						switch(codecType)
						{
							case kYUV422_720x480:
                            case kYUV422_720x486:
                            case kYUV422_720x576:
                            case kYUV422_1280x720:
                            case kYUV422_1920x1080:
							{
                                it = mFrameFormats.find(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMVideoCodecType_422YpCbCr8, formatWidth, formatHeight));
                               
							}
								break;
                            case kYUV422_10_720x480:
                            case kYUV422_10_720x486:
                            case kYUV422_10_720x576:
                            case kYUV422_10_1280x720:
                            case kYUV422_10_1920x1080:
							{
                                it = mFrameFormats.find(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMPixelFormat_422YpCbCr10, formatWidth, formatHeight));
                                
                            }
                                break;
                               
                        }
                        if (it !=mFrameFormats.end())
                        {
                             Float64 frameRate = CodecFlagsToFrameRate(codecFlags);
                            int frameRateInt = frameRate;
                            switch(frameRateInt)
                            {
                                case 60:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(60000.0 / 1000.0)] = CMTimeMake(1000, 60000);
                                    mNominalFrameDuration = CMTimeMake(1000, 60000); 
                                }
                                break;
                                case 59:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(60000.0 / 1001.0)] = CMTimeMake(1001, 60000);
                                    mNominalFrameDuration = CMTimeMake(1001, 60000); 
                                }
                                    break;
                                case 50:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(5000.0 / 100.0)] = CMTimeMake(100, 5000);
                                    mNominalFrameDuration = CMTimeMake(100, 5000); 
                                }
                                    break;
                                case 30:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1000.0)] = CMTimeMake(1000, 30000);
                                    mNominalFrameDuration = CMTimeMake(1000, 30000); 
                                 }
                                    break;
                                case 29:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1001.0)] = CMTimeMake(1001, 30000);
                                    mNominalFrameDuration = CMTimeMake(1001, 30000); 
                                }
                                    break;
                                case 25:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(2500.0 / 100.0)] = CMTimeMake(100, 2500);
                                    mNominalFrameDuration = CMTimeMake(100, 2500); 
                                }
                                    break;
                                case 24:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(24000.0 / 1000.0)] = CMTimeMake(1000, 24000);
                                    mNominalFrameDuration = CMTimeMake(1000, 24000); 
                                }
                                    break;
                                case 23:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(24000.0 / 1001.0)] = CMTimeMake(1001, 24000);
                                    mNominalFrameDuration = CMTimeMake(1001, 24000); 
                                }
                                    break;
                                    
                            }
                           
                        }
                        else
                        {
                            switch(codecType)
                            {
                                case kYUV422_720x480:
                                case kYUV422_720x486:
                                case kYUV422_720x576:
                                case kYUV422_1280x720:
                                case kYUV422_1920x1080:
                                {
                                    mFrameFormats.insert(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMVideoCodecType_422YpCbCr8, formatWidth, formatHeight));
                                    
                                }
                                    break;
                                case kYUV422_10_720x480:
                                case kYUV422_10_720x486:
                                case kYUV422_10_720x576:
                                case kYUV422_10_1280x720:
                                case kYUV422_10_1920x1080:
                                {
                                    mFrameFormats.insert(FrameFormat((CMIO::DPA::Sample::FrameType)codecType, kCMPixelFormat_422YpCbCr10, formatWidth, formatHeight));                                    
                                }
                                    break;
                                    
                            }
                            
                            Float64 frameRate = CodecFlagsToFrameRate(codecFlags);
                            int frameRateInt = frameRate;
                            switch(frameRateInt)
                            {
                                case 60:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(60000.0 / 1000.0)] = CMTimeMake(1000, 60000);
                                    mNominalFrameDuration = CMTimeMake(1000, 60000); 
                                }
                                    break;
                                case 59:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(60000.0 / 1001.0)] = CMTimeMake(1001, 60000);
                                    mNominalFrameDuration = CMTimeMake(1001, 60000); 
                                }
                                    break;
                                case 50:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(5000.0 / 100.0)] = CMTimeMake(100, 5000);
                                    mNominalFrameDuration = CMTimeMake(100, 5000); 
                                }
                                    break;
                                case 30:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1000.0)] = CMTimeMake(1000, 30000);
                                    mNominalFrameDuration = CMTimeMake(1000, 30000); 
                                }
                                    break;
                                case 29:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(30000.0 / 1001.0)] = CMTimeMake(1001, 30000);
                                    mNominalFrameDuration = CMTimeMake(1001, 30000); 
                                }
                                    break;
                                case 25:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(2500.0 / 100.0)] = CMTimeMake(100, 2500);
                                    mNominalFrameDuration = CMTimeMake(100, 2500); 
                                }
                                    break;
                                case 24:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(24000.0 / 1000.0)] = CMTimeMake(1000, 24000);
                                    mNominalFrameDuration = CMTimeMake(1000, 24000); 
                                }
                                    break;
                                case 23:
                                {
                                    mFrameRatesMap[(CMIO::DPA::Sample::FrameType)codecType][(24000.0 / 1001.0)] = CMTimeMake(1001, 24000);
                                    mNominalFrameDuration = CMTimeMake(1001, 24000); 
                                }
                                    break;
                                    
                            }
                        }
					}
				}
			}

			mProperties[PropertyAddress(kCMIOStreamPropertyFormatDescriptions, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyFormatDescription, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyFrameRates, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyEndOfData, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyOutputBufferQueueSize, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyOutputBuffersRequiredForStartup, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyOutputBufferUnderrunCount, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
			mProperties[PropertyAddress(kCMIOStreamPropertyOutputBufferRepeatCount, GetDevicePropertyScope(), element)].mShadowTime = shadowTime;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~Stream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Stream::~Stream()
	{
		if (mIOSAStream.IsValid())
		{
			// Remove the stream's callback
			(void) (**mIOSAStream).SetOutputCallback(mIOSAStream, NULL, NULL);

			// Remove the stream from the notification thread's run loop
			(void) (**mIOSAStream).RemoveFromRunLoop(mIOSAStream, GetOwningDevice().GetNotificationThread().GetRunLoop());
		}

		// Release the Mach port from which frames were requested
		if (MACH_PORT_NULL != mOutputBufferRequestPort)
			(void) mach_port_deallocate(mach_task_self(), mOutputBufferRequestPort);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStartingDeviceChannelNumber()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::GetStartingDeviceChannelNumber() const
	{
		return CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_StartingDeviceChannelNumber))), false).GetSInt32();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream::GetCurrentNumberChannels()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Stream::GetCurrentNumberChannels() const
	{
		#warning CMIO::DPA::Sample::Server::Stream::GetCurrentNumberChannels() should decided how to report multiple channels...currently always reporting 1
		return 1;
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::GetProperties(UInt64 time, const PropertyAddress& matchAddress, PropertyAddressList& matches) const
	{
		// Iterate over the properties and check for matches
		for (Properties::const_iterator i = mProperties.begin() ; i != mProperties.end() ; std::advance(i, 1))
		{
			// Skip the address if its shadow time is less than the indicated time or the addresses are not congruent 
			if (((*i).second.mShadowTime < time) or (not PropertyAddress::IsCongruentAddress(matchAddress, (*i).first)))
				continue;

			// Add the address to the matches
			matches.AppendItem((*i).first);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetNoDataTimeout()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetNoDataTimeout(UInt32 noDataTimeout)
	{
		// Don't do anything if there are no changes
		if (noDataTimeout == mNoDataTimeout)
			return;
			
		mProperties[PropertyAddress(kCMIOStreamPropertyNoDataTimeoutInMSec, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
		mNoDataTimeout = noDataTimeout;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceSyncTimeout()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetDeviceSyncTimeout(UInt32 deviceSyncTimeout)
	{
		// Don't do anything if there are no changes
		if (deviceSyncTimeout == mDeviceSyncTimeout)
			return;
			
		mProperties[PropertyAddress(kCMIOStreamPropertyDeviceSyncTimeoutInMSec, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
		mDeviceSyncTimeout = deviceSyncTimeout;	
		mDeviceSyncTimeoutChanged = true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetEndOfData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetEndOfData(bool endOfData)
	{
		// Don't do anything if there are no changes
		if (endOfData == mEndOfData)
			return;
			
		mProperties[PropertyAddress(kCMIOStreamPropertyEndOfData, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
		mEndOfData = endOfData;
	}

	#pragma mark -

    UInt32  Stream::FrameRateToCodecFlags(Float64 framerate)
    {
        UInt32 codecFlags = 0;
        
        if (framerate > 0.0)
        {
            UInt32 intFrameRate = framerate;
            switch(intFrameRate)
            {
                case 60:
                {
                    codecFlags = kSampleCodecFlags_60fps; 
                }
                    break;
                case 59:
                {
                    codecFlags = kSampleCodecFlags_60fps + kSampleCodecFlags_1001_1000_adjust; 
                 }
                    break;
                case 50:
                {
                    codecFlags = kSampleCodecFlags_50fps; 
                }
                    break;
                case 30:
                {
                    codecFlags = kSampleCodecFlags_30fps; 
                }
                    break;
               case 29:
                {
                    codecFlags = kSampleCodecFlags_30fps + kSampleCodecFlags_1001_1000_adjust; 
                 }
                    break;
               case 25:
                {
                    codecFlags = kSampleCodecFlags_25fps; 
                }
                    break;
                case 24:
                {
                    codecFlags = kSampleCodecFlags_24fps; 
                }
                case 23:
                {
                    codecFlags = kSampleCodecFlags_24fps + kSampleCodecFlags_1001_1000_adjust; 
                }
                    break;                    
            }
        }
        return codecFlags;
    }

    Float64 Stream::CodecFlagsToFrameRate(UInt32 codecFlags)
    {
        Float64 frameRate= 0.0;
        
        switch(codecFlags)
        {
            case kSampleCodecFlags_60fps:
            {
                frameRate = 60.0; 
            }
                break;
            case kSampleCodecFlags_60fps+kSampleCodecFlags_1001_1000_adjust:
            {
                frameRate = 60.0*1000/1001; 
            }
                break;
            case kSampleCodecFlags_50fps:
            {
                frameRate = 50.0; 
            }
                break;
            case kSampleCodecFlags_30fps:
            {
                frameRate = 30.0; 
            }
                break;
            case kSampleCodecFlags_30fps+kSampleCodecFlags_1001_1000_adjust:
            {
                frameRate = 30.0*1000/1001; 
            }
                break;
            case kSampleCodecFlags_25fps:
            {
                frameRate = 25.0; 
            }
                break;
            case kSampleCodecFlags_24fps:
            {
                frameRate = 24.0; 
            }
            case kSampleCodecFlags_24fps+kSampleCodecFlags_1001_1000_adjust:
            {
                frameRate = 24.0*1000/1001; 
            }
                break;                    
        }
       
        return frameRate;
    }

	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFrameFormats()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	vm_size_t Stream::GetFrameFormats(FrameFormat** formats) const
	{
		vm_size_t size = sizeof(FrameFormat) * mFrameFormats.size();
		ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(formats), size, true), CAException(-1), "Stream::GetFrameFormats: allocation failed for kCMIOStreamPropertyFormatDescriptions");

		// Fill in the array
		int index = 0;
		for (FrameFormats::const_iterator i = mFrameFormats.begin() ; i != mFrameFormats.end() ; ++i)
			(*formats)[index++] = *i;
			
		return size;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetFrameType()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetFrameType(FrameType frameType)
	{
		// No need to do anything if the frame type hasn't changed
		if (frameType == mFrameType)
			return;
			
		// Grab the mutex for the overall device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Program the device settings (which starts the stream) if currently streaming
		bool programSettings = mStreaming;
		
		// Stop the stream if needed
		if (mStreaming)
		{
			Stop(MACH_PORT_NULL);
		}
		
		// Save the current frame type & frame rate control in the event of an error so a restore attempt can be made
		FrameType currentFrameType = mFrameType;
		Float64 currentFrameRate = mFrameRate;
		

        FrameRates::const_iterator i = mFrameRatesMap[frameType].find(mFrameRate);
        if (i == mFrameRatesMap[frameType].end())
        {
            //the new frametype doesn't have the existing frame rate so grab the fastest
            mFrameRate = (*mFrameRatesMap[frameType].rbegin()).first;           
        }
		// Remember the new frame type
		mFrameType = frameType;
        mNominalFrameDuration = mFrameRatesMap[mFrameType][mFrameRate];

		// Set the frame rate to the fastest possible for this frame type
//		mFrameRate = (*mFrameRatesMap[mFrameType].rbegin()).first;
		
		if (programSettings)
		{
			try
			{
				// DoWhatYouWouldNeedToDoToProgramTheHardware();
				// Send message to kext here to change the format
				IOVideoStreamDescription theNewFormat;
				switch(mFrameType)
				{
					case kYUV422_720x480:
						theNewFormat.mVideoCodecType = kYUV422_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;

					case kYUV422_720x486:
						theNewFormat.mVideoCodecType = kYUV422_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_720x576:
						theNewFormat.mVideoCodecType = kYUV422_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;

					case kYUV422_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
				
					case kYUV422_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_1920x1080; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;

                    case kYUV422_10_720x480:
						theNewFormat.mVideoCodecType = kYUV422_10_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
                        
					case kYUV422_10_720x486:
						theNewFormat.mVideoCodecType = kYUV422_10_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_10_720x576:
						theNewFormat.mVideoCodecType = kYUV422_10_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_10_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_10_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
						
					case kYUV422_10_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_10_1920x1080; 
						theNewFormat.mVideoCodecFlags =FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;
						
					default:
						DebugMessage("Stream::SetFrameType: Unknown FrameType %lu", (unsigned long int) mFrameType);
						throw CAException(kCMIOHardwareIllegalOperationError);
				}
				
				GetOwningDevice().GetIOVADevice().SetStreamFormat(CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_StreamID))), false).GetSInt32(), &theNewFormat);

				// Start the stream
				Start(MACH_PORT_NULL, MACH_PORT_NULL, kCMIOSampleBufferDiscontinuityFlag_DataFormatChanged);
				
				// Update the shadow time for the format description since it changed
				mProperties[PropertyAddress(kCMIOStreamPropertyFormatDescription, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
				
				// Update the shadow time for the frame rate if it is different
				if (mFrameRate != currentFrameRate)
					mProperties[PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			}
			catch (...)
			{
				// Something went wrong, so try and restore the previous frame type & frame rate control
				mFrameType = currentFrameType;
				mFrameRate = currentFrameRate;
                mNominalFrameDuration = mFrameRatesMap[mFrameType][mFrameRate];
				
				// DoWhatYouWouldNeedToDoToProgramTheHardware();

				// Start the stream
				Start(MACH_PORT_NULL, MACH_PORT_NULL, kCMIOSampleBufferDiscontinuityFlag_DataFormatChanged);
			}
		}
		else
		{
			// the stream isn't running just change the format and don't start the stream back up
			try
			{
				// DoWhatYouWouldNeedToDoToProgramTheHardware();
				// Send message to kext here to change the format
				IOVideoStreamDescription theNewFormat;
				switch(mFrameType)
				{
					case kYUV422_720x480:
						theNewFormat.mVideoCodecType = kYUV422_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
					
					case kYUV422_720x486:
						theNewFormat.mVideoCodecType = kYUV422_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_720x576:
						theNewFormat.mVideoCodecType = kYUV422_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
						
					case kYUV422_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_1920x1080; 
						theNewFormat.mVideoCodecFlags =FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;

                    case kYUV422_10_720x480:
						theNewFormat.mVideoCodecType = kYUV422_10_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
                        
					case kYUV422_10_720x486:
						theNewFormat.mVideoCodecType = kYUV422_10_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_10_720x576:
						theNewFormat.mVideoCodecType = kYUV422_10_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_10_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_10_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
						
					case kYUV422_10_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_10_1920x1080; 
						theNewFormat.mVideoCodecFlags =FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;
						
					default:
						DebugMessage("Stream::SetFrameType: Unknown FrameType %lu", (unsigned long int) mFrameType);
						throw CAException(kCMIOHardwareIllegalOperationError);
				}
				
				GetOwningDevice().GetIOVADevice().SetStreamFormat(CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_StreamID))), false).GetSInt32(), &theNewFormat);


				// Update the shadow time for the format description since it changed
				mProperties[PropertyAddress(kCMIOStreamPropertyFormatDescription, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
				
				// Update the shadow time for the frame rate if it is different
				if (mFrameRate != currentFrameRate)
					mProperties[PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			}
			catch (...)
			{
				// Something went wrong, so try and restore the previous frame type & frame rate control
				mFrameType = currentFrameType;
				mFrameRate = currentFrameRate;
                mNominalFrameDuration = mFrameRatesMap[mFrameType][mFrameRate];
				
				// DoWhatYouWouldNeedToDoToProgramTheHardware();

			}
		}
	}	

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFrameRates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	vm_size_t Stream::GetFrameRates(const FrameType* qualifier, Float64** frameRates) const
	{
		// If there is no qualifier, then this request is for the frame rates of the current FrameType
		FrameType frameType = (NULL == qualifier) ? GetFrameType() : *qualifier;
		
		// Find the frame rates associated with the FrameType
		FrameTypeToFrameRatesMap::const_iterator i = mFrameRatesMap.find(frameType);
		ThrowIf(i == mFrameRatesMap.end(), CAException(kCMIOHardwareUnknownPropertyError), "Stream::GetFrameRates: unable to map FrameType to FrameRates");

		// This answer is an array of Float64s
		vm_size_t size = sizeof(Float64) * (*i).second.size();
		ThrowIfKernelError(vm_allocate(mach_task_self(), reinterpret_cast<vm_address_t*>(frameRates), size, true), CAException(-1), "Stream::GetFrameRates: allocation failed for kCMIOStreamPropertyFrameRates");

		// Fill in the array
		int index = 0;
		for (FrameRates::const_iterator ii = (*i).second.begin() ; ii != (*i).second.end() ; ++ii)
			(*frameRates)[index++] = (*ii).first;
		
		return size;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::SetFrameRate(Float64 frameRate)
	{
		// No need to do anything if the frameRate hasn't changed
		if (frameRate == mFrameRate)
			return;

		// Grab the mutex for the overall device's state
		CAMutex::Locker locker(mStateMutex);
		
		// Program the device settings (which starts the stream) if currently streaming
		bool programSettings = mStreaming;
		
		// Stop the stream if needed
		if (mStreaming)
		{
			Stop(MACH_PORT_NULL);
		}
		
		// Save the current frame type & frame rate control in the event of an error so a restore attempt can be made
		Float64 currentFrameRate = mFrameRate;
		
        
        FrameRates::const_iterator i = mFrameRatesMap[mFrameType].find(frameRate);
        if (i == mFrameRatesMap[mFrameType].end())
        {
            //the desired frameRate isn't in the current mFrameType's list
            return;
        }
        
		// Set the frame rate to the fastest possible for this frame type
        //		mFrameRate = (*mFrameRatesMap[mFrameType].rbegin()).first;
		
		if (programSettings)
		{
			try
			{
				// DoWhatYouWouldNeedToDoToProgramTheHardware();
				// Send message to kext here to change the format
				IOVideoStreamDescription theNewFormat;
				switch(mFrameType)
				{
					case kYUV422_720x480:
						theNewFormat.mVideoCodecType = kYUV422_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(frameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
                        
					case kYUV422_720x486:
						theNewFormat.mVideoCodecType = kYUV422_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_720x576:
						theNewFormat.mVideoCodecType = kYUV422_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(frameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
                        
					case kYUV422_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_1920x1080; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(frameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;
 
                    case kYUV422_10_720x480:
						theNewFormat.mVideoCodecType = kYUV422_10_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
                        
					case kYUV422_10_720x486:
						theNewFormat.mVideoCodecType = kYUV422_10_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_10_720x576:
						theNewFormat.mVideoCodecType = kYUV422_10_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_10_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_10_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
						
					case kYUV422_10_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_10_1920x1080; 
						theNewFormat.mVideoCodecFlags =FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;

						
				}
				printf("SetFrameRate newFormat.mVideoCodecType = %lu newFormat.mVideoCodecFlags = %x\n", (long unsigned int)theNewFormat.mVideoCodecType, (unsigned int)theNewFormat.mVideoCodecFlags);
                
				GetOwningDevice().GetIOVADevice().SetStreamFormat(CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_StreamID))), false).GetSInt32(), &theNewFormat);
                
				Start(MACH_PORT_NULL, MACH_PORT_NULL, kCMIOSampleBufferDiscontinuityFlag_DataFormatChanged);

				mFrameRate = frameRate;

				
				// Update the shadow time for the frame rate
				mProperties[PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			}
			catch (...)
			{
				// Something went wrong, so try and restore the previous frame type & frame rate control
				mFrameRate = currentFrameRate;
				
				// DoWhatYouWouldNeedToDoToProgramTheHardware();
                
                // Start the stream
				Start(MACH_PORT_NULL, MACH_PORT_NULL, kCMIOSampleBufferDiscontinuityFlag_DataFormatChanged);
			}
		}
		else
		{
			// the stream isn't running just change the format and don't start the stream back up
			try
			{
				// DoWhatYouWouldNeedToDoToProgramTheHardware();
				// Send message to kext here to change the format
				IOVideoStreamDescription theNewFormat;
				switch(mFrameType)
				{
					case kYUV422_720x480:
						theNewFormat.mVideoCodecType = kYUV422_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(frameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
			
					case kYUV422_720x486:
						theNewFormat.mVideoCodecType = kYUV422_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
		
					case kYUV422_720x576:
						theNewFormat.mVideoCodecType = kYUV422_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(frameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
						
					case kYUV422_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_1920x1080; 
						theNewFormat.mVideoCodecFlags =FrameRateToCodecFlags(frameRate);
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;
                    case kYUV422_10_720x480:
						theNewFormat.mVideoCodecType = kYUV422_10_720x480; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 480;
						break;
                        
					case kYUV422_10_720x486:
						theNewFormat.mVideoCodecType = kYUV422_10_720x486; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 486;
						break;
                        
					case kYUV422_10_720x576:
						theNewFormat.mVideoCodecType = kYUV422_10_720x576; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 720;
						theNewFormat.mHeight = 576;
						break;
                        
					case kYUV422_10_1280x720:
						theNewFormat.mVideoCodecType = kYUV422_10_1280x720; 
						theNewFormat.mVideoCodecFlags = FrameRateToCodecFlags(mFrameRate);
						theNewFormat.mWidth = 1280;
						theNewFormat.mHeight = 720;
						break;
						
					case kYUV422_10_1920x1080:
						theNewFormat.mVideoCodecType = kYUV422_10_1920x1080; 
						theNewFormat.mVideoCodecFlags =FrameRateToCodecFlags(mFrameRate);  
						theNewFormat.mWidth = 1920;
						theNewFormat.mHeight = 1080;
						break;
               }
				
				GetOwningDevice().GetIOVADevice().SetStreamFormat(CACFNumber(static_cast<CFNumberRef>(CFDictionaryGetValue(mStreamDictionary.GetCFDictionary(), CFSTR(kIOVideoStreamKey_StreamID))), false).GetSInt32(), &theNewFormat);
                
                            
				// Update the shadow time for the frame rate if it is different
				mFrameRate = frameRate;
                
                mProperties[PropertyAddress(kCMIOStreamPropertyFrameRate, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
			}
			catch (...)
			{
				// Something went wrong, so try and restore the previous frame rate control
				mFrameRate = currentFrameRate;
				
				// DoWhatYouWouldNeedToDoToProgramTheHardware();
                
			}
		}
		
    }

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Start()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Start(Client client, mach_port_t messagePort, UInt32 initialDiscontinuityFlags)
	{
		try
		{
			// Grab the mutex for the device's state
			CAMutex::Locker locker(mStateMutex);
			
			// Add the  Client to the set of clients to which are listening to the stream
			if (MACH_PORT_NULL != client)
			{
				// Don't do anything if the client is already in the map of ClientStreams
				if (mClientStreams.end() != mClientStreams.find(client))
					return;
					
				// Create the ClientStream object to help manage this client's connection to the stream
				ClientStream* clientStream = new ClientStream(client, messagePort, mFrameAvailableGuard);
				
				// Add it to the map of clients using the stream
				CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);
				mClientStreams[client] = clientStream;
			}
			
			// If the stream is currently active nothing else needs to be done so simply return
			if (mStreaming)
				return;
			
			// Attempt to open the stream
			mIOSAStream.Open();
			
			try
			{
				if (IsInput())
				{
					// Start the stream
					mIOSAStream.Start();
				}
				else
				{
					// Make sure the client provided a valid messagePort from which to request buffers
					ThrowIf(MACH_PORT_NULL == messagePort and MACH_PORT_NULL != client, CAException(kCMIOHardwareIllegalOperationError), "Device::StartStream: output client provided an invalid message port");

					if (MACH_PORT_NULL != messagePort)
					{
						// Remember where to request frames from
						mOutputBufferRequestPort = messagePort;

						// Remember which client started the output stream 
						mOutputClient = client;
					}

					// Release any lingering output sample buffer
					mOutputBuffer.Reset();
					mFreeList.clear();

					// Intialize event times for providing device clock information
					mEvents[0].mEventTime		= kCMTimeInvalid;
					mEvents[0].mHostTimeInNanos	= 0LL;
					mPreviousEvent				= mEvents[0];
					mPreviousNotedEvent			= mEvents[0];
					mCurrentEventIndex			= 0;

					// Start the stream
					mIOSAStream.Start();
				}
			}
			catch (...)
			{
				// Something went wrong, so close the stream 
				mIOSAStream.Close();
				throw;
			}

			SetDiscontinuityFlags(initialDiscontinuityFlags);			// Set the initial discontinuity flags

			// The stream is active now
			mStreaming = true;
		}
		catch (...)
		{
			// Something went wrong, so clean up
			ClientStreamMap::iterator i = mClientStreams.find(client);
			if (i != mClientStreams.end())
			{
				// The ClientStream had been succesfully created, so delete it and erase it from the map
				CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);
				delete (*i).second;
				mClientStreams.erase(i);
			}
			else if (MACH_PORT_NULL != messagePort)
			{
				// The ClientStream had not been created, so deallocate the message port since no ~ClientStreams() was invoked (which normally handles the deallocation)
				(void) mach_port_deallocate(mach_task_self(), messagePort);

				// Reset mOutputBufferRequestPort & mOutputClient to MACH_PORT_NULL
				mOutputBufferRequestPort = MACH_PORT_NULL;
				mOutputClient = MACH_PORT_NULL;
			}
			throw;
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::Stop(Client client)
	{
		// Grab the mutex for the overall device's state
		CAMutex::Locker locker(mStateMutex);

		// Delete and erase the client's from the ClientStream map
		ClientStreamMap::iterator i = mClientStreams.find(client);
		if (i != mClientStreams.end())
		{
			CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);
			delete (*i).second;
			mClientStreams.erase(i);
		}

		// Don't stop the stream if it isn't streaming
		if (not mStreaming)
			return;

		// Don't stop the stream if it is an input stream but there are still clients watching it
		if (IsInput() and (not mClientStreams.empty() and MACH_PORT_NULL != client))
			return;

		// Throw an exception if this is an output stream but the stop request is coming from a client that did not make the initial start request
		ThrowIf(IsOutput() and (mOutputClient != client and MACH_PORT_NULL != client), CAException(kCMIODevicePermissionsError), "Device::StopStream: stream was started by a different client"); 

		// The stream is no longer active
		mStreaming = false;

		// Make sure StreamOutputCallback() is not being invoked on another thread
		while (mInOutputCallBack)
			usleep(1000);
			
		if (IsInput())
		{
			// Stop the IOStream
			mIOSAStream.Stop();
		
			// Notify all the client stream message threads that a frame might be available
			mFrameAvailableGuard.NotifyAll();		

			// If there are still clients streaming, wait for each client queue of any existing frames be drained
			for (ClientStreamMap::iterator i = mClientStreams.begin() ; i != mClientStreams.end() ; std::advance(i, 1))
			{
				// Yield if a client still has frames in its queue to send
				while (0 != (*i).second->GetQueue().GetCount())
					pthread_yield_np();
			}
		}
		else
		{
			// Stop the IOStream
			mIOSAStream.Stop();
		
			// Release any lingering output buffer
			mOutputBuffer.Reset();
			mFreeList.clear();
			
			if (client != MACH_PORT_NULL)
			{
				// Release the Mach port from which frames were requested 
				(void) mach_port_deallocate(mach_task_self(), mOutputBufferRequestPort);
				mOutputBufferRequestPort = MACH_PORT_NULL;
				
				// Indicate no client is using the output stream
				mOutputClient = MACH_PORT_NULL;
			}
		}
	
		// Close the IOSteam
		mIOSAStream.Close();
	}

 	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StreamOutputCallback()
	//	Called when a new buffer is available from the kernel
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    void Stream::StreamOutputCallback(IOStreamRef /*streamRef*/, Stream& stream)
    {
		// Indicate that the output callback is being invoked so Stop() won't release resources  
		stream.mInOutputCallBack = true;
		
		if (not stream.mStreaming)
		{
			// The stream has been stopped already, so exit from callback
			stream.mInOutputCallBack = false;
			return;
		}
			
		if (stream.IsInput())
		{
            IOStreamBufferQueueEntry entry;
			while (kIOReturnSuccess == (**stream.mIOSAStream).DequeueOutputEntry(stream.mIOSAStream, &entry))
			{
				// Don't let any exceptions leave this callback
				try
				{
					stream.FrameArrived(entry);
				}
				catch (...)
				{
					// Something went wrong, so return the entry to the queue
					(**stream.mIOSAStream).EnqueueInputBuffer(stream.mIOSAStream, entry.bufferID, 0, 0, 0, 0);
					(**stream.mIOSAStream).SendInputNotification(stream.mIOSAStream, 0xAA);
				}
			}
		}
		else
		{
			// Process the entries on the output buffer list and move them to the free list
			IOStreamBufferQueueEntry entry;
			while (kIOReturnSuccess == (**stream.mIOSAStream).DequeueOutputEntry(stream.mIOSAStream, &entry))
			{
				SampleVideoDeviceControlBuffer theBufferControl;
				bcopy(stream.mIOSAStream.GetControlBuffer(entry.bufferID), &theBufferControl, sizeof(SampleVideoDeviceControlBuffer));
				DebugMessage("Buffer Control info vbitime = %lld dmatime = %lld framecount=%lld dropcount = %d lastSequenceNumber = %lld discontinuity flags = %ld", theBufferControl.vbiTime, theBufferControl.outputTime, theBufferControl.totalFrameCount, theBufferControl.droppedFrameCount, theBufferControl.sequenceNumber, theBufferControl.discontinuityFlags);
				if ((theBufferControl.discontinuityFlags & kCMIOSampleBufferDiscontinuityFlag_DataWasFlushed))
				{
					DebugMessage("A flushed buffer skip timing");
					stream.mLastOutputSequenceNumber = theBufferControl.sequenceNumber; 
				}
				else
				{
					stream.mLastOutputSequenceNumber = theBufferControl.sequenceNumber; 
					// Determine clock time based on frame count
					if (CMTIME_IS_INVALID(stream.mEvents[0].mEventTime))
					{
						DebugMessage("DPA::Sample::Server::Stream::StreamOutputCallback invalid mEventTime");
						// RFP FixMe and support for other sample rates
						stream.mEvents[1].mEventTime = stream.mNominalFrameDuration;
						stream.mEvents[1].mEventFrameCount = theBufferControl.totalFrameCount; 
						stream.mEvents[1].mDroppedFrameCount = theBufferControl.droppedFrameCount; 
						
						stream.mEvents[0].mEventTime = CMTimeMake(stream.mEvents[1].mEventTime.timescale, stream.mEvents[1].mEventTime.timescale);
						stream.mEvents[0].mEventFrameCount = stream.mEvents[1].mEventFrameCount;
						stream.mEvents[0].mDroppedFrameCount = stream.mEvents[1].mDroppedFrameCount;
					}
					else
					{
						// Bump to next frame
						stream.mEvents[0].mEventFrameCount = theBufferControl.totalFrameCount;
						stream.mEvents[0].mDroppedFrameCount = theBufferControl.droppedFrameCount;							
						stream.mEvents[0].mEventTime = CMTimeAdd(stream.mEvents[0].mEventTime, CMTimeMake(stream.mNominalFrameDuration.value*(stream.mEvents[0].mEventFrameCount- stream.mEvents[1].mEventFrameCount), stream.mNominalFrameDuration.timescale));
						if (stream.mEvents[0].mDroppedFrameCount != stream.mEvents[1].mDroppedFrameCount)
						{
							++stream.mUnderrunCount;
							
							// The kTundraStreamPropertyOutputBufferUnderrunCount property has changed, so send out notifications
							stream.mProperties[PropertyAddress(kCMIOStreamPropertyOutputBufferUnderrunCount, stream.GetDevicePropertyScope(), stream.GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
							(stream.GetOwningDevice()).SendPropertyStatesChangedMessage();
							DebugMessage("DPA::Sample::Server::Stream::StreamOutputCallback:  Dropped a frame (%ld)", stream.mUnderrunCount);
						}
						stream.mEvents[1].mEventFrameCount = theBufferControl.totalFrameCount; 
						stream.mEvents[1].mDroppedFrameCount = theBufferControl.droppedFrameCount; 
					}
					
					// Save hosttime
					stream.mEvents[0].mHostTimeInNanos = CAHostTimeBase::ConvertToNanos(theBufferControl.vbiTime);
                }
                        
				stream.mFreeList.push_back(entry);
			}
                        
			// Get a mOutputBuffer
            UInt32 buffersServiced = 0;
            while (buffersServiced < kMaxRequestsPerCallback)
            {
                if (not stream.mOutputBuffer.IsValid())
                    stream.GetOutputBuffer(stream.mOutputBufferRequestPort);
                        
                if (not stream.mOutputBuffer.IsValid() or not stream.mStreaming)
                    break;
                
				if (not stream.mFreeList.empty())
				{
					entry = stream.mFreeList.front();
					stream.mFreeList.pop_front();
                    
                    //copy the data from the mOutputBuffer to the entry and send back
                    if (stream.mOutputBuffer.GetDataLength() <= entry.dataLength)
                    {
                        SampleVideoDeviceControlBuffer theBufferControl;
                        bcopy(stream.mIOSAStream.GetControlBuffer(entry.bufferID), &theBufferControl, sizeof(SampleVideoDeviceControlBuffer));
									
						bcopy(stream.mOutputBuffer.GetDataPointer(0, NULL, NULL), stream.mIOSAStream.GetDataBuffer(entry.bufferID), stream.mOutputBuffer.GetDataLength());
                        
                        theBufferControl.sequenceNumber = stream.mCurrentOutputSequenceNumber;
                        theBufferControl.discontinuityFlags = stream.mCurrentDiscontinuityFlags;
                        theBufferControl.smpteTime = stream.mCurrentSMPTETime;

//                        DebugMessage("theBufferControl.mCounter = %ld",theBufferControl.smpteTime.mCounter); 
//                        DebugMessage("theBufferControl.mHours = %ld",theBufferControl.smpteTime.mHours); 
//                        DebugMessage("theBufferControl.mMinutes = %ld",theBufferControl.smpteTime.mMinutes); 
//                        DebugMessage("theBufferControl.mSeconds = %ld",theBufferControl.smpteTime.mSeconds); 
//                        DebugMessage("theBufferControl.mFrames = %ld",theBufferControl.smpteTime.mFrames); 

                        
                        bcopy(&theBufferControl, stream.mIOSAStream.GetControlBuffer(entry.bufferID), sizeof(SampleVideoDeviceControlBuffer));
					}
                    else
                    {
                        DebugMessage("MIO::DPA::Sample::Server::Stream::StreamOutputCallback:  Mismatched Data length buffer = %ld entry = %ld", stream.mOutputBuffer.GetDataLength(), entry.dataLength);
                    }
					(**stream.mIOSAStream).EnqueueInputBuffer(stream.mIOSAStream, entry.bufferID, 0, 0, 0, 0);
					(**stream.mIOSAStream).SendInputNotification(stream.mIOSAStream, 0xAA);
                    
                    stream.mOutputBuffer.Reset();
                }
                else
                {
                    DebugMessage("MIO::DPA::Sample::Server::Stream::StreamOutputCallback: NO BUFFERS IN STREAM");
                    break;
				}
				
                ++buffersServiced;
			}
		}
 
		// Indicate that the output callback is not being invoked
		stream.mInOutputCallBack = false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FrameArrived()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::FrameArrived(IOStreamBufferQueueEntry& entry)
	{
		// Get the host time of the the frame (currently reported as the sole contents of the entry's control buffer)
		SampleVideoDeviceControlBuffer* theBufferControl = reinterpret_cast<SampleVideoDeviceControlBuffer*>(mIOSAStream.GetControlBuffer(entry.bufferID));
		ThrowIfNULL(theBufferControl, CAException(kCMIOHardwareUnspecifiedError), "Stream::FrameArrived: unable to get control buffer");
		
		// Create the presentation time stamp
		CMTime presentationTimeStamp = CMTimeMakeWithSeconds(CAHostTimeBase::ConvertToNanos(theBufferControl->vbiTime) / 1000000000.0, GetNominalFrameDuration().timescale);
		
		// Create the timing information
		CMA::SampleBuffer::TimingInfo timingInfo(GetNominalFrameDuration(), presentationTimeStamp, kCMTimeInvalid);
		
		// Wrap the entry in a Frame
		Frame* frame = new Frame(mIOSAStream, GetFrameType(), theBufferControl->vbiTime, timingInfo, GetDiscontinuityFlags(), theBufferControl->droppedFrameCount, theBufferControl->firstVBITime, entry.bufferID, entry.dataLength, mIOSAStream.GetDataBuffer(entry.bufferID));

		// Clear the discontinuity flags since any accumulated discontinuties have passed onward with the frame
		SetDiscontinuityFlags(kCMIOSampleBufferNoDiscontinuities);
	
		// Create a queue of client queues into which the frame should be inserted
		std::vector<CMA::SimpleQueue<Frame*>*> queues;
		
		// Grab the ClientStreams mutex so they won't be altered in the midst of this routine
		CAMutex::Locker clientStreamsLocker(mClientStreamsMutex);

		// Insert the frame into each client's queue
		for (ClientStreamMap::iterator i = mClientStreams.begin() ; i != mClientStreams.end() ; std::advance(i, 1))
		{
			if (1.0 != (*i).second->GetQueue().Fullness())
			{
				// Add the client to the set of clients to which this frame will be messaged
				frame->AddClient((*i).first);
				queues.push_back(&(*i).second->GetQueue());
			}
			else
			{
				// This condition should never be hit, but if that assumption proves false then some code changes would be necessary to properly record the dropped frame.
				DebugMessage("Stream::FrameArrived: client (%d) queue full <---------- NEED TO EXTEND", (*i).first);
			}
		}

		// Enqueue all the frame for all the clients that had room
		for (std::vector<CMA::SimpleQueue<Frame*>*>::iterator i = queues.begin() ; i != queues.end() ; std::advance(i, 1))
			(**i).Enqueue(frame);

		// Make sure at least one client wanted the message
		if (queues.empty())
		{
			// There were no clients to message this too, so invoke RemoveClient(MACH_PORT_NULL) on the frame to make it available for refilling 
			frame->RemoveClient(MACH_PORT_NULL);
		}
		
		if (true)
		{
			mDeck.FrameArrived();
		}
		
		// Notify all the client stream message threads that a frame is available
		mFrameAvailableGuard.NotifyAll();		
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetOutputBuffer()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::GetOutputBuffer(mach_port_t& recipient)
	{
		// Get timing information so the client can drive their output clock
		CMTime clockTime = kCMTimeInvalid;
		UInt64 hostTimeInNanos = 0LL;
		
		clockTime = mEvents[0].mEventTime;
		hostTimeInNanos = mEvents[0].mHostTimeInNanos;
		
		if ((CMTimeCompare(mPreviousNotedEvent.mEventTime, clockTime) != 0) or (mPreviousNotedEvent.mHostTimeInNanos != hostTimeInNanos))
		{
			mPreviousNotedEvent.mEventTime = clockTime;
			mPreviousNotedEvent.mHostTimeInNanos =	hostTimeInNanos;
			DebugMessage("GetOutputBuffer(): clockTime = %lld : %d, hostTimeInNanos = %lld", clockTime.value, clockTime.timescale, hostTimeInNanos);
		}
		else
		{
			DebugMessage("GetOutputBuffer(): no prev noted event");
			clockTime = kCMTimeInvalid;
			hostTimeInNanos = 0;
		}

		// Release the current output buffer
		mOutputBuffer.Reset();
		
		// Use the union structure to handle the message because we send one type but receive another
		OutputBufferMessages message;
		
		// Setup the message
		message.asOutputBufferRequestedMessage.mHeader.msgh_bits		= MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);
		//		message.asOutputBufferRequestedMessage.mHeader.msgh_bits		= MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE) | MACH_MSGH_BITS_COMPLEX;
		message.asOutputBufferRequestedMessage.mHeader.msgh_size		= sizeof(OutputBufferRequestedMessage);
		message.asOutputBufferRequestedMessage.mHeader.msgh_remote_port	= recipient;
		message.asOutputBufferRequestedMessage.mHeader.msgh_local_port	= mig_get_reply_port();
		message.asOutputBufferRequestedMessage.mHeader.msgh_reserved	= 0;
		message.asOutputBufferRequestedMessage.mHeader.msgh_id			= kOutputBufferRequested;
		
		message.asOutputBufferRequestedMessage.mClockTime				= CMTimeOverride(clockTime);
		message.asOutputBufferRequestedMessage.mHostTimeInNanos			= hostTimeInNanos;
		message.asOutputBufferRequestedMessage.mLastSequenceNumber      = mLastOutputSequenceNumber;
        
//        DebugMessage("mHostTimeInNanos = %lld message.asOutputBufferRequestedMessage.mSequenceNumber = %lld", message.asOutputBufferRequestedMessage.mHostTimeInNanos,message.asOutputBufferRequestedMessage.mLastSequenceNumber);
       
		mach_msg_size_t replySizeLimit = sizeof(OutputBufferMessages) + sizeof(mach_msg_trailer_t);
		mach_msg_return_t err = mach_msg(&(message.asOutputBufferRequestedMessage.mHeader), MACH_SEND_MSG | MACH_RCV_MSG, message.asOutputBufferRequestedMessage.mHeader.msgh_size, replySizeLimit, message.asOutputBufferRequestedMessage.mHeader.msgh_local_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
		if (MACH_MSG_SUCCESS != err)
		{
			// Something went wrong, so dispose of the reply port properly
			DebugMessage("Device::GetOutputBuffer: mach_msg() failed - error: 0x%X (%s)", err, mach_error_string(err));
			switch (err)
			{
				case MACH_SEND_INVALID_REPLY:
				case MACH_RCV_INVALID_NAME:
				case MACH_RCV_PORT_DIED:
				case MACH_RCV_PORT_CHANGED:
					mig_dealloc_reply_port(message.asOutputBufferRequestedMessage.mHeader.msgh_local_port);
					break;
					
				default:
					mig_put_reply_port(message.asOutputBufferRequestedMessage.mHeader.msgh_local_port);
			}
			
			return;
		}
		
		// If the reply carried no payload, simply return
		if (kNoOutputBufferSupplied == message.asNoOutputBufferSuppliedMessage.mHeader.msgh_id)
			return;

        if (kOutputSurfaceSupplied == message.asOutputSurfaceSuppliedMessage.mHeader.msgh_id)
        {
            DebugMessage("GOT SURFACE SUPPLIED");

            mCurrentOutputSequenceNumber = message.asOutputSurfaceSuppliedMessage.mSequenceNumber;
            mCurrentDiscontinuityFlags = message.asOutputSurfaceSuppliedMessage.mDiscontinuityFlags;
            mCurrentSMPTETime = message.asOutputSurfaceSuppliedMessage.mSMPTETime;
            
            // Reconstitute the pixel buffer from the IOSurface's mach port
            CVA::Pixel::Buffer pixelBuffer(CVA::Pixel::Buffer::CreateFromIOSurface(NULL, SA::Surface(IOSurfaceLookupFromMachPort(message.asOutputSurfaceSuppliedMessage.mDescriptor.name)), NULL));
            
            // Deallocate the Mach port was used to reconstitute the IOSurface
            (void) mach_port_deallocate(mach_task_self(), message.asOutputSurfaceSuppliedMessage.mDescriptor.name);
            
            // Lock the base address of the pixel buffer
            pixelBuffer.LockBaseAddress(kCVPixelBufferLock_ReadOnly);
            
            try
            {
                // Create the Block Buffer to wrap the CV Pixel Buffer.
                CMBlockBufferCustomBlockSource customBlockSource = { kCMBlockBufferCustomBlockSourceVersion, NULL, ReleasePixelBufferCallback, pixelBuffer };
                
                size_t frameSize = CVPixelBufferGetBytesPerRow(pixelBuffer) * CVPixelBufferGetHeight(pixelBuffer);
                DebugMessage("DP::AJA::Server::Stream::GetOutputBuffer height = %lld, bpr =  %lld frameSize = %lld",CVPixelBufferGetHeight(pixelBuffer),CVPixelBufferGetBytesPerRow(pixelBuffer),frameSize);
                ThrowIf(frameSize == 0, CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::VDC::Stream::FrameArrived: frameSize was 0!");
                
                // Wrap the native frame in a block buffer.  kCFAllocatorNull will be used for the block allocator, so no memory will be deallocated when the block buffer goes out of scope.
                mOutputBuffer.Reset(CMA::BlockBuffer::CreateWithMemoryBlock(NULL, CVPixelBufferGetBaseAddress(pixelBuffer), frameSize, NULL, &customBlockSource, 0, frameSize, 0));
                
                
                // Bump the pixel buffer's retention count since it is now 'owned' by the block buffer and will be released in ReleasePixelBufferCallback()
                CVPixelBufferRetain(pixelBuffer);
            }
            catch (...)
            {
                // Something went wrong, so unlock the pixel buffer and rethrow the exception
                pixelBuffer.UnlockBaseAddress(kCVPixelBufferLock_ReadOnly);
                throw;
            }
            
            // Attach the pixel buffer reference to the block buffer
            CMSetAttachment(mOutputBuffer, kCMIOBlockBufferAttachmentKey_CVPixelBufferReference, pixelBuffer, kCMAttachmentMode_ShouldNotPropagate);
            
        }
        else
        {
            DebugMessage("GOT BLOCK BUFFER SUPPLIED");
      
            mCurrentOutputSequenceNumber = message.asOutputBufferSuppliedMessage.mSequenceNumber;
            mCurrentDiscontinuityFlags = message.asOutputBufferSuppliedMessage.mDiscontinuityFlags;
            mCurrentSMPTETime = message.asOutputBufferSuppliedMessage.mSMPTETime;

            // Wrap the buffer in a block buffer to handle invoking vm_deallocate() on the data when it is no longer needed
            CMBlockBufferCustomBlockSource customBlockSource = { kCMBlockBufferCustomBlockSourceVersion, NULL, ReleaseOutputBufferCallBack, this };
            mOutputBuffer.Reset(CMA::BlockBuffer::CreateWithMemoryBlock(NULL, message.asOutputBufferSuppliedMessage.mDescriptor.address, message.asOutputBufferSuppliedMessage.mDescriptor.size, kCFAllocatorNull, &customBlockSource, 0, message.asOutputBufferSuppliedMessage.mDescriptor.size, 0));
        }
	}	

	#pragma mark -	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::StartDeckThreads(Client client)
	{
        DebugMessage("Stream::StartDeckThreads");
		// Add the client to the set of clients which have have requested the deck threads be started
		if (MACH_PORT_NULL != client)
		{
			// Don't do anything if this client is already in the already marked as listening to the deck
			if (mDeckListeners.end() != mDeckListeners.find(client))
				return;
			
			// Add the client to the set of deck listerers
			mDeckListeners.insert(client);
		}
		
		// Start the threads
		mDeck.StartThreads();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::StopDeckThreads(Client client)
	{
		// Remove the client from the set of listing to the deck
		mDeckListeners.erase(client);
		
		// Don't stop the threads if there are are still listeners
		if (not mDeckListeners.empty() and MACH_PORT_NULL != client)
			return;
		
		// Stop the threads
		mDeck.StopThreads();
	}
	
	#pragma mark -
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ReleaseOutputBufferCallBack()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::ReleaseOutputBufferCallBack(void* refCon, void *doomedMemoryBlock, size_t sizeInBytes)
	{
		(void) vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(doomedMemoryBlock), sizeInBytes);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ReleasePixelBufferCallback()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::ReleasePixelBufferCallback(void* refCon, void *doomedMemoryBlock, size_t sizeInBytes)
	{
        DebugMessage("ReleasePixelBufferCallback");
		// The frame was being provided in an IOSurface-backed CVPixelBuffer so release it
		CVA::Pixel::Buffer pixelBuffer(static_cast<CVPixelBufferRef>(refCon));
		pixelBuffer.UnlockBaseAddress(kCVPixelBufferLock_ReadOnly);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StreamDeckChanged()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::StreamDeckChanged()
	{
		mProperties[PropertyAddress(kCMIOStreamPropertyDeck, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
		GetOwningDevice().SendPropertyStatesChangedMessage();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckTimecodeChanged()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DeckTimecodeChanged()
	{
		mProperties[PropertyAddress(kCMIOStreamPropertyDeckFrameNumber, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
		GetOwningDevice().SendPropertyStatesChangedMessage();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckCueingChanged()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Stream::DeckCueingChanged()
	{
		mProperties[PropertyAddress(kCMIOStreamPropertyDeckCueing, GetDevicePropertyScope(), GetStartingDeviceChannelNumber())].mShadowTime = CAHostTimeBase::GetTheCurrentTime();
		GetOwningDevice().SendPropertyStatesChangedMessage();
	}
}}}}
