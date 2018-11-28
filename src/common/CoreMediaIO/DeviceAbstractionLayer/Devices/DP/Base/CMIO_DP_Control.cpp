/*
	    File: CMIO_DP_Control.cpp
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
#include "CMIO_DP_Control.h"

// Internal Includes
#include "CMIO_DP_Device.h"
#include "CMIO_DP_Stream.h"

// CA Public Utility Includes
#include "CACFString.h"
#include "CMIODebugMacros.h"
#include "CAException.h"

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Control
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Control::Control(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice) :
		Object(objectID, classID, plugIn),
		mOwningDevice(owningDevice),
		mMark(false)
	{
	}

	Control::~Control()
	{
	}
	
	CMIOClassID Control::GetBaseClassID() const
	{
		return kCMIOControlClassID;
	}

	CAMutex* Control::GetObjectStateMutex()
	{
		return mOwningDevice.GetObjectStateMutex();
	}

	void Control::Show() const
	{
		// Make a string for the class ID
		char classID[] = CA4CCToCString(mClassID);
		
		// Get the object's name
		PropertyAddress address(kCMIOObjectPropertyName, kCMIOObjectPropertyScopeGlobal, kCMIOObjectPropertyElementMaster);
		CFStringRef cfname = NULL;
		UInt32 dataUsed = 0;
		try
		{
			GetPropertyData(address, 0, NULL, sizeof(CFStringRef), dataUsed, &cfname);
		}
		catch(...)
		{
			cfname = NULL;
		}
		
		// Make a C string out of the name
		char name[256];
		name[0] = 0;
		if (cfname != NULL)
		{
			CFIndex length = 0;
			CFRange range = { 0, CFStringGetLength(cfname) };
			CFStringGetBytes(cfname, range, kCFStringEncodingUTF8, 0, false, (UInt8*)name, 255, &length);
			name[length] = 0;
			CFRelease(cfname);
		}
		
		// Get a string for the scope
		const char* scope = NULL;
		switch (GetPropertyScope())
		{
			case kCMIODevicePropertyScopeInput:
				scope = "Input";
				break;
			
			case kCMIODevicePropertyScopeOutput:
				scope = "Output";
				break;
			
			case kCMIODevicePropertyScopePlayThrough:
				scope = "Play Through";
				break;
			
			case kCMIOObjectPropertyScopeGlobal:
			default:
				scope = "Global";
				break;
		};
		
		// Print the information to the standard output
		printf("CMIOObjectID:\t\t0x%lX\n\tCMIOClassID:\t'%s'\n\tName:\t\t\t%s\n\tScope:\t\t\t%s\n\tChannel:\t\t%lu\n", (long unsigned int)mObjectID, classID, name, scope, (long unsigned int)GetPropertyElement());
	}

	CFStringRef Control::CopyName() const
	{
		return NULL;
	}

	CFStringRef Control::CopyManufacturerName() const
	{
		return mOwningDevice.CopyDeviceManufacturerName();
	}

	void* Control::GetImplementationObject() const
	{
		return NULL;
	}

	UInt32 Control::GetVariant() const
	{
		return mClassID;
	}

	bool Control::IsReadOnly() const
	{
		return false;
	}

	bool Control::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		// Initialize some commonly used variables
		CFStringRef cfstring = NULL;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				cfstring = CopyName();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
				
			case kCMIOObjectPropertyManufacturer:
				cfstring = CopyManufacturerName();
				if (cfstring != NULL)
				{
					answer = true;
					CFRelease(cfstring);
				}
				break;
			
			case kCMIOControlPropertyScope:
				answer = true;
				break;
				
			case kCMIOControlPropertyElement:
				answer = true;
				break;
				
			case kCMIOControlPropertyVariant:
				answer = true;
				break;
				
			default:
				answer = Object::HasProperty(address);
				break;
		};
		
		return answer;
	}

	bool Control::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = false;
				break;
				
			case kCMIOObjectPropertyManufacturer:
				answer = false;
				break;
				
			case kCMIOControlPropertyScope:
				answer = false;
				break;
				
			case kCMIOControlPropertyElement:
				answer = false;
				break;
				
			case kCMIOControlPropertyVariant:
				answer = false;
				break;
				
			default:
				answer = Object::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	UInt32 Control::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyManufacturer:
				answer = sizeof(CFStringRef);
				break;
				
			case kCMIOControlPropertyScope:
				answer = sizeof(CMIOObjectPropertyScope);
				break;
				
			case kCMIOControlPropertyElement:
				answer = sizeof(CMIOObjectPropertyElement);
				break;
				
			case kCMIOControlPropertyVariant:
				answer = sizeof(UInt32);
				break;
				
			default:
				answer = Object::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	void Control::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyName:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Control::GetPropertyData: wrong data size for kCMIOObjectPropertyName");
				*static_cast<CFStringRef*>(data) = CopyName();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOObjectPropertyManufacturer:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Control::GetPropertyData: wrong data size for kCMIOObjectPropertyManufacturer");
				*static_cast<CFStringRef*>(data) = CopyManufacturerName();
				dataUsed = sizeof(CFStringRef);
				break;
				
			case kCMIOControlPropertyScope:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Control::GetPropertyData: wrong data size for kCMIOControlPropertyScope");
				*static_cast<CMIOObjectPropertyScope*>(data) = GetPropertyScope();
				dataUsed = sizeof(CMIOObjectPropertyScope);
				break;
				
			case kCMIOControlPropertyElement:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Control::GetPropertyData: wrong data size for kCMIOControlPropertyElement");
				*static_cast<CMIOObjectPropertyElement*>(data) = GetPropertyElement();
				dataUsed = sizeof(CMIOObjectPropertyElement);
				break;
				
			case kCMIOControlPropertyVariant:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Control::GetPropertyData: wrong data size for kCMIOControlPropertyVariant");
				*static_cast<UInt32*>(data) = GetVariant();
				dataUsed = sizeof(UInt32);
				break;
				
			default:
				Object::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	void Control::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			default:
				Object::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BooleanControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	BooleanControl::BooleanControl(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice) :
		Control(objectID, classID, plugIn, owningDevice)
	{
	}

	BooleanControl::~BooleanControl()
	{
	}

	CMIOClassID	BooleanControl::GetBaseClassID() const
	{
		return kCMIOBooleanControlClassID;
	}

	bool	BooleanControl::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOBooleanControlPropertyValue:
				answer = true;
				break;
				
			default:
				answer = Control::HasProperty(address);
				break;
		};
		
		return answer;
	}

	bool	BooleanControl::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOBooleanControlPropertyValue:
				answer = !IsReadOnly();
				break;
			
			default:
				answer = Control::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	UInt32	BooleanControl::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		// Initialize the return value
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOBooleanControlPropertyValue:
				answer = sizeof(UInt32);
				break;
			
			default:
				answer = Control::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	void BooleanControl::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOBooleanControlPropertyValue:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::BooleanControl::GetPropertyData: wrong data size for kCMIOBooleanControlPropertyValue");
				*static_cast<UInt32*>(data) = GetValue() ? 1 : 0;
				dataUsed = sizeof(UInt32);
				break;
			
			default:
				Control::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	void BooleanControl::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIOBooleanControlPropertyValue:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::BooleanControl::GetPropertyData: wrong data size for kCMIOBooleanControlPropertyValue");
				SetValue(*static_cast<const UInt32*>(data) != 0);
				break;
		
			default:
				Control::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	void BooleanControl::ValueChanged() const
	{
		PropertyAddress address(kCMIOBooleanControlPropertyValue);
		PropertiesChanged(1, &address);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SelectorControl
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	SelectorControl::SelectorControl(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice) :
		Control(objectID, classID, plugIn, owningDevice)
	{
	}

	SelectorControl::~SelectorControl()
	{
	}

	CMIOClassID	SelectorControl::GetBaseClassID() const
	{
		return kCMIOSelectorControlClassID;
	}

	bool	SelectorControl::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOSelectorControlPropertyCurrentItem:
				answer = true;
				break;
				
			case kCMIOSelectorControlPropertyAvailableItems:
				answer = true;
				break;
				
			case kCMIOSelectorControlPropertyItemName:
				answer = true;
				break;
			
			default:
				answer = Control::HasProperty(address);
				break;
		};
		
		return answer;
	}

	bool	SelectorControl::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOSelectorControlPropertyCurrentItem:
				answer = !IsReadOnly();
				break;
				
			case kCMIOSelectorControlPropertyAvailableItems:
				answer = false;
				break;
				
			case kCMIOSelectorControlPropertyItemName:
				answer = false;
				break;
			
			default:
				answer = Control::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	UInt32	SelectorControl::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		// Initialize the return value
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOSelectorControlPropertyCurrentItem:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOSelectorControlPropertyAvailableItems:
				answer = GetNumberItems() * sizeof(UInt32);
				break;
				
			case kCMIOSelectorControlPropertyItemName:
				answer = sizeof(CFStringRef);
				break;
			
			default:
				answer = Control::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	void SelectorControl::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOSelectorControlPropertyCurrentItem:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::SelectorControl::GetPropertyData: wrong data size for kCMIOSelectorControlPropertyCurrentItem");
				*static_cast<UInt32*>(data) = GetCurrentItemID();
				dataUsed = sizeof(UInt32);
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

	void SelectorControl::CurrentItemChanged() const
	{
		PropertyAddress address(kCMIOSelectorControlPropertyCurrentItem);
		PropertiesChanged(1, &address);
	}

	void SelectorControl::AvailableItemsChanged() const
	{
		PropertyAddress address(kCMIOSelectorControlPropertyAvailableItems);
		PropertiesChanged(1, &address);
	}

	void SelectorControl::ItemNameChanged() const
	{
		PropertyAddress address(kCMIOSelectorControlPropertyItemName);
		PropertiesChanged(1, &address);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	FeatureControl::FeatureControl(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn, Device& owningDevice) :
		Control(objectID, classID, plugIn, owningDevice)
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~FeatureControl()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	FeatureControl::~FeatureControl()
	{
	}

	CMIOClassID	FeatureControl::GetBaseClassID() const
	{
		return kCMIOFeatureControlClassID;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::CopyName();
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef FeatureControl::CopyName() const
	{
		CFStringRef name = NULL;
		
		switch (GetClassID())
		{
			case kCMIOHueControlClassID:					name = CFSTR("Hue");					break;
			case kCMIOSaturationControlClassID:				name = CFSTR("Saturation");				break;
			case kCMIOContrastControlClassID:				name = CFSTR("Contrast");				break;
			case kCMIOSharpnessControlClassID:				name = CFSTR("Sharpness");				break;
			case kCMIOBrightnessControlClassID:				name = CFSTR("Brightness");				break;
			case kCMIOGainControlClassID:					name = CFSTR("Gain");					break;
			case kCMIOIrisControlClassID:					name = CFSTR("Iris");					break;
			case kCMIOShutterControlClassID:				name = CFSTR("Shutter");				break;
			case kCMIOExposureControlClassID:				name = CFSTR("Exposure");				break;
			case kCMIOWhiteBalanceUControlClassID:			name = CFSTR("White Balance U");		break;
			case kCMIOWhiteBalanceVControlClassID:			name = CFSTR("White Balance V");		break;
			case kCMIOGammaControlClassID:					name = CFSTR("Gamma");					break;
			case kCMIOTemperatureControlClassID:			name = CFSTR("Temperature");			break;
			case kCMIOZoomControlClassID:					name = CFSTR("Zoom");					break;
			case kCMIOFocusControlClassID:					name = CFSTR("Focus");					break;
			case kCMIOPanControlClassID:					name = CFSTR("Pan");					break;
			case kCMIOTiltControlClassID:					name = CFSTR("Tilt");					break;
			case kCMIOOpticalFilterClassID:					name = CFSTR("Optical Filter");			break;
			case kCMIOBacklightCompensationControlClassID:	name = CFSTR("Backlight Compensation");	break;
			case kCMIOPowerLineFrequencyControlClassID:		name = CFSTR("Power Line Frequency");	break;
			case kCMIONoiseReductionControlClassID:			name = CFSTR("Noise Reduction");		break;
			default:										name = CFSTR("Unknown");				break;
		}
		
		return name;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::HasProperty()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FeatureControl::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOFeatureControlPropertyOnOff:
				answer = true;
				break;
			
			case kCMIOFeatureControlPropertyAutomaticManual:
				answer = true;
				break;

			case kCMIOFeatureControlPropertyAbsoluteNative:
				answer = true;
				break;

			case kCMIOFeatureControlPropertyTune:
				answer = HasTune();
				break;
			
			case kCMIOFeatureControlPropertyNativeValue:
				answer = true;
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteValue:
				answer = AbsoluteNativeSettable();
				break;
			
			case kCMIOFeatureControlPropertyNativeRange:
				answer = true;
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteRange:
				answer = AbsoluteNativeSettable();
				break;
			
			case kCMIOFeatureControlPropertyConvertNativeToAbsolute:
				answer = HasNativeToAbsolute();
				break;
			
			case kCMIOFeatureControlPropertyConvertAbsoluteToNative:
				answer = HasAbsoluteToNative();
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteUnitName:
				answer = AbsoluteNativeSettable();
				break;
			
			default:
				answer = Control::HasProperty(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FeatureControl::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		// Initialize the return value
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOFeatureControlPropertyOnOff:
				answer = OnOffSettable() and not IsReadOnly();
				break;
			
			case kCMIOFeatureControlPropertyAutomaticManual:
				answer = AutomaticManualSettable() and not IsReadOnly() ;
				break;

			case kCMIOFeatureControlPropertyAbsoluteNative:
				answer = AbsoluteNativeSettable() and not IsReadOnly();
				break;

			case kCMIOFeatureControlPropertyTune:
				answer = not IsReadOnly();
				break;
			
			case kCMIOFeatureControlPropertyNativeValue:
				answer = not IsReadOnly();
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteValue:
				answer = not IsReadOnly();
				break;
			
			case kCMIOFeatureControlPropertyNativeRange:
				answer = false;
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteRange:
				answer = false;
				break;
			
			case kCMIOFeatureControlPropertyConvertNativeToAbsolute:
				answer = false;
				break;
			
			case kCMIOFeatureControlPropertyConvertAbsoluteToNative:
				answer = false;
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteUnitName:
				answer = false;
				break;

			default:
				answer = Control::IsPropertySettable(address);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32	FeatureControl::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		// Initialize the return value
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOFeatureControlPropertyOnOff:
				answer = sizeof(UInt32);
				break;
			
			case kCMIOFeatureControlPropertyAutomaticManual:
				answer = sizeof(UInt32);
				break;

			case kCMIOFeatureControlPropertyAbsoluteNative:
				answer = sizeof(UInt32);
				break;
			
			case kCMIOFeatureControlPropertyTune:
				answer = sizeof(UInt32);
				break;
			
			case kCMIOFeatureControlPropertyNativeValue:
				answer = sizeof(Float32);
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteValue:
				answer = sizeof(Float32);
				break;
			
			case kCMIOFeatureControlPropertyNativeRange:
				answer = sizeof(AudioValueRange);
				break;
				
			case kCMIOFeatureControlPropertyAbsoluteRange:
				answer = sizeof(AudioValueRange);
				break;
				
			case kCMIOFeatureControlPropertyConvertNativeToAbsolute:
				answer = sizeof(Float32);
				break;
			
			case kCMIOFeatureControlPropertyConvertAbsoluteToNative:
				answer = sizeof(Float32);
				break;
			
			default:
				answer = Control::GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FeatureControl::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOFeatureControlPropertyOnOff:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyOnOff");
				*static_cast<UInt32*>(data) = GetOnOff();
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOFeatureControlPropertyAutomaticManual:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAutomaticManual");
				*static_cast<UInt32*>(data) = GetAutomaticManual();
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteNative:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAbsoluteNative");
				*static_cast<UInt32*>(data) = GetAbsoluteNative();
				dataUsed = sizeof(UInt32);
				break;
			
			case kCMIOFeatureControlPropertyTune:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyTune");
				*static_cast<UInt32*>(data) = GetTune();
				dataUsed = sizeof(UInt32);
				break;

			case kCMIOFeatureControlPropertyNativeValue:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyNativeValue");
				*static_cast<Float32*>(data) = GetNativeValue();
				dataUsed = sizeof(Float32);
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteValue:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAbsoluteValue");
				*static_cast<Float32*>(data) = GetAbsoluteValue();
				dataUsed = sizeof(Float32);
				break;
			
			case kCMIOFeatureControlPropertyNativeRange:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyNativeRange");
				static_cast<AudioValueRange*>(data)->mMinimum = GetMinimumNativeValue();
				static_cast<AudioValueRange*>(data)->mMaximum = GetMaximumNativeValue();
				dataUsed = dataSize;
				break;
			
			case kCMIOFeatureControlPropertyAbsoluteRange:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAbsoluteRange");
				static_cast<AudioValueRange*>(data)->mMinimum = GetMinimumAbsoluteValue();
				static_cast<AudioValueRange*>(data)->mMaximum = GetMaximumAbsoluteValue();
				dataUsed = dataSize;
				break;
			
			case kCMIOFeatureControlPropertyConvertNativeToAbsolute:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyConvertNativeToAbsolute");
				*static_cast<Float32*>(data) = ConverNativeValueToAbsoluteValue(*static_cast<Float32*>(data));
				dataUsed = sizeof(Float32);
				break;
			
			case kCMIOFeatureControlPropertyConvertAbsoluteToNative:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyConvertAbsoluteToNative");
				*static_cast<Float32*>(data) = ConverAbsoluteValueToNativeValue(*static_cast<Float32*>(data));
				dataUsed = sizeof(Float32);
				break;
		
			default:
				Control::GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FeatureControl::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIOFeatureControlPropertyOnOff:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyOnOff");
				SetOnOff(*static_cast<const UInt32*>(data));
				break;
		
			case kCMIOFeatureControlPropertyAutomaticManual:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAutomaticManual");
				SetAutomaticManual(*static_cast<const UInt32*>(data));
				break;
		
			case kCMIOFeatureControlPropertyAbsoluteNative:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAutomaticManual");
				SetAbsoluteNative(*static_cast<const UInt32*>(data));
				break;
		
			case kCMIOFeatureControlPropertyTune:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyTune");
				SetTune(*static_cast<const UInt32*>(data));
				break;

			case kCMIOFeatureControlPropertyNativeValue:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyNativeValue");
				SetNativeValue(*static_cast<const Float32*>(data));
				break;
		
			case kCMIOFeatureControlPropertyAbsoluteValue:
				ThrowIf(dataSize != GetPropertyDataSize(address, qualifierDataSize, qualifierData), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FeatureControl::GetPropertyData: wrong data size for kCMIOFeatureControlPropertyAbsoluteValue");
				SetAbsoluteValue(*static_cast<const Float32*>(data));
				break;
		
			default:
				Control::SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FeatureControl::CopyAbsoluteUnitName();
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CFStringRef FeatureControl::CopyAbsoluteUnitName() const
	{
		CFStringRef name = NULL;
		
		switch (GetClassID())
		{
			case kCMIOHueControlClassID:					name = CFSTR("Degrees");	break;
			case kCMIOSaturationControlClassID:				name = CFSTR("%");			break;
			case kCMIOSharpnessControlClassID:				name = CFSTR("Units");		break;
			case kCMIOBrightnessControlClassID:				name = CFSTR("%");			break;
			case kCMIOGainControlClassID:					name = CFSTR("dB");			break;
			case kCMIOIrisControlClassID:					name = CFSTR("F");			break;
			case kCMIOShutterControlClassID:				name = CFSTR("Seconds");	break;
			case kCMIOExposureControlClassID:				name = CFSTR("EV");			break;
			case kCMIOWhiteBalanceUControlClassID:			name = CFSTR("K");			break;
			case kCMIOWhiteBalanceVControlClassID:			name = CFSTR("K");			break;
			case kCMIOGammaControlClassID:					name = CFSTR("Units");		break;
			case kCMIOTemperatureControlClassID:			name = CFSTR("Units");		break;
			case kCMIOZoomControlClassID:					name = CFSTR("Power");		break;
			case kCMIOFocusControlClassID:					name = CFSTR("Meters");		break;
			case kCMIOPanControlClassID:					name = CFSTR("Degrees");	break;
			case kCMIOTiltControlClassID:					name = CFSTR("Degrees");	break;
			case kCMIOOpticalFilterClassID:					name = CFSTR("Units");		break;
			case kCMIOBacklightCompensationControlClassID:	name = CFSTR("Units");		break;
			case kCMIOPowerLineFrequencyControlClassID:		name = CFSTR("Hertz");		break;
		}
		
		return name;
	}
}}
