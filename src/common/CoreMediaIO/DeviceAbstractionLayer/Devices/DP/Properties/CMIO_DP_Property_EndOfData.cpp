/*
	    File: CMIO_DP_Property_EndOfData.cpp
	Abstract: Implements the kCMIOStreamPropertyEndOfData property.
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

#include "CMIO_DP_Property_EndOfData.h"

// Self Include
#include "CMIO_DP_Property_EndOfData.h"

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
	bool EndOfData::IsActive(const CMIOObjectPropertyAddress& /*address*/) const
	{
		return true;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool EndOfData::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyEndOfData:
				answer = true;
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 EndOfData::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyEndOfData:
				answer = sizeof(UInt32);
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void EndOfData::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyEndOfData:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::EndOfData::GetPropertyData: wrong data size for kCMIOStreamPropertyEndOfData");
				*(static_cast<UInt32*>(data)) = mEndOfData;
				dataUsed = sizeof(UInt32);
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void EndOfData::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, const void* data)
	{
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyEndOfData:
				ThrowIf(dataSize != sizeof(UInt32), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::EndOfData::SetPropertyData: wrong data size for kCMIOStreamPropertyEndOfData");
				UInt32 endOfData = *(static_cast<const UInt32*>(data));
				
				if (endOfData != mEndOfData)
				{
					// Attempt to inform the stream that end-of-data has been reached
					GetOwningStream().SetEndOfData(endOfData);
					mEndOfData = endOfData;

					PropertyAddress changedAddress(kCMIOStreamPropertyEndOfData);
					mOwningStream.PropertiesChanged(1, &changedAddress);
				}
				break;
		};
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 EndOfData::GetNumberAddressesImplemented() const
	{
		return 1;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void EndOfData::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			case 0:
				address.mSelector = kCMIOStreamPropertyEndOfData;
				address.mScope = kCMIOObjectPropertyScopeWildcard;
				address.mElement = kCMIOObjectPropertyElementWildcard;
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetEndOfData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void EndOfData::SetEndOfData(Boolean endOfData, bool sendChangeNotifications) 
	{
		// Send out property change notifications there has been a change
		if (endOfData != mEndOfData)
		{
			mEndOfData = endOfData;
			
			if (sendChangeNotifications)
			{
				PropertyAddress changedAddress(kCMIOStreamPropertyEndOfData);
				GetOwningStream().PropertiesChanged(1, &changedAddress);
			}
		}
	}

}}}