/*
	    File: CMIO_IOKA_Object.h
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

#if !defined(__CMIO_IOKA_Object_h__)
#define __CMIO_IOKA_Object_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"

// System Includes
#include <IOKit/IOKitLib.h>

namespace CMIO { namespace IOKA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Object
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Object
	{
	// Construction/Destruction
	public:
						Object(const Object& object) : mReference(IO_OBJECT_NULL) { Reset(object); }
		explicit		Object(io_object_t reference = IO_OBJECT_NULL) : mReference(IO_OBJECT_NULL) { Reset(reference); }
						~Object() { if (IO_OBJECT_NULL != mReference) (void) IOObjectRelease(mReference); }
		Object&			operator=(const Object& that) { if (this != &that) this->Reset(that); return *this; }
		  
		void			Reset(io_object_t reference = IO_OBJECT_NULL) { if (IO_OBJECT_NULL != mReference) (void) IOObjectRelease(mReference); mReference = reference; }
		void			Reset(const Object& object)
						{
							if (IO_OBJECT_NULL != mReference)
							{
								(void) IOObjectRelease(mReference);
								mReference = IO_OBJECT_NULL;
							}
							
							if (IO_OBJECT_NULL != object.Get())
							{
								IOReturn ioReturn = IOObjectRetain(object.Get());
								ThrowIfError(ioReturn, CAException(ioReturn), "CMIO::IOKA::Object::Reset: IOObjectRetain() failed");
								mReference = object.Get();
							}
						}

	private:
		io_object_t		mReference;

	// Attributes
	public:
		bool			IsValid() const { return IO_OBJECT_NULL !=  mReference; }
		bool			ConformsTo(const io_name_t className) { return IOObjectConformsTo(mReference, className); }

	// Value Access
	public:
		operator		io_object_t() const { return mReference; }
		io_object_t		Get() const { return mReference; }													
		io_object_t*	GetAddress() { return &mReference; }													
	};

}}
#endif

