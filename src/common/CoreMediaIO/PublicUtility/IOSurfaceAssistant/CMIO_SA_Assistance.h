/*
	    File: CMIO_SA_Assistance.h
	Abstract: The IOSurface Assisistant (SA) consists of various objects to facilitate using the "Acquisition is Initialization" design pattern.
				NOTE: the SA is sparsely implemented, meaning that it does not attempt to provide wrappers/access to all IOSurface features, just those needed.
	
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

#if !defined(__CMIO_SA_Assistance_h__)
#define __CMIO_SA_Assistance_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"

// System Includes
#include <IOSurface/IOSurface.h>

namespace CMIO { namespace SA
{
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Surface
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Surface
	{
	// Construction/Destruction
	public:
							Surface(IOSurfaceRef ref = NULL, bool retain = false, bool release = true) : mSurface(NULL) { Reset(ref, retain, release); }
		virtual				~Surface() { Reset(); }
		void				Reset(IOSurfaceRef ref = NULL, bool retain = false, bool release = true) { if (NULL != mSurface and mRelease) CFRelease(mSurface); if (retain and NULL != ref) CFRetain(ref); mSurface = ref; mRelease = release; }
		Surface&			operator=(const Surface& that) { if (this != &that) this->Reset(that.Get(), true, that.WillRelease());  return *this; }
        
	// Exception throwing versions of the CMBlockBufferCreateXXXXX routines for the underlying CMBlockBufferRef
	public:
		static IOSurfaceRef	Create(CFDictionaryRef properties) { IOSurfaceRef surface = IOSurfaceCreate(properties); ThrowIfNULL(surface, CAException(kIOReturnNoResources), "CMIO::SA::Surface::Create() - failed"); return surface; }
		static IOSurfaceRef	LookupFromMachPort(mach_port_t port) { IOSurfaceRef surface = IOSurfaceLookupFromMachPort(port); ThrowIfNULL(surface, CAException(kIOReturnNoResources), "CMIO::SA::Surface::LookupFromMachPort() - failed"); return surface; }

	private:
		IOSurfaceRef		mSurface;
		bool				mRelease;
		
	// Attributes
	public:
		bool				IsValid() const { return NULL != mSurface; }
		bool				WillRelease() const { return mRelease; }
		void				ShouldRelease(bool release) { mRelease = release; }

   // Operations
	public:
	mach_port_t				CreateMachPort() { mach_port_t port = IOSurfaceCreateMachPort(mSurface); ThrowIf(MACH_PORT_NULL == port, CAException(kIOReturnNoResources), "CMIO::SA::Surface::CreateMachPort() - failed"); return port; }
	
	// Value Access
	public:
		operator			IOSurfaceRef() const { return mSurface; }
		IOSurfaceRef		Get() const { return mSurface; }													
	};
}}

#endif
