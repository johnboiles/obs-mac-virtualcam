/*
	    File: CMIO_DP_Property_Deck.cpp
	Abstract: Implements the kCMIOStreamPropertyCanProcessDeckCommand, kCMIOStreamPropertyDeck, kCMIOStreamPropertyDeckFrameNumber, kCMIOStreamPropertyDeckDropness,
				kCMIOStreamPropertyDeckThreaded, kCMIOStreamPropertyDeckLocal, and kCMIOStreamPropertyDeckCueing properties.
	
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

#include "CMIO_DP_Property_Deck.h"

// Internal Includes
#include "CMIO_DP_Stream.h"

// Public Utility Includes
#include "CMIODebugMacros.h"
#include "CMIO_PropertyAddress.h"

// CA Public Utility Includes
#include "CAException.h"

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsActive()
	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Deck::IsActive(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool Deck::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyCanProcessDeckCommand:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDeckFrameNumber:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDeck:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDeckDropness:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDeckThreaded:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDeckLocal:
				answer = false;
				break;
				
			case kCMIOStreamPropertyDeckCueing:
				answer = false;
				break;
		};
		
		return answer;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Deck::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyCanProcessDeckCommand:
				answer = sizeof(Boolean);
				break;

			case kCMIOStreamPropertyDeckFrameNumber:
				answer = sizeof(Float64);
				break;
				
			case kCMIOStreamPropertyDeck:
				answer = sizeof(CMIOStreamDeck);
				break;
				
			case kCMIOStreamPropertyDeckDropness:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyDeckThreaded:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyDeckLocal:
				answer = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyDeckCueing:
				answer = sizeof(SInt32);
				break;
		};
		
		return answer;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyCanProcessDeckCommand:
				ThrowIf(dataSize != sizeof(Boolean), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyCanProcessDeckCommand");
				*static_cast<Boolean*>(data) = true;
				dataUsed = sizeof(Boolean);
				break;
				
			case kCMIOStreamPropertyDeck:
				ThrowIf(dataSize != sizeof(CMIOStreamDeck), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyDeck");
				*static_cast<CMIOStreamDeck*>(data) = mUseOneShots ? mStreamDeckOneShot(GetOwningStream()) : mStreamDeck;
				dataUsed = sizeof(CMIOStreamDeck);
				break;
				
			case kCMIOStreamPropertyDeckFrameNumber:
				ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyDeckFrameNumber");
				*static_cast<Float64*>(data) = mUseOneShots ? mTimecodeOneShot(GetOwningStream()) : mTimecode;
				dataUsed = sizeof(Float64);
				break;
				
			case kCMIOStreamPropertyDeckDropness:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyDeckDropness");
				*static_cast<UInt32*>(data) = mUseOneShots ? mDropnessOneShot(GetOwningStream()) : mDropness;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyDeckThreaded:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyDeckThreaded");
				*static_cast<UInt32*>(data) = mUseOneShots ? mThreadedOneShot(GetOwningStream()) : mThreaded;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyDeckLocal:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyDeckThreaded");
				*static_cast<UInt32*>(data) = mUseOneShots ? mLocalOneShot(GetOwningStream()) : mLocal;
				dataUsed = sizeof(UInt32);
				break;
				
			case kCMIOStreamPropertyDeckCueing:
				ThrowIf(dataSize != sizeof(SInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::Property::Deck::GetPropertyData: wrong data size for kCMIOStreamPropertyDeckCueing");
				*static_cast<SInt32*>(data) = mUseOneShots ? mCueingOneShot(GetOwningStream()) : mCueing;
				dataUsed = sizeof(SInt32);
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 Deck::GetNumberAddressesImplemented() const
	{
		return 7;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			case 0:
				address.mSelector = kCMIOStreamPropertyCanProcessDeckCommand;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 1:
				address.mSelector = kCMIOStreamPropertyDeckFrameNumber;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 2:
				address.mSelector = kCMIOStreamPropertyDeck;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 3:
				address.mSelector = kCMIOStreamPropertyDeckDropness;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 4:
				address.mSelector = kCMIOStreamPropertyDeckThreaded;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 5:
				address.mSelector = kCMIOStreamPropertyDeckLocal;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
			case 6:
				address.mSelector = kCMIOStreamPropertyDeckCueing;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetStreamDeck()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::SetStreamDeck(CMIOStreamDeck streamDeck, bool sendChangeNotifications) 
	{
		mStreamDeck = streamDeck;
		
		if (sendChangeNotifications)
		{
			PropertyAddress changedAddress(kCMIOStreamPropertyDeck);
			GetOwningStream().PropertiesChanged(1, &changedAddress);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetTimecode()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::SetTimecode(Float64 timecode, bool sendChangeNotifications) 
	{
		mTimecode = timecode;
		
		if (sendChangeNotifications)
		{
			PropertyAddress changedAddress(kCMIOStreamPropertyDeckFrameNumber);
			GetOwningStream().PropertiesChanged(1, &changedAddress);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetCueing()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Deck::SetCueing(SInt32 cueing, bool sendChangeNotifications) 
	{
		mCueing = cueing;
		if (sendChangeNotifications)
		{
			PropertyAddress changedAddress(kCMIOStreamPropertyDeckCueing);
			GetOwningStream().PropertiesChanged(1, &changedAddress);
		}
	}
}}}