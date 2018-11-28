/*
	    File: CMIO_DALA_Object.h
	Abstract: C++ wrapper for CMIOObjectID
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

#if !defined(__CMIO_DALA_Object_h__)
#define __CMIO_DALA_Object_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// System Includes
#include <CoreMediaIO/CMIOHardware.h>
#include <CoreMediaIO/CMIOHardwareObject.h>

// Public Utility Includes
#include "CMIODebugMacros.h"

namespace CMIO { namespace DALA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Object
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Object
	{
	// Construction/Destruction
	public:
									Object(CMIOObjectID objectID);
		virtual						~Object();

	// Attributes
	public:
		CMIOObjectID				GetObjectID() const;
		CMIOClassID					GetClassID() const;
		CMIOObjectID				GetOwnerObjectID() const;
		CFStringRef					CopyOwningPlugInBundleID() const;
		CFStringRef					CopyName() const;
		CFStringRef					CopyManufacturer() const;
		CFStringRef					CopyNameForElement(CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) const;
		CFStringRef					CopyCategoryNameForElement(CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) const;
		CFStringRef					CopyNumberNameForElement(CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) const;

		static bool					IsSubClass(CMIOClassID classID, CMIOClassID baseClassID);
		static bool					ObjectExists(CMIOClassID inObjectID);

	// Owned Objects
	public:
		UInt32						GetNumberOwnedObjects(CMIOClassID classID) const;
		void						GetAllOwnedObjects(CMIOClassID classID, UInt32& ioNumberObjects, CMIOObjectID* ioObjectIDs) const;
		CMIOObjectID				GetOwnedObjectByIndex(CMIOClassID classID, UInt32 index);
		
	// Property Operations
	public:
		bool						HasProperty(const CMIOObjectPropertyAddress& address) const;
		bool						IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		UInt32						GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;

		void						GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		void						SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);
		
		UInt32						GetPropertyData_UInt32(const CMIOObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const										{  UInt32 theAnswer = 0; UInt32 dataUsed = 0; UInt32 theDataSize = SizeOf32(UInt32); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize, dataUsed, &theAnswer); return theAnswer; }
		void						SetPropertyData_UInt32(const CMIOObjectPropertyAddress& inAddress, UInt32 inValue, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)								{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, SizeOf32(UInt32), &inValue); }

		Float32						GetPropertyData_Float32(const CMIOObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const										{ Float32 theAnswer = 0; UInt32 dataUsed = 0; UInt32 theDataSize = SizeOf32(Float32); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize, dataUsed, &theAnswer); return theAnswer; }
		void						SetPropertyData_Float32(const CMIOObjectPropertyAddress& inAddress, Float32 inValue, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)							{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, SizeOf32(Float32), &inValue); }

		Float64						GetPropertyData_Float64(const CMIOObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const										{ Float64 theAnswer = 0; UInt32 dataUsed = 0; UInt32 theDataSize = SizeOf32(Float64); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize,dataUsed,  &theAnswer); return theAnswer; }
		void						SetPropertyData_Float64(const CMIOObjectPropertyAddress& inAddress, Float64 inValue, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)							{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, SizeOf32(Float64), &inValue); }

		CFTypeRef					GetPropertyData_CFType(const CMIOObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const										{ CFTypeRef theAnswer = NULL; UInt32 dataUsed = 0; UInt32 theDataSize = SizeOf32(CFTypeRef); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize, dataUsed, &theAnswer); return theAnswer; }
		void						SetPropertyData_CFType(const CMIOObjectPropertyAddress& inAddress, CFTypeRef inValue, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)							{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, SizeOf32(CFTypeRef), &inValue); }

		CFStringRef					GetPropertyData_CFString(const CMIOObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const										{ CFStringRef theAnswer = NULL; UInt32 dataUsed = 0; UInt32 theDataSize = SizeOf32(CFStringRef); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize, dataUsed, &theAnswer); return theAnswer; }
		void						SetPropertyData_CFString(const CMIOObjectPropertyAddress& inAddress, CFStringRef inValue, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)						{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, SizeOf32(CFStringRef), &inValue); }

		template <class T> void		GetPropertyData_Struct(const CMIOObjectPropertyAddress& inAddress, T& outStruct, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const							{ UInt32 dataUsed = 0; UInt32 theDataSize = SizeOf32(T); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize, dataUsed, &outStruct); }
		template <class T> void		SetPropertyData_Struct(const CMIOObjectPropertyAddress& inAddress, T& inStruct, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)									{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, SizeOf32(T), &inStruct); }

		template <class T> UInt32	GetPropertyData_ArraySize(const CMIOObjectPropertyAddress& inAddress, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const									{ return GetPropertyDataSize(inAddress, inQualifierDataSize, inQualifierData) / SizeOf32(T); }
		template <class T> void		GetPropertyData_Array(const CMIOObjectPropertyAddress& inAddress, UInt32& ioNumberItems, T* outArray, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL) const	{ UInt32 dataUsed = 0; UInt32 theDataSize = ioNumberItems * SizeOf32(T); GetPropertyData(inAddress, inQualifierDataSize, inQualifierData, theDataSize, dataUsed, outArray); ioNumberItems = dataUsed / SizeOf32(T); }
		template <class T> void		SetPropertyData_Array(const CMIOObjectPropertyAddress& inAddress, UInt32 inNumberItems, T* inArray, UInt32 inQualifierDataSize = 0, const void* inQualifierData = NULL)			{ SetPropertyData(inAddress, inQualifierDataSize, inQualifierData, inNumberItems * SizeOf32(T), inArray); }

		void						AddPropertyListener(const CMIOObjectPropertyAddress& address, CMIOObjectPropertyListenerProc listenerProc, void* clientData);
		void						RemovePropertyListener(const CMIOObjectPropertyAddress& address, CMIOObjectPropertyListenerProc listenerProc, void* clientData);

	// Implementation
	private:
		CMIOObjectID				mObjectID;
	};
}}
#endif
