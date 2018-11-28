/*
	    File: CMIO_DP_IOV_Control.h
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

#if !defined(__CMIO_DP_IOV_Control_h__)
#define __CMIO_DP_IOV_Control_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class(es) Includes
#include "CMIO_DP_IOV_ControlDictionary.h"
#include "CMIO_DP_Control.h"

// Standard Library Includes
#include <map>

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in the CMIO::DP namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Device;
	class Plugin;
}}

namespace CMIO { namespace DP { namespace IOV
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Control
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Control : public DP::Control, public ControlDictionary
	{
	public:
		static DP::Control*					CreateControl(CMIOObjectID objectID, CMIOClassID classID, CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice);
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class BooleanControl : public DP::BooleanControl 
	{
	//	Construction/Destruction
	public:
											BooleanControl(CMIOObjectID objectID, CMIOClassID classID, CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice);
		virtual								~BooleanControl();
	
		virtual UInt32						GetControlID() const { return mControlID; }
		virtual	Device&						GetOwningDevice() const { return reinterpret_cast<Device&>(mOwningDevice); }
		virtual CMIOObjectPropertyScope		GetPropertyScope() const { return mDevicePropertyScope; }
		virtual CMIOObjectPropertyElement	GetPropertyElement() const { return mDevicePropertyElement; }
		virtual bool						GetValue() const;
		virtual bool						GetNoncachedValue() const;
		virtual void						SetValue(bool value);
		virtual void						UpdateValue(bool value, bool sendChangedNotifications = true);

	protected:
		CFDictionaryRef						mControlDictionary;
		CMIOObjectPropertyScope				mDevicePropertyScope;
		CMIOObjectPropertyElement			mDevicePropertyElement;
		bool								mCurrentValue;
		UInt32								mControlID;
	};

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DirectionControl
	//	A Boolean control sublcass that controls whether a device's streams are input or output
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class DirectionControl : public BooleanControl 
	{
	//	Construction/Destruction
	public:
											DirectionControl(CMIOObjectID objectID, CMIOClassID classID, CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice);
		virtual								~DirectionControl();
	
		virtual void						UpdateValue(bool value, bool sendChangedNotifications = true);
	};

	# pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SelectorControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class SelectorControl : public DP::SelectorControl 
	{
	// Construction/Destruction
	public:
											SelectorControl(CMIOObjectID objectID, CMIOClassID classID,CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice);
		virtual								~SelectorControl();

	// Attributes
	public:
		virtual UInt32						GetControlID() const;
		virtual	Device&						GetOwningDevice() const { return reinterpret_cast<Device&>(mOwningDevice); }
		virtual CMIOObjectPropertyScope		GetPropertyScope() const { return mDevicePropertyScope; }
		virtual CMIOObjectPropertyElement	GetPropertyElement() const { return mDevicePropertyElement; }
		virtual void*						GetImplementationObject() const;
		virtual UInt32						GetVariant() const { return IOV::ControlDictionary::GetVariant(mControlDictionary); }
		virtual bool						IsReadOnly() const { return IOV::ControlDictionary::IsReadOnly(mControlDictionary); }
		virtual void						GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void						SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);


	// Property Access
	public:
		virtual CFStringRef					CopyName() const { return IOV::ControlDictionary::CopyName(mControlDictionary); }
		virtual UInt32						GetNumberItems() const;

		virtual UInt32						GetCurrentItemID() const;
		virtual UInt32						GetCurrentItemIndex() const;
		
		virtual void						SetCurrentItemByID(UInt32 itemID);
		virtual void						SetCurrentItemByIndex(UInt32 itemIndex);
		virtual void						UpdateCurrentItem(bool sendChangedNotifications = true);
		
		virtual UInt32						GetItemIDForIndex(UInt32 itemIndex) const;
		virtual UInt32						GetItemIndexForID(UInt32 itemID) const;
		
		virtual CFStringRef					CopyItemNameByID(UInt32 itemID) const;
		virtual CFStringRef					CopyItemNameByIndex(UInt32 itemIndex) const;
		
		virtual CFStringRef					CopyItemNameByIDWithoutLocalizing(UInt32 itemID) const;
		virtual CFStringRef					CopyItemNameByIndexWithoutLocalizing(UInt32 itemIndex) const;
		
		virtual UInt32						GetItemKindByID(UInt32 itemID) const ;
		virtual UInt32						GetItemKindByIndex(UInt32 itemIndex) const ;

	private:
		CFDictionaryRef						mControlDictionary;
		CMIOObjectPropertyScope				mDevicePropertyScope;
		CMIOObjectPropertyElement			mDevicePropertyElement;
		UInt32								mControlID;

	private:
		void								BuildSelectorMap();
		void								CacheCurrentItemID();
	
		struct SelectorItem
		{
			CFStringRef	mItemName;
			UInt32		mItemKind;
			
			SelectorItem() : mItemName(NULL), mItemKind(0) {}
			SelectorItem(CFStringRef itemname, UInt32 itemkind) : mItemName(itemname), mItemKind(itemkind) {}
			SelectorItem(const SelectorItem& item) : mItemName(item.mItemName), mItemKind(item.mItemKind) { if (NULL != mItemName) { CFRetain(mItemName); } }
			SelectorItem&	operator=(const SelectorItem& item) { if (NULL != mItemName) { CFRelease(mItemName); } mItemName = item.mItemName; if (NULL != mItemName) { CFRetain(mItemName); } mItemKind = item.mItemKind; return *this; }
			~SelectorItem() { if (NULL != mItemName) { CFRelease(mItemName); } }
		};
		typedef std::map<UInt32, SelectorItem>	SelectorMap;
		
		SelectorMap							mSelectorMap;
		UInt32								mCurrentItemID;
	};

}}}
#endif
