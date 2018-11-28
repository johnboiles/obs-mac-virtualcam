/*
	    File: CMIO_IOVA_Assistance.h
	Abstract: The IOVideo Assisistant (IOVA) consists of various objects to facilitate using the "Acquisition is Initialization" design pattern.
				NOTE: much of the IOVA is sparsely implemented, meaning that it does not attempt to provide wrappers/access  to all IOVideo features, but just those needed by the IOVideo
				plugin.
	
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

#if !defined(__CMIO_IOVA_Assistance_h__)
#define __CMIO_IOVA_Assistance_h__

// CA Public Utility Includes
#include "CMIODebugMacros.h"
#include "CAException.h"

// System Includes
#include <IOKit/video/IOVideoDeviceLib.h>

namespace CMIO { namespace IOVA
{
    IOCFPlugInInterface**	AllocatePlugIn(io_service_t service);
    IOVideoDeviceRef		AllocateDevice(IOCFPlugInInterface** plugIn);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // PlugIn
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    class PlugIn
    {
	// Construction/Destruction
	public:
		explicit				PlugIn(IOCFPlugInInterface** plugIn) : mPlugin(0) { Reset(plugIn); }
								~PlugIn() { Reset(0); }
		void					Reset(IOCFPlugInInterface** plugIn = 0) { if (0 != mPlugin) IODestroyPlugInInterface(mPlugin); mPlugin = plugIn; }
        
	private:
		PlugIn&					operator=(PlugIn& that); // Don't allow copying
		IOCFPlugInInterface**	mPlugin;
		
	// Value Access
	public:
		operator				IOCFPlugInInterface**()	const { return mPlugin; }
		IOCFPlugInInterface**	Get() const { return mPlugin; }													
    };
    
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Device
	{
	// Construction/Destruction
	public:
        
								Device(IOVideoDeviceRef device) : mDevice(NULL), mOpened(false) { Reset(device); }
								~Device() { Reset(); }
		void					Reset(IOVideoDeviceRef device = NULL)
								{
									if (NULL != mDevice)
									{
										Close();
										(**mDevice).Release(mDevice);
										mDevice = NULL;
									} 
									
									if (NULL != device)
									{
										mDevice = device;
									}
								}

	private:
		Device&				operator=(Device& that); // Don't allow copying

		IOVideoDeviceRef	mDevice;
		bool				mOpened;
		
	// Operations
	public:
		void				Open() { IOReturn err = (**mDevice).Open(mDevice, 0); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::Open() failed"); mOpened = true; }
		void				Close() { if (not mOpened) return; IOReturn err = (**mDevice).Close(mDevice); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::Close() failed"); mOpened = false; }
        void				AddToRunLoop(CFRunLoopRef runLoop) { IOReturn err = (**mDevice).AddToRunLoop(mDevice, runLoop); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::AddToRunLoop() failed"); }
        void				RemoveFromRunLoop(CFRunLoopRef runLoop) { IOReturn err = (**mDevice).RemoveFromRunLoop(mDevice, runLoop); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::RemoveFromRunLoop() failed"); }
        void				SetNotificationCallback(IOVideoDeviceNotificationCallback deviceNotification, void* context) { IOReturn err = (**mDevice).SetNotificationCallback(mDevice, deviceNotification, context); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::SetNotificationCallback() failed"); }
		void				SetControl(UInt32 controlID, UInt32 value, UInt32* newValue) { IOReturn err = (**mDevice).SetControlValue(mDevice, controlID, value, newValue); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::SetControl() failed"); }
		void				SetStreamFormat(UInt32 streamID, IOVideoStreamDescription *newStreamFormat) { IOReturn err = (**mDevice).SetStreamFormat(mDevice, streamID, newStreamFormat); ThrowIfKernelError(err, CAException(err), "CMIO::IOVA::Device::SetFormat() failed"); }
	// Value Access
	public:
		operator			IOVideoDeviceRef() const { return mDevice; }
		IOVideoDeviceRef	Get() const { return mDevice; }													
		bool				Opened() const { return mOpened; }													
	};
}}

#endif
