/*
	    File: CMIO_DP_Object.cpp
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
#include "CMIO_DP_Object.h"

// Internal Includes
#include "CMIO_DP_Property_Base.h"
#include "CMIO_DP_Device.h"
#include "CMIO_DP_PlugIn.h"
#include "CMIO_DP_Stream.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAException.h"
#include "CAMutex.h"
#include "CATokenMap.h"

// Standard Library Includes
#include <map>

namespace
{
	static CATokenMap<CMIO::DP::Object>* sObjectIDMap = NULL;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Object_MapObject()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	inline UInt32 Object_MapObject(UInt32 id, CMIO::DP::Object* object)
	{
		if (sObjectIDMap == NULL)
		{
			sObjectIDMap = new CATokenMap<CMIO::DP::Object>();
		}
		
		sObjectIDMap->AddMapping(id, object);
		
		return id;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Object_UnmapObject()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	inline void Object_UnmapObject(CMIO::DP::Object* object)
	{
		if (sObjectIDMap != NULL)
		{
			UInt32 id = object->GetObjectID();
			sObjectIDMap->RemoveMapping(id, object);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Object_GetObjectForID()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	inline CMIO::DP::Object* Object_GetObjectForID(UInt32 id)
	{
		CMIO::DP::Object* answer = NULL;
		
		if (sObjectIDMap != NULL)
		{
			answer = sObjectIDMap->GetObject(id);
		}
		
		return answer;
	}
}

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Object()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Object::Object(CMIOObjectID objectID, CMIOClassID classID, PlugIn& plugIn) :
		mObjectID(objectID),
		mClassID(classID),
		mPlugIn(plugIn),
		mPropertyMap()
	{
		if (objectID != kCMIOObjectUnknown)
		{
			Object_MapObject(objectID, this);
		}
	}

	Object::~Object()
	{
		Object_UnmapObject(this);
	}

	void Object::Initialize()
	{
	}

	void Object::Teardown()
	{
	}

	void Object::SetObjectID(CMIOObjectID objectID)
	{
		Object_UnmapObject(this);
		mObjectID = objectID;
		if (objectID != kCMIOObjectUnknown)
		{
			Object_MapObject(objectID, this);
		}
	}

	bool Object::IsSubClass(CMIOClassID classID, CMIOClassID baseClassID)
	{
		bool answer = false;
		
		switch (baseClassID)
		{
			case kCMIOObjectClassID:
			{
				//  all classes are subclasses of CMIOObject
				answer = true;
			}
			break;
			
			case kCMIOControlClassID:
			{
				switch (classID)
				{
					case kCMIOControlClassID:
					case kCMIOBooleanControlClassID:
					case kCMIOSelectorControlClassID:
					case kCMIOFeatureControlClassID:
					case kCMIOJackControlClassID:
					case kCMIODirectionControlClassID:
					case kCMIODataSourceControlClassID:
					case kCMIODataDestinationControlClassID:
					case kCMIOBlackLevelControlClassID:
					case kCMIOWhiteLevelControlClassID:
					case kCMIOHueControlClassID:
					case kCMIOSaturationControlClassID:
					case kCMIOContrastControlClassID:
					case kCMIOSharpnessControlClassID:
					case kCMIOBrightnessControlClassID:
					case kCMIOGainControlClassID:
					case kCMIOIrisControlClassID:
					case kCMIOShutterControlClassID:
					case kCMIOExposureControlClassID:
					case kCMIOWhiteBalanceUControlClassID:
					case kCMIOWhiteBalanceVControlClassID:
					case kCMIOGammaControlClassID:
					case kCMIOTemperatureControlClassID:
					case kCMIOZoomControlClassID:
					case kCMIOFocusControlClassID:
					case kCMIOPanControlClassID:
					case kCMIOTiltControlClassID:
					case kCMIOOpticalFilterClassID:
					case kCMIOBacklightCompensationControlClassID:
					case kCMIOPowerLineFrequencyControlClassID:
					case kCMIONoiseReductionControlClassID:
					{
						answer = true;
					}
					break;
				};
			}
			break;
			
			case kCMIOBooleanControlClassID:
			{
				switch (classID)
				{
					case kCMIOBooleanControlClassID:
					case kCMIOJackControlClassID:
					case kCMIODirectionControlClassID:
					{
						answer = true;
					}
					break;
				};
			}
			break;
			
			case kCMIOSelectorControlClassID:
			{
				switch (classID)
				{
					case kCMIOSelectorControlClassID:
					case kCMIODataSourceControlClassID:
					case kCMIODataDestinationControlClassID:
					{
						answer = true;
					}
					break;
				};
			}
			break;
			
			case kCMIOFeatureControlClassID:
			{
				switch (classID)
				{
					case kCMIOFeatureControlClassID:
					case kCMIOBlackLevelControlClassID:
					case kCMIOWhiteLevelControlClassID:
					case kCMIOHueControlClassID:
					case kCMIOSaturationControlClassID:
					case kCMIOContrastControlClassID:
					case kCMIOSharpnessControlClassID:
					case kCMIOBrightnessControlClassID:
					case kCMIOGainControlClassID:
					case kCMIOIrisControlClassID:
					case kCMIOShutterControlClassID:
					case kCMIOExposureControlClassID:
					case kCMIOWhiteBalanceUControlClassID:
					case kCMIOWhiteBalanceVControlClassID:
					case kCMIOGammaControlClassID:
					case kCMIOTemperatureControlClassID:
					case kCMIOZoomControlClassID:
					case kCMIOFocusControlClassID:
					case kCMIOPanControlClassID:
					case kCMIOTiltControlClassID:
					case kCMIOOpticalFilterClassID:
					case kCMIOBacklightCompensationControlClassID:
					case kCMIOPowerLineFrequencyControlClassID:
					case kCMIONoiseReductionControlClassID:
					{
						answer = true;
					}
					break;
				};
			}
			break;
			
			case kCMIODeviceClassID:
			{
				switch (classID)
				{
					case kCMIODeviceClassID:
					{
						answer = true;
					}
					break;
				};
			}
			break;
			
			//  leaf classes
			case kCMIOJackControlClassID:
			case kCMIODirectionControlClassID:
			case kCMIODataSourceControlClassID:
			case kCMIODataDestinationControlClassID:
			case kCMIOBlackLevelControlClassID:
			case kCMIOWhiteLevelControlClassID:
			case kCMIOHueControlClassID:
			case kCMIOSaturationControlClassID:
			case kCMIOContrastControlClassID:
			case kCMIOSharpnessControlClassID:
			case kCMIOBrightnessControlClassID:
			case kCMIOGainControlClassID:
			case kCMIOIrisControlClassID:
			case kCMIOShutterControlClassID:
			case kCMIOExposureControlClassID:
			case kCMIOWhiteBalanceUControlClassID:
			case kCMIOWhiteBalanceVControlClassID:
			case kCMIOGammaControlClassID:
			case kCMIOTemperatureControlClassID:
			case kCMIOZoomControlClassID:
			case kCMIOFocusControlClassID:
			case kCMIOPanControlClassID:
			case kCMIOTiltControlClassID:
			case kCMIOOpticalFilterClassID:
			case kCMIOBacklightCompensationControlClassID:
			case kCMIOPowerLineFrequencyControlClassID:
			case kCMIONoiseReductionControlClassID:
			case kCMIOSystemObjectClassID:
			case kCMIOPlugInClassID:
			case kCMIOStreamClassID:
			{
				answer = classID == baseClassID;
			}
			break;
			
			default:
			{
				#if CMIO_Debug
					char classIDString[5] = CA4CCToCString(baseClassID);
					DebugMessage("CMIO::DP::Object::IsSubClass: unknown base class '%s'", classIDString);
				#endif
				answer = classID == baseClassID;
			}
			break;
			
		};
		
		return answer;
	}

	CAMutex* Object::GetObjectStateMutex()
	{
		//	most object don't have a state mutex
		return NULL;
	}

	void Object::Show() const
	{
		//  the default implementation is to print the object's ID, class ID and name (if it has one)
		
		//  make a string for the class ID
		char classID[] = CA4CCToCString(mClassID);
		
		//  get the object's name
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
		
		//  make a C string out of the name
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
		
		//  print the information to the standard output
		printf("CMIOObjectID:\t\t0x%lX\n\tCMIOClassID:\t'%s'\n\tName:\t\t\t%s\n", (long unsigned int)mObjectID, classID, name);
	}

	Object* Object::GetObjectByID(CMIOObjectID objectID)
	{
		return Object_GetObjectForID(objectID);
	}

	Device* Object::GetDeviceByID(CMIOObjectID objectID)
	{
		Device* answer = NULL;
		Object* object = Object_GetObjectForID(objectID);
		if ((object != NULL) && IsSubClass(object->GetClassID(), kCMIODeviceClassID))
		{
			answer = static_cast<Device*>(object);
		}
		return answer;
	}

	Stream* Object::GetStreamByID(CMIOObjectID objectID)
	{
		Stream* answer = NULL;
		Object* object = Object_GetObjectForID(objectID);
		if ((object != NULL) && IsSubClass(object->GetClassID(), kCMIOStreamClassID))
		{
			answer = static_cast<Stream*>(object);
		}
		return answer;
	}

	typedef std::map<CMIOObjectID, CAMutex*>	ObjectStateMutexMap;
	static ObjectStateMutexMap*	sObjectStateMutexMap = NULL;

	CAMutex* Object::GetObjectStateMutexByID(CMIOObjectID objectID)
	{
		CAMutex* answer = NULL;
		
		if (sObjectStateMutexMap != NULL)
		{
			ObjectStateMutexMap::iterator iterator = sObjectStateMutexMap->find(objectID);
			if (iterator != sObjectStateMutexMap->end())
			{
				answer = iterator->second;
			}
		}
		
		return answer;
	}

	void Object::SetObjectStateMutexForID(CMIOObjectID objectID, CAMutex* mutex)
	{
		if (sObjectStateMutexMap == NULL)
		{
			sObjectStateMutexMap = new ObjectStateMutexMap;
		}
		
		if (sObjectStateMutexMap != NULL)
		{
			ObjectStateMutexMap::iterator iterator = sObjectStateMutexMap->find(objectID);
			if (iterator != sObjectStateMutexMap->end())
			{
				iterator->second = mutex;
			}
			else
			{
				sObjectStateMutexMap->insert(ObjectStateMutexMap::value_type(objectID, mutex));
			}
		}
	}

	bool Object::HasProperty(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyListenerAdded:
			case kCMIOObjectPropertyListenerRemoved:
				answer = true;
				break;
			
			default:
			{
				Property::Base* property = FindActivePropertyByAddress(address);
				if (property != NULL)
				{
					answer = true;
				}
			}
			break;
		};
		
		return answer;
	}

	bool Object::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyListenerAdded:
			case kCMIOObjectPropertyListenerRemoved:
				answer = true;
				break;
				
			default:
			{
				Property::Base* property = FindActivePropertyByAddress(address);
				if (property != NULL)
				{
					answer = property->IsPropertySettable(address);
				}
				else
				{
					DebugMessage("CMIO::DP::Object::IsPropertySettable: unknown property");
					Throw(CAException(kCMIOHardwareUnknownPropertyError));
				}
			}
			break;
		};
		
		return answer;
	}

	UInt32 Object::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyListenerAdded:
			case kCMIOObjectPropertyListenerRemoved:
				answer = sizeof(CMIOObjectPropertyAddress);
				break;
				
			default:
			{
				Property::Base* property = FindActivePropertyByAddress(address);
				if (property != NULL)
				{
					answer = property->GetPropertyDataSize(address, qualifierDataSize, qualifierData);
				}
				else
				{
					DebugMessage("CMIO::DP::Object::GetPropertyDataSize: unknown property");
					Throw(CAException(kCMIOHardwareUnknownPropertyError));
				}
			}
			break;
		};
		
		return answer;
	}

	void Object::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyListenerAdded:
			case kCMIOObjectPropertyListenerRemoved:
				ThrowIf(dataSize != sizeof(CMIOObjectPropertyAddress), CAException(kCMIOHardwareBadPropertySizeError), "IOA_Device::GetPropertyData: wrong data size for kCMIOObjectPropertyListenerAdded/kCMIOObjectPropertyListenerRemoved");
				memset(data, 0, dataSize);
				break;
				
			default:
			{
				Property::Base* property = FindActivePropertyByAddress(address);
				if (property != NULL)
				{
					property->GetPropertyData(address, qualifierDataSize, qualifierData, dataSize, dataUsed, data);
				}
				else
				{
					DebugMessage("CMIO::DP::Object::GetPropertyData: unknown property");
					Throw(CAException(kCMIOHardwareUnknownPropertyError));
				}
			}
			break;
		};
	}

	void Object::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data)
	{
		ThrowIf(!IsPropertySettable(address), CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::Object::SetPropertyData: address isn't settable");
		switch (address.mSelector)
		{
			case kCMIOObjectPropertyListenerAdded:
				ThrowIf(dataSize != sizeof(CMIOObjectPropertyAddress), CAException(kCMIOHardwareBadPropertySizeError), "IOA_Device::SetPropertyData: wrong data size for kCMIOObjectPropertyListenerAdded");
				PropertyListenerAdded(*(static_cast<const CMIOObjectPropertyAddress*>(data)));
				break;
			
			case kCMIOObjectPropertyListenerRemoved:
				ThrowIf(dataSize != sizeof(CMIOObjectPropertyAddress), CAException(kCMIOHardwareBadPropertySizeError), "IOA_Device::SetPropertyData: wrong data size for kCMIOObjectPropertyListenerRemoved");
				PropertyListenerRemoved(*(static_cast<const CMIOObjectPropertyAddress*>(data)));
				break;
				
			default:
			{
				Property::Base* property = FindActivePropertyByAddress(address);
				if (property != NULL)
				{
					property->SetPropertyData(address, qualifierDataSize, qualifierData, dataSize, data);
				}
				else
				{
					DebugMessage("CMIO::DP::Object::SetPropertyData: unknown property");
					Throw(CAException(kCMIOHardwareUnknownPropertyError));
				}
			}
			break;
		};
	}

	void Object::PropertiesChanged(UInt32 numberAddresses, const CMIOObjectPropertyAddress addresses[]) const
	{
		//	note that we need to be sure that the object state mutex is not held while we call the listeners
		bool ownsStateMutex = false;
		CAMutex* objectStateMutex = const_cast<Object*>(this)->GetObjectStateMutex();
		if (objectStateMutex != NULL)
		{
			ownsStateMutex = objectStateMutex->IsOwnedByCurrentThread();
			if (ownsStateMutex)
			{
				objectStateMutex->Unlock();
			}
		}
			
		OSStatus error = CMIOObjectPropertiesChanged(mPlugIn.GetInterface(), mObjectID, numberAddresses, addresses);
		AssertNoError(error, "CMIO::DP::Object::PropertiesChanged: got an error calling the listeners");
			
		//	re-lock the mutex
		if ((objectStateMutex != NULL) && ownsStateMutex)
		{
			objectStateMutex->Lock();
		}
	}

	void Object::PropertyListenerAdded(const CMIOObjectPropertyAddress& /*address*/)
	{
	}

	void Object::PropertyListenerRemoved(const CMIOObjectPropertyAddress& /*address*/)
	{
	}

	void Object::AddProperty(Property::Base* property)
	{
		//  get the number of addresses implemented by this property object
		UInt32 numberAddresses = property->GetNumberAddressesImplemented();
		
		//  iterate across the addresses
		for(UInt32 index = 0; index < numberAddresses; ++index)
		{
			//  get the address
			PropertyAddress address;
			property->GetImplementedAddressByIndex(index, address);
			
			//  look to see if it has already been spoken for
			PropertyMap::iterator propertyMapIterator = FindPropertyByAddress(address);
			ThrowIf(propertyMapIterator != mPropertyMap.end(), CAException(kCMIOHardwareIllegalOperationError), "CMIO::DP::Object::AddProperty: redefined address");
			
			//  it isn't, so add it
			PropertyMapItem item(address, property);
			mPropertyMap.push_back(item);
		}
	}

	void Object::RemoveProperty(Property::Base* property)
	{
		//  get the number of addresses implemented by this property object
		UInt32 numberAddresses = property->GetNumberAddressesImplemented();
		
		//  iterate across the addresses
		for(UInt32 index = 0; index < numberAddresses; ++index)
		{
			//  get the address
			PropertyAddress address;
			property->GetImplementedAddressByIndex(index, address);
			
			//  look for it in the property map
			PropertyMap::iterator propertyMapIterator = FindPropertyByAddress(address);
			if (propertyMapIterator != mPropertyMap.end())
			{
				//  we found it, so get rid of it
				mPropertyMap.erase(propertyMapIterator);
			}
		}
	}

	Object::PropertyMap::iterator Object::FindPropertyByAddress(const CMIOObjectPropertyAddress& address)
	{
		PropertyMap::iterator answer = mPropertyMap.end();
		
		//  iterate through the property map
		PropertyMap::iterator propertyMapIterator = mPropertyMap.begin();
		while ((answer == mPropertyMap.end()) && (propertyMapIterator != mPropertyMap.end()))
		{
			//  check to see if the addresses match
			if (PropertyAddress::IsCongruentAddress(propertyMapIterator->first, address))
			{
				//  they do
				answer = propertyMapIterator;
			}
			
			std::advance(propertyMapIterator, 1);
		}
		
		return answer;
	}

	Property::Base* Object::FindActivePropertyByAddress(const CMIOObjectPropertyAddress& address) const
	{
		Property::Base* answer = NULL;
		
		//  iterate through the property map
		PropertyMap::const_iterator propertyMapIterator = mPropertyMap.begin();
		while ((answer == NULL) && (propertyMapIterator != mPropertyMap.end()))
		{
			//  check to see if the addresses match
			if (PropertyAddress::IsCongruentAddress(propertyMapIterator->first, address))
			{
				//  they do, so make sure that what we found is active
				if (propertyMapIterator->second->IsActive(address))
				{
					answer = propertyMapIterator->second;
				}
			}
			
			std::advance(propertyMapIterator, 1);
		}
		
		return answer;
	}
}}
