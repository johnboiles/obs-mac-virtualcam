/*
	    File: CMIO_DP_IOV_Control.cpp
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
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Self Include
#include "CMIO_DP_IOV_Control.h"

// Internal Includes
#include "CMIO_DP_PlugIn.h"
#include "CMIO_DP_Device.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"
#include "CACFString.h"
#include "CACFDictionary.h"
#include "CACFArray.h"

// System Includes
#include <IOKit/video/IOVideoTypes.h>

namespace CMIO { namespace DP { namespace IOV
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// CreateControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	DP::Control* Control::CreateControl(CMIOObjectID objectID, CMIOClassID classID, CFDictionaryRef controlDictionary, DP::PlugIn& plugIn, DP::Device& owningDevice)
	{
		DP::Control* control = NULL;
		
		switch (ControlDictionary::GetBaseClassID(controlDictionary))
		{
			case kCMIOBooleanControlClassID:
				{
					switch (ControlDictionary::GetClassID(controlDictionary))
					{
						case kCMIODirectionControlClassID:
							control = new DirectionControl(objectID, classID, controlDictionary, static_cast<PlugIn&>(plugIn), owningDevice);
							break;
						default:
							control = new BooleanControl(objectID, classID, controlDictionary, static_cast<PlugIn&>(plugIn), owningDevice);
							break;
					}
				}
				break;
			case kCMIOSelectorControlClassID:
				control = new SelectorControl(objectID, classID, controlDictionary, static_cast<PlugIn&>(plugIn), owningDevice);
				break;
							
			default:
				ThrowIf(true, CAException(-1), "CMIO::DP::CX88::Control::CreateControl: unknown control base class");
		};
		
		// Intialize the control
		control->Initialize();
		
		return control;
	}
	
	# pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl::BooleanControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	BooleanControl::BooleanControl(CMIOObjectID objectID, CMIOClassID classID, CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice) :
		DP::BooleanControl(objectID, classID, plugIn, owningDevice),
		mControlDictionary(controlDictionary),
		mDevicePropertyScope(IOV::ControlDictionary::GetPropertyScope(controlDictionary)),
		mDevicePropertyElement(IOV::ControlDictionary::GetPropertyElement(controlDictionary)),
		mCurrentValue(IOV::ControlDictionary::GetBooleanControlValue(controlDictionary)),
		mControlID(IOV::ControlDictionary::GetControlID(controlDictionary))
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl::BooleanControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	BooleanControl::~BooleanControl()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl::GetValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool BooleanControl::GetValue() const
	{
		return mCurrentValue;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl::GetNoncachedValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool BooleanControl::GetNoncachedValue() const
	{
		CACFDictionary controlDictionary(GetOwningDevice().CopyControlDictionaryByControlID(mControlID), true);
		return IOV::ControlDictionary::GetBooleanControlValue(mControlDictionary);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl::SetValue()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void BooleanControl::SetValue(bool value)
	{
		if (value != mCurrentValue)
		{
			ThrowIf(not GetOwningDevice().DeviceMasterIsOwnedBySelfOrIsFree(), CAException(kCMIODevicePermissionsError), "CMIO::DP::IOV::BooleanControl::SetValue: another process is DeviceMaster");
			
			// Attempt to change the control
			UInt32 newValue = mCurrentValue;
			GetOwningDevice().SetControlValue(mControlID, value ? 1 : 0, &newValue);

			// Update the cached value to reflect is current state and send out change notifications if needed
			if (newValue != mCurrentValue)
				UpdateValue(newValue);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl::UpdateValue()
	//	Update the control's value to reflect it current state
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void BooleanControl::UpdateValue(bool value, bool sendChangedNotifications)
	{
		CFDictionaryRef theControlDictionary = GetOwningDevice().CopyControlDictionaryByControlID(mControlID);
		CFRelease(mControlDictionary);
		mControlDictionary = theControlDictionary;

		// Update the cached value
		mCurrentValue = IOV::ControlDictionary::GetBooleanControlValue(mControlDictionary);
		
		// Send out the change notifications if needed
		if (sendChangedNotifications)
				ValueChanged();
	}

	# pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DirectionControl::DirectionControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	DirectionControl::DirectionControl(CMIOObjectID objectID, CMIOClassID classID, CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice) :
		BooleanControl(objectID, classID, controlDictionary, plugIn, owningDevice)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DirectionControl::~DirectionControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	DirectionControl::~DirectionControl()
	{
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DirectionControl::UpdateValue()
	//	Update the control's value to reflect it current state and inform the device the stream direction has changed if needed
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DirectionControl::UpdateValue(bool value, bool sendChangedNotifications)
	{
		if (mCurrentValue != value)
		{
			// Inform the device the stream direction has changed
			GetOwningDevice().StreamDirectionChanged(value ? kCMIODevicePropertyScopeInput : kCMIODevicePropertyScopeOutput);

			// Have the superclass do its work
			BooleanControl::UpdateValue(value, sendChangedNotifications);
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//	SelectorControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	SelectorControl::SelectorControl(CMIOObjectID objectID, CMIOClassID classID,CFDictionaryRef controlDictionary, PlugIn& plugIn, Device& owningDevice) :
		DP::SelectorControl(objectID, classID, plugIn, owningDevice),
		mControlDictionary(controlDictionary),
		mDevicePropertyScope(IOV::ControlDictionary::GetPropertyScope(controlDictionary)),
		mDevicePropertyElement(IOV::ControlDictionary::GetPropertyElement(controlDictionary)),
		mControlID(IOV::ControlDictionary::GetControlID(controlDictionary)),
		mSelectorMap(),
		mCurrentItemID(IOV::ControlDictionary::GetSelectorControlValue(controlDictionary))
	{
		BuildSelectorMap();
	}

	SelectorControl::~SelectorControl()
	{
		CFRelease(mControlDictionary);
		mSelectorMap.clear();
	}


	void SelectorControl::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOSelectorControlPropertyCurrentItem:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::SelectorControl::GetPropertyData: wrong data size for kCMIOSelectorControlPropertyCurrentItem");
				*static_cast<UInt32*>(data) = GetCurrentItemID();
				dataUsed = sizeof(const UInt32);
				break;
			
			case kCMIOSelectorControlPropertyAvailableItems:
				{
					UInt32 numberItemsToGet = std::min((UInt32)(dataSize / sizeof(UInt32)), GetNumberItems());
					UInt32* itemIDs = static_cast<UInt32*>(data);
					for(UInt32 index = 0; index < numberItemsToGet; ++index)
					{
						itemIDs[index] = GetItemIDForIndex(index);
					}
					dataUsed = numberItemsToGet * sizeof(UInt32);
				}
				break;
			
			case kCMIOSelectorControlPropertyItemName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::SelectorControl::GetPropertyData: wrong data size for kCMIOSelectorControlPropertyItemName");
				ThrowIf(qualifierDataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::SelectorControl::GetPropertyData: wrong qualifier size for kCMIOSelectorControlPropertyItemName");
				*static_cast<CFStringRef*>(data) = CopyItemNameByID(*static_cast<const UInt32*>(qualifierData));
				dataUsed = sizeof(CFStringRef);
				break;
			
			default:
				Control::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}
	void SelectorControl::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIOSelectorControlPropertyCurrentItem:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::SelectorControl::GetPropertyData: wrong data size for kCMIOSelectorControlPropertyCurrentItem");
				SetCurrentItemByID(*static_cast<const UInt32*>(data));
				break;
			
			default:
				Control::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	UInt32 SelectorControl::GetControlID() const
	{
		return mControlID;
	}

	void* SelectorControl::GetImplementationObject() const
	{
		return (void*)mControlDictionary;
	}

	UInt32 SelectorControl::GetNumberItems() const
	{
		return mSelectorMap.size();
	}

	UInt32 SelectorControl::GetCurrentItemID() const
	{
		//	Always get the value from the hardware and cache it in mCurrentItemID. Note that if
		//	getting the value from the hardware fails for any reason, we just return mCurrentItemID.
		UInt32 theAnswer = mCurrentItemID;
		
		// Get the control dictionary
		CFDictionaryRef theControlDictionary = GetOwningDevice().CopyControlDictionaryByControlID(mControlID);
		if (NULL != theControlDictionary)
		{
			try
			{
				theAnswer = IOV::ControlDictionary::GetSelectorControlValue(theControlDictionary);
				const_cast<SelectorControl*>(this)->mCurrentItemID = theAnswer;
				CFRelease(mControlDictionary);
				const_cast<SelectorControl*>(this)->mControlDictionary = theControlDictionary;
			}
			catch(...)
			{
			}
		}
		
		return theAnswer;
	}

	UInt32 SelectorControl::GetCurrentItemIndex() const
	{
		UInt32 theItemID = GetCurrentItemID();
		return GetItemIndexForID(theItemID);
	}

	void SelectorControl::SetCurrentItemByID(UInt32 itemID)
	{
		// Set the value in hardware. Note that mCurrentItemID should be updated only if setting the hardware value is synchronous.
		// Otherwise, mCurrentItemID will be updated when the hardware notifies us that the value of the control changed.
		
		if (itemID != mCurrentItemID)
		{
			UInt32 newItemID = mCurrentItemID;
			GetOwningDevice().SetControlValue(mControlID, itemID, &newItemID);

			// Update the cached value to reflect is current state and send out change notifications
			if (newItemID != mCurrentItemID)
				UpdateCurrentItem();
		}
	}

	void SelectorControl::UpdateCurrentItem(bool sendChangedNotifications)
	{
		// Get the current ID  mCurrentItemID (forces a sync with the hardware)
		GetCurrentItemID();
		
		// And fire off a value changed notification
		if (sendChangedNotifications)
			CurrentItemChanged();
	}


	void SelectorControl::SetCurrentItemByIndex(UInt32 itemIndex)
	{
		UInt32 theItemID = GetItemIDForIndex(itemIndex);
		SetCurrentItemByID(theItemID);
	}

	UInt32 SelectorControl::GetItemIDForIndex(UInt32 itemIndex) const
	{
		ThrowIf(itemIndex >= mSelectorMap.size(), CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::GetItemIDForIndex: index out of range");
		SelectorMap::const_iterator theIterator = mSelectorMap.begin();
		std::advance(theIterator, itemIndex);
		return theIterator->first;
	}

	UInt32 SelectorControl::GetItemIndexForID(UInt32 itemID) const
	{
		UInt32 theIndex = 0;
		bool wasFound = false;
		SelectorMap::const_iterator theIterator = mSelectorMap.begin();
		while(!wasFound && (theIterator != mSelectorMap.end()))
		{
			if (theIterator->first == itemID)
			{
				wasFound = true;
			}
			else
			{
				++theIndex;
				std::advance(theIterator, 1);
			}
		}
		ThrowIf(!wasFound, CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::GetItemIndexForID: ID not in selector map");
		return theIndex;
	}

	CFStringRef SelectorControl::CopyItemNameByID(UInt32 itemID) const
	{
		SelectorMap::const_iterator theIterator = mSelectorMap.find(itemID);
		ThrowIf(theIterator == mSelectorMap.end(), CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::CopyItemNameByID: ID not in selector map");
		
		return (CFStringRef)CFRetain(theIterator->second.mItemName);
	}

	CFStringRef SelectorControl::CopyItemNameByIndex(UInt32 itemIndex) const
	{
		CFStringRef theAnswer = NULL;
		
		if (itemIndex < mSelectorMap.size())
		{
			SelectorMap::const_iterator theIterator = mSelectorMap.begin();
			std::advance(theIterator, itemIndex);
			ThrowIf(theIterator == mSelectorMap.end(), CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::CopyItemNameByIndex: index out of range");
			
			theAnswer = (CFStringRef)CFRetain(theIterator->second.mItemName);
		}
			
		return theAnswer;
	}

	CFStringRef SelectorControl::CopyItemNameByIDWithoutLocalizing(UInt32 itemID) const
	{
		return CopyItemNameByID(itemID);
	}

	CFStringRef SelectorControl::CopyItemNameByIndexWithoutLocalizing(UInt32 itemIndex) const
	{
		return CopyItemNameByIndex(itemIndex);
	}

	UInt32 SelectorControl::GetItemKindByID(UInt32 itemID) const
	{
		SelectorMap::const_iterator theIterator = mSelectorMap.find(itemID);
		ThrowIf(theIterator == mSelectorMap.end(), CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::GetItemKindByID: ID not in selector map");
		
		return theIterator->second.mItemKind;
	}

	UInt32 SelectorControl::GetItemKindByIndex(UInt32 itemIndex) const
	{
		UInt32 theAnswer = 0;
		
		if (itemIndex < mSelectorMap.size())
		{
			SelectorMap::const_iterator theIterator = mSelectorMap.begin();
			std::advance(theIterator, itemIndex);
			ThrowIf(theIterator == mSelectorMap.end(), CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::GetItemKindByIndex: index out of range");
			theAnswer = theIterator->second.mItemKind;
		}
		
		return theAnswer;
	}

	void SelectorControl::BuildSelectorMap()
	{
		// Clear the current items
		mSelectorMap.clear();
		
		// Get the selector map from the control dictionary
		CFArrayRef theCFSelectorMap = IOV::Control::CopySelectorControlSelectorMap(mControlDictionary);
		CACFArray theSelectorMap(theCFSelectorMap, true);
		if (theSelectorMap.IsValid())
		{
			// Make sure there is something in the map
			UInt32 theNumberSelectors = theSelectorMap.GetNumberItems();
			ThrowIf(0 == theNumberSelectors, CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::BuildSelectorMap: selector map has no items");
			
			// Iterate through the items in the map
			for(UInt32 theSelectorIndex = 0; theSelectorIndex < theNumberSelectors; ++theSelectorIndex)
			{
				// Get the selector dictionary
				CFDictionaryRef theCFSelector = NULL;
				theSelectorMap.GetDictionary(theSelectorIndex, theCFSelector);
				ThrowIfNULL(theCFSelector, CAException(kCMIOHardwareIllegalOperationError), "SelectorControl::BuildSelectorMap: no selector at the given index");
				CACFDictionary theSelector(theCFSelector, false);
				
				// Get the selector values
				UInt32 theValue = 0;
				CFStringRef theName = NULL;
				UInt32 theKind = 0;
				theSelector.GetUInt32(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Value), theValue);
				theSelector.GetString(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Name), theName);
				theSelector.GetUInt32(CFSTR(kIOVideoSelectorControlSelectorMapItemKey_Kind), theKind);
				if (NULL != theName)
				{
					CFRetain(theName);
				}
				
				// Stick them into the selector map
				mSelectorMap.insert(SelectorMap::value_type(theValue, SelectorItem(theName, theKind)));
			}
		}
	}

	void SelectorControl::CacheCurrentItemID()
	{
		//	Set mCurrentItemID to the value of the hardware.
		
		// Get the control dictionary
		CFDictionaryRef theControlDictionary = GetOwningDevice().CopyControlDictionaryByControlID(mControlID);
		if (NULL != theControlDictionary)
		{
			try
			{
				mCurrentItemID = IOV::ControlDictionary::GetSelectorControlValue(theControlDictionary);
				CFRelease(mControlDictionary);
				mControlDictionary = theControlDictionary;
			}
			catch(...)
			{
			}
		}
	}
}}}