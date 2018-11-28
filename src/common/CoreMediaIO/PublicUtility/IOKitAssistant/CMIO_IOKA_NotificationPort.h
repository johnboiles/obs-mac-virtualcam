/*
	    File: CMIO_IOKA_NotificationPort.h
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

#if !defined(__CMIO_IOKA_NotificationPort_h__)
#define __CMIO_IOKA_NotificationPort_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <IOKit/pwr_mgt/IOPM.h>

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_IOKA_Object.h"

namespace CMIO { namespace IOKA
{
	IONotificationPortRef AllocateNotificationPort();
	void IsClamshellClosed( bool *isClosed);
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// NotificationPort
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class NotificationPort
	{
	// Construction/Destruction
	public:
								NotificationPort(IONotificationPortRef notificationPort = NULL, CFRunLoopRef runLoop = NULL, CFStringRef runLoopMode = NULL) : mNotificationPort(NULL), mRunLoop(NULL), mRunLoopMode(NULL) { Reset(notificationPort, runLoop, runLoopMode);}
								~NotificationPort() { Reset(); }
		
		void					Reset(IONotificationPortRef notificationPort = NULL, CFRunLoopRef runLoop = NULL, CFStringRef runLoopMode = NULL)
								{
									if (NULL != mNotificationPort)
									{
										// Remove the notification port's run loop source and then destroy the port
										CFRunLoopRemoveSource(mRunLoop, IONotificationPortGetRunLoopSource(mNotificationPort), mRunLoopMode);
										IONotificationPortDestroy(mNotificationPort);
										
										mNotificationPort = NULL;
										mRunLoop = NULL;
										mRunLoopMode = NULL;
									}
									
									if (NULL != notificationPort)
									{
										mNotificationPort = notificationPort;

										if (NULL != runLoop and NULL != runLoopMode)
										{
											// Add the notification port's run loop source to the specified run loop
											CFRunLoopAddSource(runLoop, IONotificationPortGetRunLoopSource(mNotificationPort), runLoopMode);
											mRunLoop = runLoop;
											mRunLoopMode = runLoopMode;
										}
									}
								}

	private:
		NotificationPort&		operator=(NotificationPort& that);		// Don't allow copying
		IONotificationPortRef	mNotificationPort;						// Port on which notifications are received		
		CFRunLoopRef			mRunLoop;								// The run loop to which the the port's run loop source was added		
		CFStringRef				mRunLoopMode;							// The mode which the port's port's run loop source was operating

	// Operations
	public:
		void					AddInterestNotification(io_service_t service, const io_name_t interestType, IOServiceInterestCallback callback, void* refCon, IOKA::Object& notifier)
								{
									IOReturn ioReturn = IOServiceAddInterestNotification(mNotificationPort, service, interestType, callback, refCon, notifier.GetAddress());
									ThrowIfError(ioReturn, CAException(ioReturn), "CMIO::IOKA::NotificationPort::AddInterestNotification: IOServiceAddInterestNotification() failed");
								}

	// Value Access
	public:
		operator				IONotificationPortRef()	const { return mNotificationPort; }
		IONotificationPortRef	Get() const { return mNotificationPort; }													
	};
}}

#endif

