/*
	    File: CMIO_DP_Property_FormatList.h
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

#if !defined(__CMIO_DP_Property_FormatList_h__)
#define __CMIO_DP_Property_FormatList_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Property_Base.h"

// Public Utility Includes
#include "CMIO_CMA_FormatDescription.h"

// CA Public Utility Includes
#include "CACFArray.h"
#include "CAAudioValueRange.h"

// System Includes
#include <CoreMedia/CMSampleBuffer.h>

// Standard Library Includes
#include <vector>
#include <map>

namespace CMIO
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in CMIO namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class PropertyAddressList;
}

namespace CMIO { namespace DP
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Types in CMIO::DP namespace
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Device;
	class Stream;
}}

namespace CMIO { namespace DP { namespace Property
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FormatList
	//  This class encapsulates the necessary smarts for implementing the format related properties in the DAL's API for CMIOStream objects.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class FormatList : public Base
	{
	private:
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// Types
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		typedef std::vector<Float64> FrameRates;
		typedef std::map<CMFormatDescriptionRef, FrameRates> FrameRatesMap;
		typedef std::vector<CAAudioValueRange> FrameRateRanges;
		typedef std::map<CMFormatDescriptionRef, FrameRateRanges> FrameRateRangeMap;
		
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// FormatList::FormatDescriptionEquals<T>
		//	A unary predicate object which reports if the FrameRateXXXXXMap.first equals the specified CMFormatDescriptionRef.
		//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		template <typename T> class FormatDescriptionEquals
		{
		public:
					FormatDescriptionEquals(CMFormatDescriptionRef formatDescription) : mFormatDescription(formatDescription) {};
			bool	operator()(const std::pair<CMFormatDescriptionRef, T>& pair) const { return CMFormatDescriptionEqual(pair.first, mFormatDescription); }

		private:
			CMFormatDescriptionRef	mFormatDescription;
		};

	#pragma mark - FormatList
	// Construction/Destruction
	public:
									FormatList(Stream* owningStream);
		virtual						~FormatList();

	// Format Operations
	public:
		void						CurrentFormatIsSettable(bool settable) { mCurrentFormatIsSettable = settable; }
		CMFormatDescriptionRef		GetCurrentFormat() const { return mCurrentFormat; }
		void						SetCurrentFormat(CMFormatDescriptionRef format, bool tellHardware);
		
		UInt32						GetNumberAvailableFormats() const;
		CMFormatDescriptionRef		GetAvailableFormatByIndex(UInt32 index) const;
		void						AddAvailableFormat(CMFormatDescriptionRef format);
		void						RemoveAllAvailableFormats();

		bool						SupportsFormatDescription(CMFormatDescriptionRef format) const;
		CMFormatDescriptionRef		BestMatchForFormat(CMFormatDescriptionRef format) const;
		UInt32						GetCurrentNumberChannels() const;

	// Still Image Operations
	public:
		void						ActivateStillImageProperties(bool activate) { mStillImagePropertiesActive = activate; }
		CMSampleBufferRef			GetStillImage(CMFormatDescriptionRef description) const;
		
		UInt32						GetNumberStillImageFormats() const;
		CMFormatDescriptionRef		GetStillImageFormatByIndex(UInt32 index) const;
		void						AddStillImageFormat(CMFormatDescriptionRef format);
		void						RemoveAllStillImageFormats();

	// Video Frame Rate Operations
	public:
		void						ActivateFrameRateProperties(bool activate) { mFrameRatePropertiesActive = activate; }
		void						ActivateMinimumFrameRateProperties(bool activate) { mMinimumFrameRatePropertiesActive = activate; }
		void						ActivateFrameRateRangesProperties(bool activate) { mFrameRateRangesPropertiesActive = activate; }

		Float64						GetCurrentFrameRate() const;
		void						SetCurrentFrameRate(Float64 frameRate, bool tellHardware);
		Float64						GetMinimumFrameRate() const;
		void						SetMinimumFrameRate(Float64 frameRate, bool tellHardware);

		UInt32						GetNumberFrameRates(CMFormatDescriptionRef format) const ;
		Float64						GetFrameRateByIndex(CMFormatDescriptionRef format, UInt32 index) const;
		void						SetFrameRates(CMFormatDescriptionRef format, UInt32 numberOfRates, const Float64 rates[]);
		bool						SupportsFrameRate(Float64 frameRate) const;
		bool						SupportsPreferredFrameRate(Float64 preferredFrameRate) const;

		UInt32						GetNumberFrameRateRanges(CMFormatDescriptionRef format) const ;
		AudioValueRange				GetFrameRateRangeByIndex(CMFormatDescriptionRef format, UInt32 index) const;
		void						SetFrameRateRanges(CMFormatDescriptionRef format, UInt32 numberOfRateRanges, const AudioValueRange rateRanges[]);

	// Preferred Format/Framerate Operations
	public:
		void						ActivatePreferredFormat(bool activate) { mPreferredFormatIsActive = activate; }
		CMFormatDescriptionRef		GetPreferredFormat() const { return mPreferredFormat; }
		void						SetPreferredFormat(CMFormatDescriptionRef inNewPreferredFormat, bool tellHardware);

		void						ActivatePreferredFrameRate(bool activate) { mPreferredFrameRateIsActive = activate; }
		Float64						GetPreferredFrameRate() const { return mPreferredFrameRate; }
		void						SetPreferredFrameRate(Float64 inNewPreferredFrameRate, bool tellHardware);

	// Property Operations
	public:
		virtual bool				IsActive(const CMIOObjectPropertyAddress& address) const;
		virtual bool				IsPropertySettable(const CMIOObjectPropertyAddress& address) const;
		virtual UInt32				GetPropertyDataSize(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData) const;
		virtual void				GetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32& dataUsed, void* data) const;
		virtual void				SetPropertyData(const CMIOObjectPropertyAddress& address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data);

		virtual UInt32				GetNumberAddressesImplemented() const;
		virtual void				GetImplementedAddressByIndex(UInt32 index, CMIOObjectPropertyAddress& address) const;
		
		
		
		static void					DetermineNotifications(const Stream& stream, CMFormatDescriptionRef oldFormat, CMFormatDescriptionRef newFormat, PropertyAddressList& streamNotifications);

	// Implementation
	private:
		Stream*						mOwningStream;
		CMA::FormatDescription		mCurrentFormat;
		CMA::FormatDescription		mPreferredFormat;
		CACFArray					mDescriptions;
		CACFArray					mStillImageDescriptions;
		bool						mCurrentFormatIsSettable;
		bool						mStillImagePropertiesActive;
		bool						mFrameRatePropertiesActive;
		bool						mMinimumFrameRatePropertiesActive;
		bool						mFrameRateRangesPropertiesActive;
		bool						mPreferredFormatIsActive;
		bool						mPreferredFrameRateIsActive;
		
		Float64						mCurrentFrameRate;
		Float64						mMinimumFrameRate;
		Float64						mPreferredFrameRate;
		FrameRatesMap				mFrameRates;
		FrameRateRangeMap			mFrameRateRanges;
		FrameRates					mAllPossibleFrameRates;

	};
}}}
#endif
