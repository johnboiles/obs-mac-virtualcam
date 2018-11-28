/*
	    File: CMIO_DP_IOV_ControlDictionary.cpp
	Abstract: User space utility functions that extract items from an IOV control dictionary.
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
#include "CMIO_DP_IOV_ControlDictionary.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"
#include "CACFDictionary.h"

// System Includes
#include <IOKit/video/IOVideoTypes.h>

namespace CMIO { namespace DP { namespace IOV
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControlID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetControlID(CFDictionaryRef inControlDictionary)
	{
		UInt32 theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_ControlID), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetControlID: couldn't get the control ID");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetBaseClassID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMIOClassID ControlDictionary::GetBaseClassID(CFDictionaryRef inControlDictionary)
	{
		CMIOClassID theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_BaseClass), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetBaseClassID: couldn't get the base class ID");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetClassID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMIOClassID ControlDictionary::GetClassID(CFDictionaryRef inControlDictionary)
	{
		CMIOClassID theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_Class), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetClassID: couldn't get the class ID");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyScope()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMIOObjectPropertyScope ControlDictionary::GetPropertyScope(CFDictionaryRef inControlDictionary)
	{
		CMIOObjectPropertyScope theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_Scope), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetPropertyScope: couldn't get the control scope");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyElement()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMIOObjectPropertyElement ControlDictionary::GetPropertyElement(CFDictionaryRef inControlDictionary)
	{
		CMIOObjectPropertyElement theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_Element), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetPropertyScope: couldn't get the control element");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsReadOnly()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool ControlDictionary::IsReadOnly(CFDictionaryRef inControlDictionary)
	{
		bool theAnswer = false;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		theControlDictionary.GetBool(CFSTR(kIOVideoControlKey_IsReadOnly), theAnswer);
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetVariant()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetVariant(CFDictionaryRef inControlDictionary)
	{
		UInt32 theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_Variant), theAnswer);
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyName()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef ControlDictionary::CopyName(CFDictionaryRef inControlDictionary)
	{
		CFStringRef theAnswer = NULL;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		theControlDictionary.GetString(CFSTR(kIOVideoControlKey_Name), theAnswer);
		if (NULL != theAnswer)
		{
			CFRetain(theAnswer);
		}
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetBooleanControlValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool ControlDictionary::GetBooleanControlValue(CFDictionaryRef inControlDictionary)
	{
		bool theAnswer = false;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetBool(CFSTR(kIOVideoControlKey_Value), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetBooleanControlValue: couldn't get the value");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetSelectorControlValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetSelectorControlValue(CFDictionaryRef inControlDictionary)
	{
		UInt32 theAnswer = 0;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetUInt32(CFSTR(kIOVideoControlKey_Value), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::GetSelectorControlValue: couldn't get the value");
		return theAnswer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetSelectorControlValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFArrayRef ControlDictionary::CopySelectorControlSelectorMap(CFDictionaryRef inControlDictionary)
	{
		CFArrayRef theAnswer = NULL;
		CACFDictionary theControlDictionary(inControlDictionary, false);
		bool hasValue = theControlDictionary.GetArray(CFSTR(kIOVideoSelectorControlKey_SelectorMap), theAnswer);
		ThrowIf(!hasValue, CAException(kCMIOHardwareIllegalOperationError), "CMIO:DP:IOV::ControlDictionary::CopySelectorControlSelectorMap: couldn't get the selector map");
		if (NULL != theAnswer)
		{
			CFRetain(theAnswer);
		}
		return theAnswer;
	}
}}}