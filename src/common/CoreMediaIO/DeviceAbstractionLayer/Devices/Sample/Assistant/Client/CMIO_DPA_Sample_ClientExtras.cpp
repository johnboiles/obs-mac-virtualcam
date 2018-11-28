/*
	    File: CMIO_DPA_Sample_ClientExtras.cpp
	Abstract: Extra utility functions used by clients of the SampleAssistant.
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
#include "CMIO_DPA_Sample_ClientExtras.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CACFData.h"
#include "CACFObject.h"
#include "CAException.h"
#include "CAProcess.h"

// System Includes
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <mach/mach_error.h>
#include <unistd.h>


namespace CMIO { namespace DPA { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPort()
	//	Returns the port which the all messages will be sent to the SampleAssistant.  Under normal circumstances, the SampleAssistant service will have been registered with the bootstrap
	//	via an appropriately configured plist in /Library/LaunchDaemons.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	mach_port_t GetPort()
	{
		mach_port_t port = MACH_PORT_NULL;
		int connectionAttempts = 0;
		
		while (MACH_PORT_NULL == port)
		{
			// Lookup the SampleAssistant service from the bootstrap mechanism
			mach_port_t assistantServicePort;		
			name_t assistantServiceName = "com.apple.cmio.DPA.Sample";
			kern_return_t err = bootstrap_look_up(bootstrap_port, assistantServiceName, &assistantServicePort);
			ThrowIfKernelError(err, CAException(err), "CMIO::DPA::Sample::GetPort: bootstrap_look_up() failed");
	
			// Invoke the SampleAssistantConnect() routine to get the port on which future messages will be sent
			err = CMIODPASampleConnect(assistantServicePort, CAProcess::GetPID(), &port);
	
			// The service port is not longer needed so get rid of it
			(void) mach_port_deallocate(mach_task_self(), assistantServicePort);
			
			// Make sure the call to SampleAssistantConnect() succeeded
			if (KERN_SUCCESS != err)
			{
				// If the error is MACH_SEND_INVALID_DEST, that means that that there was a race condition where the Assistant was in the process of shutting down because its only existing
				// client had quit while this new client was trying to establish a connection, so simply run through the loop again.
				// The # of connection attempts is tracked to prevent an endless loop.
				if (MACH_SEND_INVALID_DEST == err and ++connectionAttempts < 5)
					continue;
				
				// Something more problematic must have happened, so throw an exception
				ThrowIfKernelError(err, err, "CMIO::DPA::Sample::GetPort: - couldn't send Connect message");
			}
		}
		
		return port;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Disconnect()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Disconnect(mach_port_t port)
	{
		IOReturn ioReturn = CMIODPASampleDisconnect(port);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::Disconnect:");
	}
	
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceStates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GetDeviceStates(mach_port_t port, mach_port_t messagePort, AutoFreeUnboundedArray<DeviceState>& deviceStates)
	{
		IOReturn ioReturn = CMIODPASampleGetDeviceStates(port, messagePort, deviceStates.GetAddress(), &deviceStates.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeviceStates:");
	}

	#pragma mark -
	#pragma mark Controls
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyControlList()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFArrayRef CopyControlList(mach_port_t clientSendPort, UInt64 guid)
	{
		// Get the raw XML data from the Assistant 
		AutoFreeUnboundedArray<UInt8> xmlRaw;
		IOReturn ioReturn = CMIODPASampleGetControlList(clientSendPort, guid, xmlRaw.GetByteAddress(), &xmlRaw.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetControlList: CMIODPASampleGetControlList() failed");
		
		// Convert the raw XML data into CFData
		CACFData xmlData(CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, reinterpret_cast<UInt8 *>(&xmlRaw[0]), xmlRaw.GetLength(), kCFAllocatorNull));
		ThrowIf(not xmlData.IsValid(), CAException(kCMIOHardwareUnspecifiedError), "CMIO::DPA::Sample::ClientExtras::CopyControlList: xmlData not valid");

		// Create a property list from the CFData
		#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6)
			CFArrayRef controlList = reinterpret_cast<CFArrayRef>(CFPropertyListCreateWithData(kCFAllocatorDefault, xmlData.GetCFData(), kCFPropertyListMutableContainersAndLeaves, NULL, NULL));
		#else
			CFArrayRef controlList = reinterpret_cast<CFArrayRef>(CFPropertyListCreateFromXMLData(kCFAllocatorDefault, xmlData.GetCFData(), kCFPropertyListMutableContainersAndLeaves, NULL));
		#endif
		ThrowIfNULL(controlList, CAException(kCMIOHardwareUnspecifiedError), "CMIO::DPA::Sample::ClientExtras::CopyControlList: controlList not valid");

		return controlList;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControls()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GetControls(mach_port_t port, UInt64 guid, mach_port_t messagePort, UInt64 time, AutoFreeUnboundedArray<ControlChanges>& controlChanges)
	{
		IOReturn ioReturn = CMIODPASampleGetControls(port, guid, messagePort, time, controlChanges.GetAddress(), &controlChanges.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetControls: failed");
	}
	
	#pragma mark -
	#pragma mark Properties
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetProperties()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GetProperties(mach_port_t port, UInt64 guid, mach_port_t messagePort, UInt64 time, const PropertyAddress& matchAddress, AutoFreeUnboundedArray<PropertyAddress>& addresses)
	{
		IOReturn ioReturn = CMIODPASampleGetProperties(port, guid, messagePort, time, matchAddress, addresses.GetAddress(), &addresses.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetProperties: failed");
	}

	#pragma mark -
	#pragma mark Properties at the Device Level
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStreamConfiguration()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GetStreamConfiguration(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, AutoFreeUnboundedArray<UInt32>& configuration)
	{
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIODevicePropertyStreamConfiguration, scope), NULL, 0, configuration.GetByteAddress(), &configuration.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeviceIsRunningSomewhere: failed");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		configuration.CorrectLength();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceIsRunningSomewhere()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool GetDeviceIsRunningSomewhere(mach_port_t port, UInt64 guid)
	{
		AutoFreeUnboundedArray<Byte> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIODevicePropertyDeviceIsRunningSomewhere), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeviceIsRunningSomewhere: failed");
		ThrowIf(data.GetSize() != sizeof(Byte), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetDeviceIsRunningSomewhere: wrong length returned");
		return data[0] ? true : false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetHogMode()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	pid_t GetHogMode(mach_port_t port, UInt64 guid)
	{
		AutoFreeUnboundedArray<pid_t> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIODevicePropertyHogMode), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetHogMode: failed");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();

		ThrowIf(data.GetSize() != sizeof(pid_t), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetHogMode: wrong length returned");
		return data[0];
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceMaster()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	pid_t GetDeviceMaster(mach_port_t port, UInt64 guid)
	{
		AutoFreeUnboundedArray<pid_t> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIODevicePropertyDeviceMaster), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeviceMaster: failed");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();

		ThrowIf(data.GetSize() != sizeof(pid_t), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetDeviceMaster: wrong length returned");
		return data[0];
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceMaster()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetDeviceMaster(mach_port_t port, UInt64 guid, pid_t pid, bool sendChangedNotifications)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(port, guid, sendChangedNotifications, PropertyAddress(kCMIODevicePropertyDeviceMaster), NULL, 0, reinterpret_cast<Byte*>(&pid), sizeof(pid_t));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetDeviceMaster: failed");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetExcludeNonDALAccess()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool GetExcludeNonDALAccess(mach_port_t port, UInt64 guid)
	{
		AutoFreeUnboundedArray<Byte> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIODevicePropertyExcludeNonDALAccess), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetExcludeNonDALAccess: failed");
		ThrowIf(data.GetSize() != sizeof(Byte), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetExcludeNonDALAccess: wrong length returned");
		return data[0] ? true : false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetExcludeNonDALAccess()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetExcludeNonDALAccess(mach_port_t port, UInt64 guid, Boolean excludeNonDALAccess, bool sendChangedNotifications)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(port, guid, sendChangedNotifications, PropertyAddress(kCMIODevicePropertyExcludeNonDALAccess), NULL, 0, reinterpret_cast<Byte*>(&excludeNonDALAccess), sizeof(Boolean));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetExcludeNonDALAccess: failed");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetClientSyncDiscontinuity()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool GetClientSyncDiscontinuity(mach_port_t port, UInt64 guid)
	{
		AutoFreeUnboundedArray<Byte> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIODevicePropertyClientSyncDiscontinuity), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetClientSyncDiscontinuity: failed");
		ThrowIf(data.GetSize() != sizeof(Byte), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetClientSyncDiscontinuity: wrong length returned");
		return data[0] ? true : false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetClientSyncDiscontinuity()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetClientSyncDiscontinuity(mach_port_t port, UInt64 guid, Boolean forceDiscontinuity, bool sendChangedNotifications)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(port, guid, sendChangedNotifications, PropertyAddress(kCMIODevicePropertyClientSyncDiscontinuity), NULL, 0, reinterpret_cast<Byte*>(&forceDiscontinuity), sizeof(Boolean));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetClientSyncDiscontinuity: failed");
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetControl(mach_port_t port, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue)
	{
		IOReturn ioReturn = CMIODPASampleSetControl(port, guid, controlID, value, newValue);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetControl: failed");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ProcessRS422Command()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ProcessRS422Command(mach_port_t clientSendPort, UInt64 guid, CMIODeviceRS422Command& ioRS422Command)
	{
		AutoFreeUnboundedArray<UInt8> response;
		
		IOReturn ioReturn = CMIODPASampleProcessRS422Command(clientSendPort, guid, ioRS422Command.mCommand, ioRS422Command.mCommandLength, ioRS422Command.mResponseLength, &ioRS422Command.mResponseUsed, response.GetByteAddress(), &response.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::ProcessRS422Command:");
		
		for (UInt32 i = 0 ; i < ioRS422Command.mResponseUsed ; ++i)
			ioRS422Command.mResponse[i] = response[i];
	}
	
	#pragma mark -
	#pragma mark All Streams
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void StartStream(mach_port_t port, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		IOReturn ioReturn = CMIODPASampleStartStream(port, guid, messagePort, scope, element);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::StartStream:");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopStream()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void StopStream(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		IOReturn ioReturn = CMIODPASampleStopStream(port, guid, scope, element);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::StopStream:");
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFormatDescriptions()
	//	This gets an array of FrameFormats from the Assistant from which the CMFormatDescriptionRefs will be synthesized.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GetFormatDescriptions(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, AutoFreeUnboundedArray<FrameFormat>& frameFormats)
	{
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIOStreamPropertyFormatDescriptions, scope, element), NULL, 0, frameFormats.GetByteAddress(), &frameFormats.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetFormatDescriptions:");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		frameFormats.CorrectLength();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFormatDescription()
	//	This gets the current FrameType which is then mapped to the corresponding CMFormatDescriptionRef.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	FrameType GetFormatDescription(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<FrameType> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIOStreamPropertyFormatDescription, scope, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetFormatDescription:");
		
		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();

		ThrowIf(data.GetSize() != sizeof(FrameType), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::CMIODPASampleGetPropertyState: wrong length returned");
		return data[0];
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetFormatDescription()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetFormatDescription(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, FrameType frameType)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(port, guid, true, PropertyAddress(kCMIOStreamPropertyFormatDescription, scope, element), NULL, 0, reinterpret_cast<Byte*>(&frameType), sizeof(FrameType));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetFormatDescription:");
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFrameRates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void GetFrameRates(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, FrameType frameType, AutoFreeUnboundedArray<Float64>& frameRates)
	{
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIOStreamPropertyFrameRates, scope, element), reinterpret_cast<Byte*>(&frameType), sizeof(FrameType), frameRates.GetByteAddress(), &frameRates.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetFormatDescriptions:");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		frameRates.CorrectLength();
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Float64 GetFrameRate(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<Float64> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(port, guid, PropertyAddress(kCMIOStreamPropertyFrameRate, scope, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetFrameRate:");
		
		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();

		ThrowIf(data.GetSize() != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetNoDataTimeout: wrong length returned");
		return data[0];
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetFrameRate(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 frameRate)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(port, guid, true, PropertyAddress(kCMIOStreamPropertyFrameRate, scope, element), NULL, 0, reinterpret_cast<Byte*>(&frameRate), sizeof(Float64));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetFrameRateControl:");
	}
	
	#pragma mark -
	#pragma mark Streams with Decks
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StartDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void StartDeckThreads(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		IOReturn ioReturn = CMIODPASampleStartDeckThreads(port, guid, scope, element);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::StartDeckThreads: failed");
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// StopDeckThreads()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void StopDeckThreads(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		IOReturn ioReturn = CMIODPASampleStopDeckThreads(port, guid, scope, element);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::StopDeckThreads: failed");
	}
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeck()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMIOStreamDeck GetDeck(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<CMIOStreamDeck> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(clientSendPort, guid, PropertyAddress(kCMIOStreamPropertyDeck, scope, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeck: failed");
		
		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();
		
		ThrowIf(data.GetSize() != sizeof(CMIOStreamDeck), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetDeck: wrong length returned");
		return data[0];
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeckTimecode()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Float64 GetDeckTimecode(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<Float64> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(clientSendPort, guid, PropertyAddress(kCMIOStreamPropertyDeckFrameNumber, scope, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeckTimecode: failed");
		
		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();
		
		ThrowIf(data.GetSize() != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetDeckTimecode: wrong length returned");
		return data[0];
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeckCueing()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	SInt32 GetDeckCueing(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<SInt32> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(clientSendPort, guid, PropertyAddress(kCMIOStreamPropertyDeckCueing, scope, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeckCueing: failed");
		
		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();
		
		ThrowIf(data.GetSize() != sizeof(SInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetDeckCueing: wrong length returned");
		return data[0];
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckPlay()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeckPlay(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		IOReturn ioReturn = CMIODPASampleDeckPlay(port, guid, scope, element);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::DeckPlay:");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckStop()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeckStop(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
	{
		IOReturn ioReturn = CMIODPASampleDeckStop(port, guid, scope, element);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::DeckStop:");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckJog()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeckJog(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed)
	{
		IOReturn ioReturn = CMIODPASampleDeckJog(port, guid, scope, element, speed);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::DeckJog:");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DeckCueTo()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DeckCueTo(mach_port_t port, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, bool	playOnCue)
	{
		IOReturn ioReturn = CMIODPASampleDeckCueTo(port, guid, scope, element, requestedTimecode, playOnCue);
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::DeckCueTo:");
	}
	
	#pragma mark -
	#pragma mark Input Streams
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNoDataTimeout()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 GetNoDataTimeout(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<UInt32> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(clientSendPort, guid, PropertyAddress(kCMIOStreamPropertyNoDataTimeoutInMSec, kCMIODevicePropertyScopeInput, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetNoDataTimeout: failed");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();

		ThrowIf(data.GetSize() != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetNoDataTimeout: wrong length returned");
		return data[0];
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetNoDataTimeout()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetNoDataTimeout(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element, UInt32 noDataTimeout, bool sendChangedNotifications)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(clientSendPort, guid, sendChangedNotifications, PropertyAddress(kCMIOStreamPropertyNoDataTimeoutInMSec, kCMIODevicePropertyScopeInput, element), NULL, 0, reinterpret_cast<Byte*>(&noDataTimeout), sizeof(UInt32));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetNoDataTimeout: failed");
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetDeviceSyncTimeout()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 GetDeviceSyncTimeout(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<UInt32> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(clientSendPort, guid, PropertyAddress(kCMIOStreamPropertyDeviceSyncTimeoutInMSec, kCMIODevicePropertyScopeInput, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetDeviceSyncTimeout: failed");

		// Correct the data's length to properly reflect the # of elements (since CMIODPASampleGetPropertyState() returned the number of bytes)
		data.CorrectLength();

		ThrowIf(data.GetSize() != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetDeviceSyncTimeout: wrong length returned for UInt32");
		return data[0];
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetDeviceSyncTimeout()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetDeviceSyncTimeout(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element, UInt32 deviceSyncTimeout, bool sendChangedNotifications)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(clientSendPort, guid, sendChangedNotifications, PropertyAddress(kCMIOStreamPropertyDeviceSyncTimeoutInMSec, kCMIODevicePropertyScopeInput, element), NULL, 0, reinterpret_cast<Byte*>(&deviceSyncTimeout), sizeof(UInt32));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetDeviceSyncTimeout: failed");
	}

	#pragma mark -
	#pragma mark Output Streams
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetEndOfData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool GetEndOfData(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element)
	{
		AutoFreeUnboundedArray<Byte> data;
		IOReturn ioReturn = CMIODPASampleGetPropertyState(clientSendPort, guid, PropertyAddress(kCMIOStreamPropertyEndOfData, kCMIODevicePropertyScopeOutput, element), NULL, 0, data.GetByteAddress(), &data.GetLength());
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::GetEndOfData: failed");
		ThrowIf(data.GetSize() != sizeof(Byte), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DPA::Sample::ClientExtras::GetEndOfData: wrong length returned");
		return data[0] ? true : false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetEndOfData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetEndOfData(mach_port_t clientSendPort, UInt64 guid, CMIOObjectPropertyElement element, UInt32 endOfData, bool sendChangedNotifications)
	{
		IOReturn ioReturn = CMIODPASampleSetPropertyState(clientSendPort, guid, sendChangedNotifications, PropertyAddress(kCMIOStreamPropertyEndOfData, kCMIODevicePropertyScopeOutput, element), NULL, 0, reinterpret_cast<Byte*>(&endOfData), sizeof(UInt32));
		ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::DPA::Sample::ClientExtras::SetEndOfData: failed");
	}
}}}

