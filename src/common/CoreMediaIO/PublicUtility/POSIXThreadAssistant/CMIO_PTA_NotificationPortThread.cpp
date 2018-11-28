/*
	    File: CMIO_PTA_NotificationPortThread.cpp
	Abstract: A thread for getting messages on a IOKit Notifcation port (using the CMIO::IOKA::NotificationPort wrappers)
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

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_PTA_NotificationPortThread.h"

// System Includes
#include <objc/objc-auto.h>

namespace CMIO { namespace PTA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Reset()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void NotificationPortThread::Reset(bool spawn)
	{
		// Wait for the thread to be running or invalid prior to tearing it down.  
		while (kStarting == mState)
			pthread_yield_np();

		if (kRunning == mState)
		{
			// Wait for the thread's run loop to be "waiting."  This will avoid a small window of having set 'mState = kRunning' prior but not having invoked CFRunLoopRun() yet.
			while (not CFRunLoopIsWaiting(mRunLoop.GetCFObject()))
				pthread_yield_np();
				
			// Stop the thread's run loop
			CFRunLoopStop(mRunLoop.GetCFObject());
			
			// Set the thread's state to kStopping
			mState = kStopping;
			
			// Wait for the thread to run to completion (it will set the state to kInvalid) 
			while (kStopping == mState)
			{
				(void) pthread_cond_signal(mStoppingCondition);
				pthread_yield_np();
			}
		}
		
		mNotificationPort.Reset();
		mPThread	= 0;
		mRunLoop	= 0;
		mState		= kInvalid;

		if (spawn)
		{
			typedef void* (*PThreadStart)(void*);
			
			mState	= kStarting;
			int err = pthread_create(&mPThread, 0, reinterpret_cast<PThreadStart>(Start), this);
			ThrowIfError(err, CAException(err), "CMIO::PTA::NotificationPortThread::Reset: pthread_create() failed");
				
			(void) pthread_detach(mPThread);
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Start()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* NotificationPortThread::Start(NotificationPortThread* thread)
	{
		// Register this thread the the garbage collector if needed
		#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_5)
			objc_registerThreadWithCollector();
		#endif
		
		thread->mRunLoop = CFRunLoopGetCurrent();
		
		try
		{
			// Setup the IOKA::NotificationPort used by this thread
			thread->mNotificationPort.Reset(IOKA::AllocateNotificationPort(), CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
			
			// Indicate that the run loop will need to be stopped during the next call to Reset()
			thread->mState = kRunning;
	
			CFRunLoopRun();

			// In the unlikey event that CFRunLoopRun() returned prior to CFRunLoopStop() being called in Reset(), wait until the stopping condition has been explicitly signaled prior to
			// letting the thread run to completion.  This will make sure that the run loop is still valid when CFRunLoopStop() is called.
			{
				PTA::Mutex::Locker locker(thread->mStoppingMutex);
				(void) pthread_cond_wait(thread->mStoppingCondition, thread->mStoppingMutex);
				thread->mState = kInvalid;
			}
		}
		catch (...)
		{
			thread->mState = kInvalid;
			return reinterpret_cast<void*>(-1);
		}

		return 0;
	}
}}
