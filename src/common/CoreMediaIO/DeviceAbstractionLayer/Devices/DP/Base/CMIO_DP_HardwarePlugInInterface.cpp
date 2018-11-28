/*
	    File: CMIO_DP_HardwarePlugInInterface.cpp
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

// Internal Includes
#include "CMIO_DP_Device.h"
#include "CMIO_DP_Object.h"
#include "CMIO_DP_PlugIn.h"
#include "CMIO_DP_Stream.h"

// CA Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CACFObject.h"
#include "CAException.h"
#include "CAMutex.h"

// System Includes
#include <CoreMediaIO/CMIOHardwarePlugIn.h>


namespace
{
	using namespace CMIO;
	using namespace CMIO::DP;
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//	HardwarePlugInInterface
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	#pragma mark	Plug-In Operations

	ULONG HardwarePlugIn_AddRef(CMIOHardwarePlugInRef self)
	{
		ULONG answer = 0;
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_AddRef: no plug-in");

			// Get the object out of the interface reference
			PlugIn* plugIn = PlugIn::GetObject(self);
			
			// Retain it
			answer = plugIn->Retain();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return answer;
	}

	ULONG HardwarePlugIn_Release(CMIOHardwarePlugInRef self)
	{
		ULONG answer = 0;
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_Release: no plug-in");

			// Get the object out of the interface reference
			PlugIn* plugIn = PlugIn::GetObject(self);
			
			// Release it
			answer = plugIn->Release();
			
			// Note that plugIn is invalid now, so don't use it!
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return answer;
	}

	HRESULT HardwarePlugIn_QueryInterface(CMIOHardwarePlugInRef self, REFIID uuid, LPVOID* interface)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_QueryInterface: no plug-in");
			ThrowIfNULL(interface, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_QueryInterface: no place to store the return value");

			// Set the returned interface to NULL
			*interface = NULL;
			
			// Get the object out of the interface reference
			PlugIn* plugIn = PlugIn::GetObject(self);
			
			// Create a CoreFoundation UUIDRef for the requested interface.
			CACFUUID interfaceUUID(CFUUIDCreateFromUUIDBytes(NULL, uuid));

			// Test the requested ID against the valid interfaces.
//			if (interfaceUUID.IsEqual(kCMIOHardwarePlugInInterface3ID) || interfaceUUID.IsEqual(kCMIOHardwarePlugInInterface2ID) || interfaceUUID.IsEqual(kCMIOHardwarePlugInInterfaceID) || interfaceUUID.IsEqual(IUnknownUUID))
			if (interfaceUUID.IsEqual(kCMIOHardwarePlugInInterfaceID) || interfaceUUID.IsEqual(IUnknownUUID))
			{
				//	 it's one of the interfaces we understand
				
				// Retain the object on behalf of the caller
				plugIn->Retain();
				
				// Return the interface;
				*interface = plugIn->GetInterface();
			}
			else
			{
				// Not anything we understand
				error = E_NOINTERFACE;
			}
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_Initialize(CMIOHardwarePlugInRef self)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_Initialize: no plug-in");

			// Get the object out of the interface reference
			PlugIn* plugIn = PlugIn::GetObject(self);
			
			// Do the work
			plugIn->Initialize();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_InitializeWithObjectID(CMIOHardwarePlugInRef self, CMIOObjectID objectID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_Initialize: no plug-in");

			// Get the object out of the interface reference
			PlugIn* plugIn = PlugIn::GetObject(self);
			
			// Do the work
			plugIn->InitializeWithObjectID(objectID);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_Teardown(CMIOHardwarePlugInRef self)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_Teardown: no plug-in");

			// Get the object out of the interface reference
			PlugIn* plugIn = PlugIn::GetObject(self);
			
			// Do the work
			plugIn->Teardown();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	#pragma mark	CMIOObject Operations

	void HardwarePlugIn_ObjectShow(CMIOHardwarePlugInRef self, CMIOObjectID objectID)
	{
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectShow: no plug-in");

			// Find the object for the given ID
			Object* object = Object::GetObjectByID(objectID);
			ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectShow: no object with given ID");
			
			// Do the work
			object->Show();
		}
		catch(...)
		{
		}
	}

	Boolean  HardwarePlugIn_ObjectHasProperty(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address)
	{
		Boolean		answer = false;
		CAMutex*	objectStateMutex = NULL;
		bool		objectStateMutexNeedsUnlocking = false;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectHasProperty: no plug-in");
			ThrowIfNULL(address, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectHasProperty: no address");
			
			// Find the object for the given ID
			Object* object = Object::GetObjectByID(objectID);
			ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectHasProperty: no object with given ID");
			
			// Get the object state mutex
			objectStateMutex = Object::GetObjectStateMutexByID(objectID);
			
			// Lock the mutex
			if (objectStateMutex != NULL)
			{
				objectStateMutexNeedsUnlocking = objectStateMutex->Lock();
				
				// Re-find the object for the given ID since it may have changed while we blocked waiting for the lock
				object = Object::GetObjectByID(objectID);
				ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectHasProperty: no object with given ID after locking");
			}
			
			// Do the work
			answer = object->HasProperty(*address);
		}
		catch(const CAException& exception)
		{
			answer = false;
		}
		catch(...)
		{
			answer = false;
		}
		
		// Unlock the object state mutex if we need to
		if ((objectStateMutex != NULL) && objectStateMutexNeedsUnlocking)
		{
			objectStateMutex->Unlock();
		}
		
		return answer;
	}

	OSStatus HardwarePlugIn_ObjectIsPropertySettable(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, Boolean* isSettable)
	{
		OSStatus	error = kCMIOHardwareNoError;
		CAMutex*	objectStateMutex = NULL;
		bool		objectStateMutexNeedsUnlocking = false;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectIsPropertySettable: no plug-in");
			ThrowIfNULL(address, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectIsPropertySettable: no address");
			ThrowIfNULL(isSettable, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectIsPropertySettable: no place to store return value");
			
			// Find the object for the given ID
			Object* object = Object::GetObjectByID(objectID);
			ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectIsPropertySettable: no object with given ID");
			
			// Get the object state mutex
			objectStateMutex = Object::GetObjectStateMutexByID(objectID);
			
			// Lock the mutex
			if (objectStateMutex != NULL)
			{
				objectStateMutexNeedsUnlocking = objectStateMutex->Lock();
				
				// Re-find the object for the given ID since it may have changed while we blocked waiting for the lock
				object = Object::GetObjectByID(objectID);
				ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectIsPropertySettable: no object with given ID after locking");
			}
			
			// Do the work
			*isSettable = object->IsPropertySettable(*address);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		// Unlock the object state mutex if we need to
		if ((objectStateMutex != NULL) && objectStateMutexNeedsUnlocking)
		{
			objectStateMutex->Unlock();
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_ObjectGetPropertyDataSize(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, UInt32 qualifierDataSize, const void* qualifierData, UInt32* dataSize)
	{
		OSStatus	error = kCMIOHardwareNoError;
		CAMutex*	objectStateMutex = NULL;
		bool		objectStateMutexNeedsUnlocking = false;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyDataSize: no plug-in");
			ThrowIfNULL(address, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyDataSize: no address");
			ThrowIfNULL(dataSize, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyDataSize: no place to store return value");
			
			// Find the object for the given ID
			Object* object = Object::GetObjectByID(objectID);
			ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyDataSize: no object with given ID");
			
			// Get the object state mutex
			objectStateMutex = Object::GetObjectStateMutexByID(objectID);
			
			// Lock the mutex
			if (objectStateMutex != NULL)
			{
				objectStateMutexNeedsUnlocking = objectStateMutex->Lock();
				
				// Re-find the object for the given ID since it may have changed while we blocked waiting for the lock
				object = Object::GetObjectByID(objectID);
				ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyDataSize: no object with given ID after locking");
			}
			
			// Do the work
			*dataSize = object->GetPropertyDataSize(*address, qualifierDataSize, qualifierData);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		// Unlock the object state mutex if we need to
		if ((objectStateMutex != NULL) && objectStateMutexNeedsUnlocking)
		{
			objectStateMutex->Unlock();
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_ObjectGetPropertyData(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32* dataUsed, void* data)
	{
		OSStatus	error = kCMIOHardwareNoError;
		CAMutex*	objectStateMutex = NULL;
		bool		objectStateMutexNeedsUnlocking = false;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyData: no plug-in");
			ThrowIfNULL(address, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyData: no address");
			
			// Find the object for the given ID
			Object* object = Object::GetObjectByID(objectID);
			ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyData: no object with given ID");
			
			// Get the object state mutex
			objectStateMutex = Object::GetObjectStateMutexByID(objectID);
			
			// Lock the mutex
			if (objectStateMutex != NULL)
			{
				objectStateMutexNeedsUnlocking = objectStateMutex->Lock();
				
				// Re-find the object for the given ID since it may have changed while we blocked waiting for the lock
				object = Object::GetObjectByID(objectID);
				ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectGetPropertyData: no object with given ID after locking");
			}
			
			// Do the work
			object->GetPropertyData(*address, qualifierDataSize, qualifierData, dataSize, *dataUsed, data);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		// Unlock the object state mutex if we need to
		if ((objectStateMutex != NULL) && objectStateMutexNeedsUnlocking)
		{
			objectStateMutex->Unlock();
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_ObjectSetPropertyData(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		OSStatus	error = kCMIOHardwareNoError;
		CAMutex*	objectStateMutex = NULL;
		bool		objectStateMutexNeedsUnlocking = false;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectSetPropertyData: no plug-in");
			ThrowIfNULL(address, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_ObjectSetPropertyData: no address");
			
			// Find the object for the given ID
			Object* object = Object::GetObjectByID(objectID);
			ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectSetPropertyData: no object with given ID");
			
			// Get the object state mutex
			objectStateMutex = Object::GetObjectStateMutexByID(objectID);
			
			// Lock the mutex
			if (objectStateMutex != NULL)
			{
				objectStateMutexNeedsUnlocking = objectStateMutex->Lock();
				
				// Re-find the object for the given ID since it may have changed while we blocked waiting for the lock
				object = Object::GetObjectByID(objectID);
				ThrowIfNULL(object, CAException(kCMIOHardwareBadObjectError), "CMIO::DP::HardwarePlugIn_ObjectSetPropertyData: no object with given ID after locking");
			}
			
			// Do the work
			object->SetPropertyData(*address, qualifierDataSize, qualifierData, dataSize, data);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		// Unlock the object state mutex if we need to
		if ((objectStateMutex != NULL) && objectStateMutexNeedsUnlocking)
		{
			objectStateMutex->Unlock();
		}
		
		return error;
	}

	#pragma mark	CMIODevice Operations
	OSStatus HardwarePlugIn_DeviceStartStream(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIOStreamID streamID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceStartStream: no plug-in");
			
			// Find the device for the given ID
			Device* device = Object::GetDeviceByID(deviceID);
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_DeviceStart: no device with given ID");
			
			// Do the work
			device->StartStream(streamID);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_DeviceSuspend(CMIOHardwarePlugInRef self, CMIODeviceID deviceID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceSuspend: no plug-in");
			
			// Find the device for the given ID
			Device* device = Object::GetDeviceByID(deviceID);
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_DeviceSuspend: no device with given ID");
			
			// Do the work
			device->Suspend();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_DeviceResume(CMIOHardwarePlugInRef self, CMIODeviceID deviceID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceResume: no plug-in");
			
			// Find the device for the given ID
			Device* device = Object::GetDeviceByID(deviceID);
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_DeviceResume: no device with given ID");
			
			// Do the work
			device->Resume();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_DeviceStopStream(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIOStreamID streamID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceStreamStop: no plug-in");
			
			// Find the device for the given ID
			Device* device = Object::GetDeviceByID(deviceID);
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_DeviceStreamStop: no device with given ID");
			
			// Do the work
			device->StopStream(streamID);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_DeviceProcessAVCCommand(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIODeviceAVCCommand* ioAVCCommand)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceProcessAVCCommand: no plug-in");
			ThrowIfNULL(ioAVCCommand, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceProcessAVCCommand: no AVC commad");
			
			// Find the device for the given ID
			Device* device = Object::GetDeviceByID(deviceID);
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_DeviceProcessAVCCommand: no device with given ID");
			
			// Do the work
			device->ProcessAVCCommand(ioAVCCommand);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_DeviceProcessRS422Command(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIODeviceRS422Command* ioRS422Command)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceProcessRS422Command: no plug-in");
			ThrowIfNULL(ioRS422Command, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_DeviceProcessRS422Command: no RS422 commad");
			
			// Find the device for the given ID
			Device* device = Object::GetDeviceByID(deviceID);
			ThrowIfNULL(device, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_DeviceProcessRS422Command: no device with given ID");
			
			// Do the work
			device->ProcessRS422Command(ioRS422Command);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_StreamDeckPlay(CMIOHardwarePlugInRef self, CMIOStreamID streamID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_StreamDeckPlay: no plug-in");
			
			// Find the device for the given ID
			Stream* stream = Object::GetStreamByID(streamID);
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadStreamError), "CMIO::DP::HardwarePlugIn_StreamDeckPlay: no device with given ID");
			
			// Do the work
			stream->DeckPlay();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_StreamDeckStop(CMIOHardwarePlugInRef self,CMIOStreamID streamID)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_StreamDeckStop: no plug-in");
			
			// Find the device for the given ID
			Stream* stream = Object::GetStreamByID(streamID);
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadStreamError), "CMIO::DP::HardwarePlugIn_StreamDeckStop: no device with given ID");
			
			// Do the work
			stream->DeckStop();
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_StreamDeckJog(CMIOHardwarePlugInRef self, CMIOStreamID streamID, SInt32 speed)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_StreamDeckJog: no plug-in");
			
			// Find the device for the given ID
			Stream* stream = Object::GetStreamByID(streamID);
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_StreamDeckJog: no device with given ID");
			
			// Do the work
			stream->DeckJog(speed);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	OSStatus HardwarePlugIn_StreamDeckCueTo(CMIOHardwarePlugInRef self, CMIOStreamID streamID, Float64 requestedTimecode, Boolean	playOnCue)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_StreamDeckCueTo: no plug-in");
			
			// Find the device for the given ID
			Stream* stream = Object::GetStreamByID(streamID);
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadDeviceError), "CMIO::DP::HardwarePlugIn_StreamDeckCueTo: no device with given ID");
			
			// Do the work
			stream->DeckCueTo(requestedTimecode, playOnCue);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}

	#pragma mark	CMIOStream Operations
	OSStatus HardwarePlugIn_StreamCopyBufferQueue(CMIOHardwarePlugInRef self, CMIOStreamID streamID, CMIODeviceStreamQueueAlteredProc queueAlteredProc, void* queueAlteredRefCon, CMSimpleQueueRef* queue)
	{
		OSStatus error = kCMIOHardwareNoError;
		
		try
		{
			// Check the function arguments
			ThrowIfNULL(self, CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_StreamCopyBufferQueue: no plug-in");
			ThrowIf(((NULL == queue) and (NULL != queueAlteredProc)), CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::HardwarePlugIn_StreamCopyBufferQueue: no queue provided");
			
			// Find the stream for the given ID
			Stream* stream = Object::GetStreamByID(streamID);
			ThrowIfNULL(stream, CAException(kCMIOHardwareBadStreamError), "CMIO::DP::HardwarePlugIn_StreamCopyBufferQueue: no stream with given ID");
			
			// Do the work
			*queue = stream->CopyBufferQueue(queueAlteredProc, queueAlteredRefCon);
		}
		catch(const CAException& exception)
		{
			error = exception.GetError();
		}
		catch(...)
		{
			error = kCMIOHardwareUnspecifiedError;
		}
		
		return error;
	}
}

namespace CMIO { namespace DP
{
	CMIOHardwarePlugInInterface	PlugIn::sInterface = 
	{
		//	Padding for COM
		NULL,
		
		//	IUnknown Routines
		(HRESULT (*)(void*, CFUUIDBytes, void**))HardwarePlugIn_QueryInterface,
		(ULONG (*)(void*))HardwarePlugIn_AddRef,
		(ULONG (*)(void*))HardwarePlugIn_Release,
		
		//	DAL Plug-In Routines
		HardwarePlugIn_Initialize,
		HardwarePlugIn_InitializeWithObjectID,
		HardwarePlugIn_Teardown,
		HardwarePlugIn_ObjectShow,
		HardwarePlugIn_ObjectHasProperty,
		HardwarePlugIn_ObjectIsPropertySettable,
		HardwarePlugIn_ObjectGetPropertyDataSize,
		HardwarePlugIn_ObjectGetPropertyData,
		HardwarePlugIn_ObjectSetPropertyData,
		HardwarePlugIn_DeviceSuspend,
		HardwarePlugIn_DeviceResume,
		HardwarePlugIn_DeviceStartStream,
		HardwarePlugIn_DeviceStopStream,
		HardwarePlugIn_DeviceProcessAVCCommand,
		HardwarePlugIn_DeviceProcessRS422Command,
		HardwarePlugIn_StreamCopyBufferQueue,
		HardwarePlugIn_StreamDeckPlay,
		HardwarePlugIn_StreamDeckStop,
		HardwarePlugIn_StreamDeckJog,
		HardwarePlugIn_StreamDeckCueTo
	};
}}
