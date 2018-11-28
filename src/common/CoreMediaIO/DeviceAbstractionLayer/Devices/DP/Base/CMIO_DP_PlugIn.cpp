/*
	    File: CMIO_DP_PlugIn.cpp
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
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DP_PlugIn.h"

// Internal Includes
#include "CMIO_DP_Device.h"

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// PlugIn()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	PlugIn::PlugIn(CFUUIDRef factoryUUID) :
		Object(kCMIOObjectUnknown, kCMIOPlugInClassID, *this),
		mStateMutex("CMIO::DP::Plugin state mutex"),
		mDeviceMap(),
		mInterface(&sInterface),
		mFactoryUUID((CFUUIDRef)CFRetain(factoryUUID)),
		mRefCount(0)
	{
		CFPlugInAddInstanceForFactory(factoryUUID);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~PlugIn()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	PlugIn::~PlugIn()
	{
		CFPlugInRemoveInstanceForFactory(mFactoryUUID.GetCFObject());
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initialize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::Initialize()
	{
		InitializeWithObjectID(kCMIOObjectUnknown);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// InitializeWithObjectID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::InitializeWithObjectID(CMIOObjectID objectID)
	{
		Object::Initialize();
		SetObjectID(objectID);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// InitializeWithObjectID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::Teardown()
	{
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Retain()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 PlugIn::Retain()
	{
		++mRefCount;
		return mRefCount;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Release()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 PlugIn::Release()
	{
		UInt32 answer = --mRefCount;
		
		if (answer == 0)
			delete this;
		
		return answer;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberDevices()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::Show() const
	{
		// Make a string for the class name
		const char* className = NULL;
		
		switch (mClassID)
		{
			case kCMIOPlugInClassID:
			default:
				className = "CMIO PlugIn";
				break;
		}
		
		// Get the object's name
		CFStringRef cfname = NULL;
		UInt32 dataUsed = 0;
		try
		{
			GetPropertyData(PropertyAddress(kCMIOObjectPropertyName), 0, NULL, sizeof(CFStringRef), dataUsed, &cfname);
		}
		catch(...)
		{
			cfname = NULL;
		}
		
		// Make a C string out of the name
		char name[256];
		name[0] = 0;
		if (cfname != NULL)
		{
			CFIndex length = 0;
			CFRange range = { 0, CFStringGetLength(cfname) };
			CFStringGetBytes(cfname, range, kCFStringEncodingUTF8, 0, false, (UInt8*)name, 255, &length);
			name[length] = 0;
			CFRelease(cfname);
		}
		
		// Print the information to the standard output
		printf("CMIOObjectID:\t\t\t0x%lX\n\tClass:\t\t\t\t%s\n\tName:\t\t\t\t%s\n", (long unsigned int)mObjectID, className, name);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberDevices()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 PlugIn::GetNumberDevices() const
	{
		return mDeviceMap.size();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Object* PlugIn::GetDeviceByIndex(UInt32 index) const
	{
		Object* answer = NULL;
		DeviceMap::const_iterator iterator = mDeviceMap.begin();
		std::advance(iterator, index);
		if (iterator != mDeviceMap.end())
			answer = iterator->second;

		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceByObjectID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Object* PlugIn::GetDeviceByObjectID(CMIOObjectID id) const
	{
		Object* answer = NULL;
		DeviceMap::const_iterator iterator = mDeviceMap.find(id);
		if (iterator != mDeviceMap.end())
			answer = iterator->second;

		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AddDevice()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::AddDevice(Object& device)
	{
		CMIOObjectID id = device.GetObjectID();
		DeviceMap::iterator iterator = mDeviceMap.find(id);
		if (iterator == mDeviceMap.end())
		{
			mDeviceMap.insert(DeviceMap::value_type(id, &device));
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// RemoveDevice()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PlugIn::RemoveDevice(Object& device)
	{
		CMIOObjectID id = device.GetObjectID();
		DeviceMap::iterator iterator = mDeviceMap.find(id);
		if (iterator != mDeviceMap.end())
		{
			mDeviceMap.erase(iterator);
		}
	}

}}
