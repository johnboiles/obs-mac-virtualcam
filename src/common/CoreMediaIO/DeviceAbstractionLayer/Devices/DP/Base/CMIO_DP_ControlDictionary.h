/*
	    File: CMIO_DP_ControlDictionary.h
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

#if !defined(__CMIO_DP_ControlDictionary_h__)
#define __CMIO_DP_ControlDictionary_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Object.h"

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Types in global namespace
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
class CACFArray;
class CACFDictionary;
class CACFString;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ControlDictionary
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
namespace CMIO { namespace DP
{
	class ControlDictionary : public Object
	{
	// Construction/Destruction
	public:
		static CFMutableDictionaryRef	Create(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, const CACFString& name, bool isReadOnly = false, UInt32 variant = 0);
		static CFMutableDictionaryRef	CreateLevelControl(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, UInt32 value, CACFArray& rangeMap, const CACFString& name , UInt32 dbtoIntegerTransferFunction = 0, bool isReadOnly = false, UInt32 variant = 0);
		static CFMutableDictionaryRef	CreateBooleanControl(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, bool value, const CACFString& name , bool isReadOnly = false, UInt32 variant = 0);
		static CFMutableDictionaryRef	CreateSelectorControl(UInt32 controlID, UInt32 baseClass, UInt32 derivedClass, UInt32 scope, UInt32 element, UInt32 value, CACFArray& selectorMap, const CACFString& name, bool isReadOnly = false, UInt32 variant = 0);

	// General Attributes
	public:
		static CFMutableDictionaryRef	GetControlByID(const CACFArray& controlList, UInt32 controlID);

		static UInt32					GetControlID(const CACFDictionary& dictionary);
		static void						SetControlID(CACFDictionary& dictionary, UInt32 controlID);

		static UInt32					GetBaseClass(const CACFDictionary& dictionary);
		static void						SetBaseClass(CACFDictionary& dictionary, UInt32 baseClass);

		static UInt32					GetClass(const CACFDictionary& dictionary);
		static void						SetClass(CACFDictionary& dictionary, UInt32 derivedClass);

		static UInt32					GetScope(const CACFDictionary& dictionary);
		static void						SetScope(CACFDictionary& dictionary, UInt32 scope);

		static UInt32					GetElement(const CACFDictionary& dictionary);
		static void						SetElement(CACFDictionary& dictionary, UInt32 element);

		static bool						IsReadOnly(const CACFDictionary& dictionary);
		static void						SetIsReadOnly(CACFDictionary& dictionary, bool isReadOnly);

		static UInt32					GetVariant(const CACFDictionary& dictionary);
		static void						SetVariant(CACFDictionary& dictionary, UInt32 variant);

		static CFStringRef				CopyName(const CACFDictionary& dictionary);
		static void						SetName(CACFDictionary& dictionary, const CACFString& name);

	// Boolean Control Attributes
	public:
		static bool						GetBooleanControlValue(const CACFDictionary& dictionary);
		static void						SetBooleanControlValue(CACFDictionary& dictionary, bool value);

	// Selector Control Attributes
	public:
		static UInt32					GetSelectorControlValue(const CACFDictionary& dictionary);
		static void						SetSelectorControlValue(CACFDictionary& dictionary, UInt32 value);

		static CFArrayRef				CopySelectorControlSelectorMap(const CACFDictionary& dictionary);
		static void						SetSelectorControlSelectorMap(CACFDictionary& dictionary, CACFArray& selectorMap);

	// Selector Control Selector Map Item Support
	public:
		static CFDictionaryRef			CreateSelectorControlSelectorMapItem(UInt32 value, const CACFString& name);
		static CFDictionaryRef			CreateSelectorControlSelectorMapItem(UInt32 value, const CACFString& name, UInt32 kind);
	};
}}

#endif
