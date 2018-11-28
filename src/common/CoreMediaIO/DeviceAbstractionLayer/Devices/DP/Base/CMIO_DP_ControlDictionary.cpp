/*
	    File: CMIO_DP_ControlDictionary.cpp
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
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DP_ControlDictionary.h"

// CA Public Utilility Includes
#include "CACFArray.h"
#include "CACFDictionary.h"
#include "CACFString.h"

// System Includes
#include <IOKit/IOTypes.h>
#include <IOKit/video/IOVideoTypes.h>

namespace
{
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetBoolean
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool GetBoolean(const CACFDictionary& dictionary, const CFStringRef key)
	{
		bool answer = false;
		dictionary.GetBool(key, answer);
		return answer;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetUInt32
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 GetUInt32(const CACFDictionary& dictionary, const CFStringRef key)
	{
		UInt32 answer = 0;
		dictionary.GetUInt32(key, answer);
		return answer;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetUInt64
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt64 GetUInt64(const CACFDictionary& dictionary, const CFStringRef key)
	{
		UInt64 answer = 0;
		dictionary.GetUInt64(key, answer);
		return answer;
	}
}

namespace CMIO { namespace DP
{
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Create()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFMutableDictionaryRef ControlDictionary::Create(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, const CACFString& name, bool isReadOnly, UInt32 variant)
	{
		CACFDictionary answer = CACFDictionary(false);

		if (answer.IsValid())
		{
			// Fill out the info, starting with the control ID
			answer.AddUInt32(CFSTR(kIOVideoControlKey_ControlID), controlID);
			
			// Do the base class
			answer.AddUInt32(CFSTR(kIOVideoControlKey_BaseClass), baseClass);
			
			// Do the class
			answer.AddUInt32(CFSTR(kIOVideoControlKey_Class), derivedClass);
			
			// Do the scope
			answer.AddUInt32(CFSTR(kIOVideoControlKey_Scope), scope);
			
			// Do the element
			answer.AddUInt32(CFSTR(kIOVideoControlKey_Element), element);
			
			// Do the is read only value
			answer.AddBool(CFSTR(kIOVideoControlKey_IsReadOnly), isReadOnly);
			
			// Do the variant
			answer.AddUInt32(CFSTR(kIOVideoControlKey_Variant), variant);
			
			// Do the name
			if (name.IsValid())
			{
				answer.AddString(CFSTR(kIOVideoControlKey_Name), name.GetCFString());
			}
		}

		return (answer.GetCFMutableDictionary());
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateBooleanControl()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFMutableDictionaryRef ControlDictionary::CreateBooleanControl(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, bool value, const CACFString& name, bool isReadOnly, UInt32 variant)
	{
		// Do the general fields
		CACFDictionary answer(Create(controlID, baseClass, derivedClass, scope, element, name, isReadOnly, variant), false);
		if (answer.IsValid())
		{
			// Do the value
			answer.AddBool(CFSTR(kIOVideoControlKey_Value), value);
		}

		return (answer.GetCFMutableDictionary());
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateSelectorControl()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFMutableDictionaryRef ControlDictionary::CreateSelectorControl(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, UInt32 value, CACFArray& selectorMap, const CACFString& name, bool isReadOnly, UInt32 variant)
	{
		// Do the general fields
		CACFDictionary answer(Create(controlID, baseClass, derivedClass, scope, element, name, isReadOnly, variant), false);
		if (answer.IsValid())
		{
			// Do the value
			answer.AddUInt32(CFSTR(kIOVideoControlKey_Value), value);
			
			// Do the range map
			if (selectorMap.IsValid())
			{
				answer.AddArray(CFSTR(kIOVideoSelectorControlKey_SelectorMap), selectorMap.GetCFArray());
			}
		}

		return (answer.GetCFMutableDictionary());
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControlByID()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFMutableDictionaryRef ControlDictionary::GetControlByID(const CACFArray& controlList, UInt32 controlID)
	{
		// Note that this routine require that the returned dictionary not be released
		CACFDictionary	answer;
		CFMutableDictionaryRef returnDict = NULL;
		CFDictionaryRef cfanswer;
		UInt32 count, i;
		
		if (controlList.IsValid())
		{
			count = controlList.GetNumberItems();
			for (i = 0 ; i< count; ++i)
			{
				if (controlList.GetDictionary(i, cfanswer))
				{
					CACFDictionary interimAnswer(cfanswer, false);
					if (interimAnswer.IsValid())
					{
						if (controlID == ControlDictionary::GetControlID(interimAnswer))
						{
							returnDict = interimAnswer.GetCFMutableDictionary();
							break;
						}
					}
				}
			}
		}
		
		return returnDict;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetControlID()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetControlID(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_ControlID));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetControlID()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetControlID(CACFDictionary& dictionary, UInt32 controlID)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_ControlID), controlID);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetBaseClass()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetBaseClass(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_BaseClass));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetBaseClass()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetBaseClass(CACFDictionary& dictionary, UInt32 baseClass)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_BaseClass), baseClass);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetClass()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetClass(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_Class));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetClass()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetClass(CACFDictionary& dictionary, UInt32 derivedClass)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_Class), derivedClass);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetScope()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetScope(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_Scope));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetScope()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetScope(CACFDictionary& dictionary, UInt32 scope)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_Scope), scope);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetElement()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetElement(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_Element));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetElement()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetElement(CACFDictionary& dictionary, UInt32 element)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_Element), element);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsReadOnly()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool ControlDictionary::IsReadOnly(const CACFDictionary& dictionary)
	{
		return GetBoolean(dictionary, CFSTR(kIOVideoControlKey_IsReadOnly));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetIsReadOnly()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetIsReadOnly(CACFDictionary& dictionary, bool isReadOnly)
	{
		dictionary.AddBool(CFSTR(kIOVideoControlKey_IsReadOnly), isReadOnly);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetVariant()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetVariant(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_Variant));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetVariant()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetVariant(CACFDictionary& dictionary, UInt32 variant)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_Variant), variant);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopyName()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef ControlDictionary::CopyName(const CACFDictionary& dictionary)
	{
		// Get the value from the dictionary and make sure it's a string
		CFStringRef answer;
		dictionary.GetString(CFSTR(kIOVideoControlKey_Name), answer);
		
		return answer;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetName()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetName(CACFDictionary& dictionary, const CACFString& name)
	{
		dictionary.AddString(CFSTR(kIOVideoControlKey_Name), name.GetCFString());
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetBooleanControlValue()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool ControlDictionary::GetBooleanControlValue(const CACFDictionary& dictionary)
	{
		return GetBoolean(dictionary, CFSTR(kIOVideoControlKey_Value));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetBooleanControlValue()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetBooleanControlValue(CACFDictionary& dictionary, bool value)
	{
		dictionary.AddBool(CFSTR(kIOVideoControlKey_Value), value);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetSelectorControlValue()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 ControlDictionary::GetSelectorControlValue(const CACFDictionary& dictionary)
	{
		return GetUInt32(dictionary, CFSTR(kIOVideoControlKey_Value));
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetSelectorControlValue()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetSelectorControlValue(CACFDictionary& dictionary, UInt32 value)
	{
		dictionary.AddUInt32(CFSTR(kIOVideoControlKey_Value), value);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CopySelectorControlSelectorMap()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFArrayRef ControlDictionary::CopySelectorControlSelectorMap(const CACFDictionary& dictionary)
	{
		CFArrayRef answer;
		
		// Get the value from the dictionary and make sure it's an array
		dictionary.GetArray(CFSTR(kIOVideoSelectorControlKey_SelectorMap), answer);
		
		return answer;
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetSelectorControlSelectorMap()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ControlDictionary::SetSelectorControlSelectorMap(CACFDictionary& dictionary, CACFArray& selectorMap)
	{
		dictionary.AddArray(CFSTR(kIOVideoSelectorControlKey_SelectorMap), selectorMap.GetCFArray());
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateSelectorControlSelectorMapItem()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFDictionaryRef ControlDictionary::CreateSelectorControlSelectorMapItem(UInt32 value, const CACFString& name)
	{
		CACFDictionary answer = CACFDictionary(false);
		if (answer.IsValid())
		{
			// Do the value
			answer.AddUInt32(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Value), value);
			
			// Do the starting name
			answer.AddString(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Name), name.GetCFString());
		}

		return (answer.GetCFDictionary());
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateSelectorControlSelectorMapItem()
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFDictionaryRef ControlDictionary::CreateSelectorControlSelectorMapItem(UInt32 value, const CACFString& name, UInt32 kind)
	{
		CACFDictionary answer = CACFDictionary(false);
		if (answer.IsValid())
		{
			// Do the value
			answer.AddUInt32(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Value), value);
			
			// Do the starting name
			answer.AddString(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Name), name.GetCFString());
			
			// Do the kind
			answer.AddUInt32(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Kind), kind);
		}

		return (answer.GetCFDictionary());
	}
}}
