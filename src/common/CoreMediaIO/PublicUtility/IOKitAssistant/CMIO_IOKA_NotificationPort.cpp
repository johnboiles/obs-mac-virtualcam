/*
	    File: CMIO_IOKA_NotificationPort.cpp
	Abstract: The IOKit Assisistant (IOKA) consists of various objects to facilitate using the "Acquisition is Initialization" design pattern.
				NOTE: the IOKA is sparsely implemented, meaning that it does not attempt to provide wrappers/access to all IOKit features, but just those needed in the CMIO namespace.
	
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

#include "CMIO_IOKA_NotificationPort.h"

namespace CMIO { namespace IOKA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AllocateNotificationPort()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	IONotificationPortRef AllocateNotificationPort()
	{
		// Retrieve the IOKit's master port so a notification port can be created
		mach_port_t masterPort;
		IOReturn ioReturn = IOMasterPort(MACH_PORT_NULL, &masterPort);
		ThrowIfError(ioReturn, CAException(ioReturn), "CMIO::IOKA::AllocateNotificationPort: IOMasterPort() failed");

		// Create the notification port
		IONotificationPortRef notificationPort = IONotificationPortCreate(masterPort);
		ThrowIfNULL(notificationPort, CAException(-1), "CMIO::IOKA::AllocateNotificationPort: IONotificationPortCreate() failed");

		return notificationPort;
	}

	//--------------------------------------------------------------------------------------------------------------------
	// IsClamshellClosed( &bool isClosed)
	//
	// On Intel-based Powerbooks with a Built-in iSight, the "AppleClamshellClosed" state indicates whether the lid is closed or not.
	// We need to look and see if this property exists to know the current state of the lid.  The property is published in the IOPMRootDomain.
	//--------------------------------------------------------------------------------------------------------------------
	void IsClamshellClosed( bool *isClosed)
	{
		io_registry_entry_t		rootDomain;
		mach_port_t				masterPort;
		CFTypeRef				clamShellStateRef = NULL;
		
		// Retrieve the IOKit's master port so a notification port can be created
		IOReturn ioReturn = IOMasterPort(MACH_PORT_NULL, &masterPort);
		ThrowIfError(ioReturn, CAException(ioReturn), "CMIO::IOKA::IsClamshellClosed: IOMasterPort() failed");
		
		// Check to see if the "AppleClamshellClosed" property is in the PM root domain:
		rootDomain = IORegistryEntryFromPath(masterPort, kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
		ThrowIf(rootDomain == MACH_PORT_NULL, CAException(-1), "CMIO::IOKA::IsClamshellClosed: IORegistryEntryFromPath returned NULL root domain");
		
		clamShellStateRef = IORegistryEntryCreateCFProperty( rootDomain,CFSTR(kAppleClamshellStateKey), kCFAllocatorDefault, 0);
		if (clamShellStateRef == NULL)
		{
			DebugMessageLevel(5, "CMIO::IOKA::IsClamshellClosed  Could not find AppleClamshellState in IOPMRootDomain");
			*isClosed = false;
			if ( rootDomain )
				IOObjectRelease(rootDomain);
			ioReturn = kIOReturnNoResources;
			ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::IOKA::IsClamshellClosed: Could not find AppleClamshellState in IOPMRootDomain");
		}
		
		if ( CFBooleanGetValue( (CFBooleanRef)(clamShellStateRef) ) == true )
		{
			DebugMessageLevel(6,"CMIO::IOKA::IsClamshellClosed is TRUE");
			*isClosed = true;
		}
		else if ( CFBooleanGetValue( (CFBooleanRef)(clamShellStateRef) ) == false )
		{
			DebugMessageLevel(6,"CMIO::IOKA::IsClamshellClosed is FALSE");
			*isClosed = false;
		}
		
		if ( rootDomain )
			IOObjectRelease(rootDomain);
		
		if (clamShellStateRef)
			CFRelease(clamShellStateRef);
		
		ThrowIfError(ioReturn, CAException(ioReturn), "CMIO::IOKA::IsClamshellClosed: IOMasterPort() failed");
	}
	
}}
