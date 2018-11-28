/*
	    File: CMIO_DP_Property_FormatList.cpp
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
#include "CMIO_DP_Property_FormatList.h"

// Internal Includes
#include "CMIO_DP_Device.h"
#include "CMIO_DP_Stream.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CMIODebugMacros.h"
#include "CAAudioValueRange.h"
#include "CAException.h"

namespace CMIO { namespace DP { namespace Property
{
	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FormatList()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	FormatList::FormatList(Stream* owningStream) :
		Base(),
		mOwningStream(owningStream),
		mCurrentFormat(),
		mPreferredFormat(),
		mDescriptions(true),
		mStillImageDescriptions(true),
		mCurrentFormatIsSettable(true),
		mStillImagePropertiesActive(false),
		mFrameRatePropertiesActive(false),
		mMinimumFrameRatePropertiesActive(false),
		mFrameRateRangesPropertiesActive(false),
		mPreferredFormatIsActive(false),
		mPreferredFrameRateIsActive(false),
		mCurrentFrameRate(0.0),
		mMinimumFrameRate(0.0),
		mPreferredFrameRate(0.0),
		mFrameRates(),
		mFrameRateRanges(),
		mAllPossibleFrameRates()
	{
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// ~FormatList()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	FormatList::~FormatList()
	{
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetCurrentFormat()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetCurrentFormat(CMFormatDescriptionRef format, bool tellHardware)
	{
		if (not tellHardware or mOwningStream->TellHardwareToSetFormatDescription(format))
		{
			mCurrentFormat = format;
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAvailableFormats()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetNumberAvailableFormats() const
	{
		return mDescriptions.GetNumberItems();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetAvailableFormatByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMFormatDescriptionRef FormatList::GetAvailableFormatByIndex(UInt32 index) const
	{
		CFTypeRef description = NULL;
		(void) mDescriptions.GetCFType(index, description);
		return static_cast<CMFormatDescriptionRef>(const_cast<void*>(description));
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AddAvailableFormat()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::AddAvailableFormat(CMFormatDescriptionRef description)
	{
		// No need to add the reference if it is already present (or an equivalent one is)
		if (mDescriptions.HasItem(description))
			return;

		// Add it
		 mDescriptions.AppendCFType(description);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// RemoveAllAvailableFormats()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::RemoveAllAvailableFormats()
	{
		mDescriptions.Clear();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// BestMatchForFormat()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMFormatDescriptionRef FormatList::BestMatchForFormat(CMFormatDescriptionRef description) const 
	{
		UInt32 descriptionCount = GetNumberAvailableFormats();
		if (0 == descriptionCount)
			return NULL;
			
		// See if the description is equivalent to any existing ones
		UInt32 index;
		if (mDescriptions.GetIndexOfItem(description, index))
		{
			CFTypeRef matchedFormat = NULL;
			(void) mDescriptions.GetCFType(index, matchedFormat);
			return static_cast<CMFormatDescriptionRef>(const_cast<void*>(matchedFormat));
		}

		// No exact match, so simply return the first description
		CFTypeRef firstDescription = NULL;
		(void) mDescriptions.GetCFType(0, firstDescription);
		
		return static_cast<CMFormatDescriptionRef>(const_cast<void*>(firstDescription));
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SupportsFormatDescription()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FormatList::SupportsFormatDescription(CMFormatDescriptionRef description) const
	{
		// See if the description is equivalent to any existing ones
		if (mDescriptions.HasItem(description))
			return true;
			
		return false;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetCurrentNumberChannels()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetCurrentNumberChannels() const
	{
		#warning GetCurrentNumberChannels should decided how to report multiple channels
		return 1; /* return mCurrentFormat.mChannelsPerFrame; */
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStillImage()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMSampleBufferRef FormatList::GetStillImage(CMFormatDescriptionRef description) const
	{
		return mOwningStream->TellHardwareToGetStillImage(description);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberStillImageFormats()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetNumberStillImageFormats() const
	{
		return mStillImageDescriptions.GetNumberItems();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetStillImageFormatByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CMFormatDescriptionRef FormatList::GetStillImageFormatByIndex(UInt32 index) const
	{
		CFTypeRef description = NULL;
		(void) mStillImageDescriptions.GetCFType(index, description);
		return static_cast<CMFormatDescriptionRef>(const_cast<void*>(description));
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AddStillImageFormat()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::AddStillImageFormat(CMFormatDescriptionRef description)
	{
		// No need to add the reference if it is already present (or an equivalent one is)
		if (mStillImageDescriptions.HasItem(description))
			return;

		// Add it
		 mStillImageDescriptions.AppendCFType(description);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// RemoveAllStillImageFormats()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::RemoveAllStillImageFormats()
	{
		mStillImageDescriptions.Clear();
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetCurrentFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Float64 FormatList::GetCurrentFrameRate() const
	{
		 return mCurrentFrameRate; 
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetCurrentFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetCurrentFrameRate(Float64 newFrameRate, bool tellHardware)
	{
		// Don't do anything if the frame rate is not supported
		if (not SupportsFrameRate(newFrameRate))
			return;
			
		if (not tellHardware or mOwningStream->TellHardwareToSetFrameRate(newFrameRate))
			mCurrentFrameRate = newFrameRate;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetMinimumFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Float64 FormatList::GetMinimumFrameRate() const
	{
		 return mMinimumFrameRate; 
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetMinimumFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetMinimumFrameRate(Float64 frameRate, bool tellHardware)
	{
		// Don't do anything if the frame rate is not supported
		if (not SupportsFrameRate(frameRate))
			return;
			
		if (not tellHardware or mOwningStream->TellHardwareToSetMinimumFrameRate(frameRate))
			mMinimumFrameRate = frameRate;
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberFrameRates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetNumberFrameRates(CMFormatDescriptionRef format) const
	{
		if (NULL != format)
		{
			FrameRatesMap::const_iterator i = std::find_if(mFrameRates.begin(), mFrameRates.end(), FormatDescriptionEquals<FrameRates>(format));
			ThrowIf(i == mFrameRates.end(), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::GetNumberFrameRates: invalid format description");
			return (*i).second.size();
		}
		return mAllPossibleFrameRates.size();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFrameRateByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	Float64 FormatList::GetFrameRateByIndex(CMFormatDescriptionRef format, UInt32 index) const
	{
		if (NULL != format)
		{
			FrameRatesMap::const_iterator i = std::find_if(mFrameRates.begin(), mFrameRates.end(), FormatDescriptionEquals<FrameRates>(format));
			ThrowIf(i == mFrameRates.end(), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::GetFrameRateByIndex: invalid format description");
			return (*i).second.at(index);
		}
		return mAllPossibleFrameRates.at(index);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetFrameRates()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetFrameRates(CMFormatDescriptionRef format, UInt32 numberOfRates, const Float64 rates[])
	{
		if (NULL != format)
		{
			// Extract the vector of frames for the format for easy reference
			FrameRates& frameRates = mFrameRates[format];
			
			// Erase the existing rates for that format
			frameRates.erase(frameRates.begin(), frameRates.end());
			
			// Add the new rates
			for (UInt32 i = 0 ; i < numberOfRates; ++i)
				frameRates.push_back(rates[i]);
		}
		else
		{
			// Erase the existing rates for that format
			mAllPossibleFrameRates.erase(mAllPossibleFrameRates.begin(), mAllPossibleFrameRates.end());
			
			// Add the new rates
			for (UInt32 i = 0 ; i < numberOfRates; ++i)
				mAllPossibleFrameRates.push_back(rates[i]);
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SupportFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FormatList::SupportsFrameRate(Float64 inNewFrameRate) const
	{
		FrameRatesMap::const_iterator i = mFrameRates.find(mCurrentFormat);
		if (i == mFrameRates.end())
			return false;
			
		const FrameRates& rates = (*i).second;
		return (rates.end() != std::find(rates.begin(), rates.end(), inNewFrameRate));
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SupportsPreferredFrameRate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FormatList::SupportsPreferredFrameRate(Float64 preferredFrameRate) const
	{
		FrameRatesMap::const_iterator i = mFrameRates.find(mCurrentFormat);
		if (i == mFrameRates.end())
			return false;
			
		return (mAllPossibleFrameRates.end() != std::find(mAllPossibleFrameRates.begin(), mAllPossibleFrameRates.end(), preferredFrameRate));
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetFrameRateRangeByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	AudioValueRange FormatList::GetFrameRateRangeByIndex(CMFormatDescriptionRef format, UInt32 index) const
	{
		FrameRateRangeMap::const_iterator i = std::find_if(mFrameRateRanges.begin(), mFrameRateRanges.end(), FormatDescriptionEquals<FrameRateRanges>(format));
		ThrowIf(i == mFrameRateRanges.end(), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::GetFrameRateByIndex: invalid format description");
		return (*i).second.at(index);
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberFrameRateRanges()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetNumberFrameRateRanges(CMFormatDescriptionRef format) const
	{
		FrameRateRangeMap::const_iterator i = std::find_if(mFrameRateRanges.begin(), mFrameRateRanges.end(), FormatDescriptionEquals<FrameRateRanges>(format));
		ThrowIf(i == mFrameRateRanges.end(), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::GetNumberFrameRateRanges: invalid format description");
		return (*i).second.size();
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetFrameRateRanges()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetFrameRateRanges(CMFormatDescriptionRef format, UInt32 numberOfRateRanges, const AudioValueRange rateRanges[])
	{
		// Extract the vector of frame rate ranges for the format for easy reference
		FrameRateRanges& frameRateRanges = mFrameRateRanges[format];
		
		// Erase the existing rate ranges for that format
		frameRateRanges.erase(frameRateRanges.begin(), frameRateRanges.end());
		
		// Add the new rate ranges
		for (UInt32 i = 0 ; i < numberOfRateRanges; ++i)
			frameRateRanges.push_back(rateRanges[i]);
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPreferredFormat()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetPreferredFormat(CMFormatDescriptionRef inNewPreferredFormat, bool tellHardware)
	{
		if (not tellHardware or mOwningStream->TellHardwareToSetPreferredFormatDescription(inNewPreferredFormat))
		{
			mPreferredFormat = inNewPreferredFormat;
			
			// Indicate that kCMIOStreamPropertyPreferredFormatDescription has changed
			PropertyAddressList changedProperties;
			PropertyAddress address(kCMIOStreamPropertyPreferredFormatDescription);
			changedProperties.AppendUniqueItem(address);
			mOwningStream->PropertiesChanged(changedProperties.GetNumberItems(), changedProperties.GetItems());
		}
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPreferredFramerate()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetPreferredFrameRate(Float64 inNewPreferredFrameRate, bool tellHardware)
	{
		if (not tellHardware or mOwningStream->TellHardwareToSetPreferredFrameRate(inNewPreferredFrameRate))
		{
			mPreferredFrameRate = inNewPreferredFrameRate;
			
			// Indicate that kCMIOStreamPropertyPreferredFrameRate has changed
			PropertyAddressList changedProperties;
			PropertyAddress address(kCMIOStreamPropertyPreferredFrameRate);
			changedProperties.AppendUniqueItem(address);
			mOwningStream->PropertiesChanged(changedProperties.GetNumberItems(), changedProperties.GetItems());
		}
	}

	#pragma mark -
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsActive()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FormatList::IsActive(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyFormatDescription:
				answer = true;
				break;
			
			// The available format descriptions property is active for streams whose kCMIOStreamPropertyFormatDescription property is settable
			case kCMIOStreamPropertyFormatDescriptions:
				answer = mCurrentFormatIsSettable;
				break;

			// The still image properties are only available if the owning stream has explicitly activated them.
			// This is because some streams might lack still image support
			case kCMIOStreamPropertyStillImage:
			case kCMIOStreamPropertyStillImageFormatDescriptions:
				answer = mStillImagePropertiesActive;
				break;
			
			// The video frame rate properties are only available if the owning stream has explicitly activated them.
			// This is because some streams might lack video (e.g., an only stream) or might defer determining its associated properties (e.g., a muxed DV or MPEG2 stream)
			case kCMIOStreamPropertyFrameRate:
			case kCMIOStreamPropertyFrameRates:
				answer = mFrameRatePropertiesActive;
				break;

			// The minimum frame rate properties are only available if the owning stream has explicitly activated them.
			// This is because some streams might lack the ability to guarantee a mininum frame rate
			case kCMIOStreamPropertyMinimumFrameRate:
				answer = mMinimumFrameRatePropertiesActive;
				break;
			
			case kCMIOStreamPropertyPreferredFormatDescription:
				answer = mCurrentFormatIsSettable and mPreferredFormatIsActive;
				break;
			
			case kCMIOStreamPropertyPreferredFrameRate:
				answer = mFrameRatePropertiesActive and mPreferredFrameRateIsActive;
				break;

			// The frame rate range property is only available if the owning stream has explicitly activated them.
			// This is because some streams might lack the ability to guarantee a mininum frame rate
			case kCMIOStreamPropertyFrameRateRanges:
				answer = mFrameRateRangesPropertiesActive;
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IsPropertySettable()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool FormatList::IsPropertySettable(const CMIOObjectPropertyAddress& address) const
	{
		bool answer = false;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyFormatDescription:
				answer = mCurrentFormatIsSettable;
				break;
				
			case kCMIOStreamPropertyFormatDescriptions:
				answer = false;
				break;

			case kCMIOStreamPropertyStillImage:
				answer = false;
				break;
				
			case kCMIOStreamPropertyStillImageFormatDescriptions:
				answer = false;
				break;

			case kCMIOStreamPropertyFrameRate:
				answer = true;
				break;
				
			case kCMIOStreamPropertyFrameRates:
				answer = false;
				break;
			
			case kCMIOStreamPropertyMinimumFrameRate:
				answer = true;
				break;
				
			case kCMIOStreamPropertyFrameRateRanges:
				answer = false;
				break;

			case kCMIOStreamPropertyPreferredFormatDescription:
				answer = mCurrentFormatIsSettable and mPreferredFormatIsActive;
				break;
			
			case kCMIOStreamPropertyPreferredFrameRate:
				answer = mPreferredFrameRateIsActive;
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyDataSize()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const
	{
		UInt32 answer = 0;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyFormatDescription:
			case kCMIOStreamPropertyPreferredFormatDescription:
				answer = sizeof(CMFormatDescriptionRef);
				break;
				
			case kCMIOStreamPropertyFormatDescriptions:
				answer = sizeof(CFArrayRef);
				break;

			case kCMIOStreamPropertyStillImage:
				answer = sizeof(CMSampleBufferRef);
				break;
				
			case kCMIOStreamPropertyStillImageFormatDescriptions:
				answer = sizeof(CFArrayRef);
				break;

			case kCMIOStreamPropertyFrameRate:
			case kCMIOStreamPropertyPreferredFrameRate:
				answer = sizeof(Float64);
				break;
				
			case kCMIOStreamPropertyFrameRates:
				{
					CMFormatDescriptionRef format = (0 == qualifierDataSize) ? mCurrentFormat.Get() : *static_cast<CMFormatDescriptionRef*>(const_cast<void*>(qualifierData));
					answer = GetNumberFrameRates(format) * sizeof(Float64);
				}
				break;
				
			case kCMIOStreamPropertyMinimumFrameRate:
				answer = sizeof(Float64);
				break;

			case kCMIOStreamPropertyFrameRateRanges:
				{
					CMFormatDescriptionRef format = (0 == qualifierDataSize) ? mCurrentFormat.Get() : *static_cast<CMFormatDescriptionRef*>(const_cast<void*>(qualifierData));
					answer = GetNumberFrameRateRanges(format) * sizeof(AudioValueRange);
				}
				break;
		};
		
		return answer;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const
	{
		Float64* rates;
		AudioValueRange* rateRanges;
		UInt32 theNumberRates;
		UInt32 theNumberRateRanges;
		UInt32 theIndex;
		CMFormatDescriptionRef format = NULL;
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyFormatDescription:
				ThrowIf(dataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyFormatDescription");
				if (NULL != GetCurrentFormat()) CFRetain(GetCurrentFormat());
				*static_cast<CMFormatDescriptionRef*>(data) = GetCurrentFormat();
				dataUsed = sizeof(CMFormatDescriptionRef);
				break;
				
			case kCMIOStreamPropertyFormatDescriptions:
				ThrowIf(dataSize != sizeof(CFArrayRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyFormatDescriptions");
				*static_cast<CFArrayRef*>(data) = CFArrayCreateCopy(NULL, mDescriptions.GetCFArray());
				dataUsed = sizeof(CFArrayRef);
				break;

			case kCMIOStreamPropertyStillImage:
				ThrowIf(dataSize != sizeof(CMSampleBufferRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyStillImage");
				ThrowIf(qualifierDataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong qualifier data size for kCMIOStreamPropertyStillImage");
				*static_cast<CMSampleBufferRef*>(data) = GetStillImage(*static_cast<CMFormatDescriptionRef*>(const_cast<void*>(qualifierData)));
				dataUsed = sizeof(CMFormatDescriptionRef);
				break;
				
			case kCMIOStreamPropertyStillImageFormatDescriptions:
				ThrowIf(dataSize != sizeof(CFArrayRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyStillImageFormatDescriptions");
				*static_cast<CFArrayRef*>(data) = CFArrayCreateCopy(NULL, mStillImageDescriptions.GetCFArray());
				dataUsed = sizeof(CFArrayRef);
				break;

			case kCMIOStreamPropertyFrameRate:
				ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyFrameRate");
				*(static_cast<Float64*>(data)) = GetCurrentFrameRate();
				dataUsed = sizeof(Float64);
				break;
				
			case kCMIOStreamPropertyFrameRates:
				ThrowIf(qualifierDataSize != 0 and qualifierDataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong qualifier data size for kCMIOStreamPropertyFrameRates");
				format = (0 == qualifierDataSize) ? mCurrentFormat.Get() : *static_cast<CMFormatDescriptionRef*>(const_cast<void*>(qualifierData));
				theNumberRates = std::min((UInt32)(dataSize / sizeof(Float64)), GetNumberFrameRates(format));
				rates = static_cast<Float64*>(data);
				for(theIndex = 0; theIndex < theNumberRates; ++theIndex)
				{
					rates[theIndex] = GetFrameRateByIndex(format, theIndex);
				}
				dataUsed = theNumberRates * sizeof(Float64);
				break;
			
			case kCMIOStreamPropertyMinimumFrameRate:
				ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyMinimumFrameRate");
				*(static_cast<Float64*>(data)) = GetMinimumFrameRate();
				dataUsed = sizeof(Float64);
				break;
				
			case kCMIOStreamPropertyFrameRateRanges:
				ThrowIf(qualifierDataSize != 0 and qualifierDataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong qualifier data size for kCMIOStreamPropertyFrameRateRanges");
				format = (0 == qualifierDataSize) ? mCurrentFormat.Get() : *static_cast<CMFormatDescriptionRef*>(const_cast<void*>(qualifierData));
				theNumberRateRanges = std::min((UInt32)(dataSize / sizeof(AudioValueRange)), GetNumberFrameRateRanges(format));
				rateRanges = static_cast<AudioValueRange*>(data);
				for(theIndex = 0; theIndex < theNumberRateRanges; ++theIndex)
				{
					rateRanges[theIndex] = GetFrameRateRangeByIndex(format, theIndex);
				}
				dataUsed = theNumberRateRanges * sizeof(AudioValueRange);
				break;
				
			case kCMIOStreamPropertyPreferredFormatDescription:
				ThrowIf(dataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyPreferredFormatDescription");
				if (NULL != GetPreferredFormat()) CFRetain(GetPreferredFormat());
				*static_cast<CMFormatDescriptionRef*>(data) = GetPreferredFormat();
				dataUsed = sizeof(CMFormatDescriptionRef);
				break;

			case kCMIOStreamPropertyPreferredFrameRate:
				ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::GetPropertyData: wrong data size for kCMIOStreamPropertyPreferredFrameRate");
				*(static_cast<Float64*>(data)) = GetPreferredFrameRate();
				dataUsed = sizeof(Float64);
				break;
				
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SetPropertyData()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 /*qualifierDataSize*/, const void* /*qualifierData*/, UInt32 dataSize, const void* data)
	{
		CMFormatDescriptionRef newFormat;
		const CMFormatDescriptionRef* formatDataPtr = static_cast<const CMFormatDescriptionRef*>(data);
		
		switch (address.mSelector)
		{
			case kCMIOStreamPropertyFormatDescription:
			
				ThrowIf(dataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::SetPropertyData: wrong data size for kCMIOStreamPropertyFormatDescription");
				
				// Make a mutable copy of the new format
				newFormat = *formatDataPtr;
				
				// Screen the format
				ThrowIf(not SupportsFormatDescription(newFormat), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::SetPropertyData: given format is not supported for kCMIOStreamPropertyFormatDescription");
					
				// Set the new format
				SetCurrentFormat(newFormat, true);
				break;
		
			case kCMIOStreamPropertyFrameRate:
				{
					ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::SetPropertyData: wrong data size for kCMIOStreamPropertyFrameRate");
					
					// Get the new sample rate
					Float64 theFrameRate = *(static_cast<const Float64*>(data));
					
					// Screen the sample rate
					ThrowIf(not SupportsFrameRate(theFrameRate), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::SetPropertyData: given frame rate is not supported for kCMIOStreamPropertyFrameRate");
						
					SetCurrentFrameRate(theFrameRate, true);
				}
				break;
		
			case kCMIOStreamPropertyMinimumFrameRate:
				{
					ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::SetPropertyData: wrong data size for kCMIOStreamPropertyMinimumFrameRate");
					
					// Get the new sample rate
					Float64 theFrameRate = *(static_cast<const Float64*>(data));
					
					// Screen the sample rate
					ThrowIf(not SupportsFrameRate(theFrameRate), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::SetPropertyData: given frame rate is not supported for kCMIOStreamPropertyMinimumFrameRate");
						
					SetMinimumFrameRate(theFrameRate, true);
				}
				break;
		
			case kCMIOStreamPropertyPreferredFormatDescription:
			
				ThrowIf(dataSize != sizeof(CMFormatDescriptionRef), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::SetPropertyData: wrong data size for kCMIOStreamPropertyPreferredFormatDescription");
				
				// Make a mutable copy of the new format
				newFormat = *formatDataPtr;
				
				// Screen the format
				if (newFormat)
					ThrowIf(not SupportsFormatDescription(newFormat), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::SetPropertyData: given format is not supported for kCMIOStreamPropertyPreferredFormatDescription");
					
				// Set the new format
				SetPreferredFormat(newFormat, true);
				break;
		
			case kCMIOStreamPropertyPreferredFrameRate:
				{
					ThrowIf(dataSize != sizeof(Float64), CAException(kCMIOHardwareBadPropertySizeError), "CMIO::DP::FormatList::SetPropertyData: wrong data size for kCMIOStreamPropertyPreferredFrameRate");
					
					// Get the new sample rate
					Float64 theFrameRate = *(static_cast<const Float64*>(data));
					
					// Screen the sample rate
					if (0.0 < theFrameRate)
						ThrowIf(not SupportsPreferredFrameRate(theFrameRate), CAException(kCMIODeviceUnsupportedFormatError), "CMIO::DP::FormatList::SetPropertyData: given frame rate is not supported for kCMIOStreamPropertyPreferredFrameRate");
						
					SetPreferredFrameRate(theFrameRate, true);
				}
				break;
		};
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetNumberAddressesImplemented()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	UInt32 FormatList::GetNumberAddressesImplemented() const
	{
		return 10;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GetImplementedAddressByIndex()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const
	{
		switch (index)
		{
			// Device properties
			case 0:
				address.mSelector = kCMIOStreamPropertyFormatDescription;
				break;
				
			case 1:
				address.mSelector = kCMIOStreamPropertyFormatDescriptions;
				break;

			case 2:
				address.mSelector = kCMIOStreamPropertyStillImage;
				break;
				
			case 3:
				address.mSelector = kCMIOStreamPropertyStillImageFormatDescriptions;
				break;

			case 4:
				address.mSelector = kCMIOStreamPropertyFrameRate;
				break;
				
			case 5:
				address.mSelector = kCMIOStreamPropertyFrameRates;
				break;
				
			case 6:
				address.mSelector = kCMIOStreamPropertyMinimumFrameRate;
				break;
				
			case 7:
				address.mSelector = kCMIOStreamPropertyFrameRateRanges;
				break;
				
			case 8:
				address.mSelector = kCMIOStreamPropertyPreferredFormatDescription;
				break;

			case 9:
				address.mSelector = kCMIOStreamPropertyPreferredFrameRate;
				break;
		};

		address.mScope = kCMIOObjectPropertyScopeWildcard;
		address.mElement = kCMIOObjectPropertyElementWildcard;
	}

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// DetermineNotifications()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FormatList::DetermineNotifications(const Stream& stream, CMFormatDescriptionRef oldFormat, CMFormatDescriptionRef newFormat, PropertyAddressList& streamNotifications)
	{
		if (oldFormat != newFormat)
		{
			// Figure out what aspects of the format have changed
			bool formatChanged = true;
			
			// Figure out the addresses of the various properties that change
			CMIOObjectPropertyAddress address;

			if (formatChanged)
			{
				// When the format changes, the following properties change:
				//	kCMIOStreamPropertyFormatDescription
				
				// The stream notifications
				address.mSelector = kCMIOStreamPropertyFormatDescription;
				address.mScope = kCMIOObjectPropertyScopeGlobal;
				address.mElement = kCMIOObjectPropertyElementMaster;
				streamNotifications.AppendUniqueItem(address);
			}
			
//			if (stillImageChanged)
//			{}
//			
//			if (sampleRateChanged)
//			{}
//			
//			if (numberChannelsChanged)
//			{}
		}
	}

}}}
