/*
	    File: CMIO_DP_Control.h
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

#if !defined(__CMIO_DP_Control_h__)
#define __CMIO_DP_Control_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Object.h"

// Internal Includes
#include "CMIO_DP_Property_Base.h"

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Device;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Control
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Control : public Object
	{
	// Construction/Destruction
	public:
											Control(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice);
		virtual								~Control();

	// Attributes
	public:
		virtual CMIOClassID					GetBaseClassID() const;
		virtual CAMutex*					GetObjectStateMutex();
		virtual void						Show() const;
		bool								GetMark() const { return mMark; }
		void								SetMark(bool mark) { mMark = mark; }
		virtual CFStringRef					CopyName() const;
		virtual CFStringRef					CopyManufacturerName() const;
		virtual CMIOObjectPropertyScope		GetPropertyScope() const = 0;
		virtual CMIOObjectPropertyElement	GetPropertyElement() const = 0;
		virtual void*						GetImplementationObject() const;
		virtual UInt32						GetVariant() const;
		virtual bool						IsReadOnly() const;
		
	// Property Access
	public:
		virtual bool						HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool						IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32						GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void						GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void						SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);

	// Implementation
	protected:
		Device&								mOwningDevice;
		bool								mMark;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class BooleanControl : public Control
	{
	// Construction/Destruction
	public:
							BooleanControl(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice);
		virtual				~BooleanControl();

	// Attributes
	public:
		virtual CMIOClassID	GetBaseClassID() const;
		virtual bool		GetValue() const = 0;
		virtual void		SetValue(bool value) = 0;

	// Property Access
	public:
		virtual bool		HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool		IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32		GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void		GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void		SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);
 
 		virtual void		ValueChanged() const;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SelectorControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class SelectorControl : public Control
	{
	// Construction/Destruction
	public:
							SelectorControl(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice);
		virtual				~SelectorControl();

	// Attributes
	public:
		virtual CMIOClassID	GetBaseClassID() const;
		virtual UInt32		GetNumberItems() const = 0;

		virtual UInt32		GetCurrentItemID() const = 0;
		virtual UInt32		GetCurrentItemIndex() const = 0;
		
		virtual void		SetCurrentItemByID(UInt32 itemID) = 0;
		virtual void		SetCurrentItemByIndex(UInt32 itemIndex) = 0;
		
		virtual UInt32		GetItemIDForIndex(UInt32 itemIndex) const = 0;
		virtual UInt32		GetItemIndexForID(UInt32 itemID) const = 0;
		
		virtual CFStringRef	CopyItemNameByID(UInt32 itemID) const = 0;
		virtual CFStringRef	CopyItemNameByIndex(UInt32 itemIndex) const = 0;
		
		virtual CFStringRef	CopyItemNameByIDWithoutLocalizing(UInt32 itemID) const = 0;
		virtual CFStringRef	CopyItemNameByIndexWithoutLocalizing(UInt32 itemIndex) const = 0;
		
		virtual UInt32		GetItemKindByID(UInt32 itemID) const = 0;
		virtual UInt32		GetItemKindByIndex(UInt32 itemIndex) const = 0;

		virtual void		CurrentItemChanged() const;
		virtual void		AvailableItemsChanged() const;
		virtual void		ItemNameChanged() const;

	// Property Access
	public:
		virtual bool		HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool		IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32		GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void		GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void		SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class FeatureControl : public Control
	{
	// Construction/Destruction
	public:
							FeatureControl(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice);
		virtual				~FeatureControl();

	// Attributes
	public:
		virtual CMIOClassID	GetBaseClassID() const;
		virtual CFStringRef	CopyName() const;

		virtual bool		OnOffSettable() const = 0;
		virtual UInt32		GetOnOff() const = 0;
		virtual void		SetOnOff(UInt32 onOff) = 0;
		
		virtual bool		AutomaticManualSettable() const = 0;
		virtual UInt32		GetAutomaticManual() const = 0;
		virtual void		SetAutomaticManual(UInt32 automaticManual) = 0;
		
		virtual bool		AbsoluteNativeSettable() const = 0;
		virtual UInt32		GetAbsoluteNative() const = 0;
		virtual void		SetAbsoluteNative(UInt32 nativeAbsolute) = 0;
		
		virtual bool		HasTune() const = 0;
		virtual UInt32		GetTune() const = 0;
		virtual void		SetTune(UInt32 tune) = 0;

		virtual Float32		GetNativeValue() const = 0;
		virtual void		SetNativeValue(Float32 nativeValue) = 0;
		virtual Float32		GetMinimumNativeValue() const = 0;
		virtual Float32		GetMaximumNativeValue() const = 0;

		virtual Float32		GetAbsoluteValue() const = 0;
		virtual void		SetAbsoluteValue(Float32 absoluteValue) = 0;
		virtual Float32		GetMinimumAbsoluteValue() const = 0;
		virtual Float32		GetMaximumAbsoluteValue() const = 0;

		virtual bool		HasNativeToAbsolute() const = 0;
		virtual Float32		ConverNativeValueToAbsoluteValue(Float32 nativeValue) const = 0;
		
		virtual bool		HasAbsoluteToNative() const = 0;
		virtual Float32		ConverAbsoluteValueToNativeValue(Float32 absoluteValue) const = 0;
		
		virtual CFStringRef	CopyAbsoluteUnitName() const;

	// Property Access
	public:
		virtual bool		HasProperty(const CMIOObjectPropertyAddress& address) const;
		virtual bool		IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32		GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void		GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void		SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);
	};
}}
#endif
