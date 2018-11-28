/*
	    File: CMIO_PTA_Assistance.h
	Abstract: IMPORTANT IMPLEMENTATION NOTE:  THIS WILL HOPEFULLY BE REPLACED BY CAMutex.h AT SOME POINT IN THE FUTURE
	
				The POSIX Thread Assistant (PTA) consists of various objects to facilitate using the "Acquisition is Initialization" design pattern.
				NOTE: the PTA is sparsely implemented, meaning that it does not attempt to provide wrappers/access to all POSIX Thread features.
	
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

#if !defined(__CMIO_PTA_Assistance_h__)
#define __CMIO_PTA_Assistance_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CMIODebugMacros.h"
#include "CAException.h"

// System Includes
#include <pthread.h>

namespace CMIO { namespace PTA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Mutex
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Mutex
	{
	public:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Mutex::Condition
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class Condition
		{
			pthread_cond_t mCondition;
			Condition& operator=(Condition& that);										// Don't allow copying
		
		public:
		
			// Creators
			Condition()
			{
				int err = pthread_cond_init(&mCondition, 0);
				ThrowIfError(err, CAException(err), "CMIO::PTA::Mutex::Condition: pthread_cond_init() failed");
			}
			
			~Condition()												{ (void) pthread_cond_destroy(&mCondition); }

			// Conversion operators
			operator pthread_cond_t*()									{ return &mCondition; }
	
			// Accessors
			pthread_cond_t*	Get()										{ return &mCondition; }													
		};
		
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Mutex::Attribute
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class Attribute
		{
			pthread_mutexattr_t mAttribute;
			Attribute& operator=(Attribute& that);										// Don't allow copying
		
		public:
		
			// Creators
			Attribute()
			{
				int err = pthread_mutexattr_init(&mAttribute);
				ThrowIfError(err, CAException(err), "CMIO::PTA::Mutex::Attribute: pthread_mutexattr_init() failed");
			}
			
			~Attribute()												{ (void) pthread_mutexattr_destroy(&mAttribute); }

			// Conversion operators
			operator pthread_mutexattr_t*()								{ return &mAttribute; }
	
			// Accessors
			pthread_mutexattr_t*	Get()								{ return &mAttribute; }													
		};
		
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Mutex::Locker
		//	Helper class to provide stack based locking / unlocking of the mutex
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		class Locker
		{
			Mutex& mMutex;
			Locker& operator=(Locker& that);										// Don't allow copying
		
		public:
		
			// Creators
			Locker(Mutex& mutex) :
				mMutex(mutex)
			{
				int err = pthread_mutex_lock(mutex);
				ThrowIfError(err, CAException(err), "CMIO::PTA::Mutex::Locker: pthread_mutex_lock() failed");
			}
			
			~Locker()															{ (void) pthread_mutex_unlock(mMutex); }
		};
		
		
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Mutex
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		Mutex(int type = PTHREAD_MUTEX_DEFAULT)
		{
			Attribute attribute;
			int err = pthread_mutexattr_settype(attribute, type);
			ThrowIfError(err, CAException(err), "CMIO::PTA::Mutex: pthread_mutexattr_settype() failed");
				
			err = pthread_mutex_init(&mMutex, attribute);
			ThrowIfError(err, CAException(err), "CMIO::PTA::Mutex: pthread_mutex_init() failed");
		}
		  
		~Mutex()																{ (void) pthread_mutex_destroy(&mMutex); }
			
		// Conversion operators
		operator pthread_mutex_t*()												{ return &mMutex; }

		// Accessors
		pthread_mutex_t*	Get()												{ return &mMutex; }													

	private:
		pthread_mutex_t	mMutex;
		Mutex& operator=(Mutex& that);											// Don't allow copying
	};
}}

#endif
