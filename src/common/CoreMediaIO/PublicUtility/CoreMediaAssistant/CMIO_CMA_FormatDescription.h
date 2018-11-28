/*
	    File: CMIO_CMA_FormatDescription.h
	Abstract: C++ wrapper for CMFormatDescriptionRef
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

#if !defined(__CMIO_CMA_FormatDescription_h__)
#define __CMIO_CMA_FormatDescription_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Public Utility
#include "CMIODebugMacros.h"

// CA Public Utility
#include "CAException.h"
#include "CACFString.h"

// System Includes
#include <CoreMedia/CMFormatDescription.h>

namespace CMIO { namespace CMA
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// FormatDescription
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	struct FormatDescription
	{
	// Construction/Destruction
	public:
										FormatDescription(CMFormatDescriptionRef ref = NULL, bool retain = true, bool release = true) : mRef(NULL)	{ Reset(ref, retain, release); }
		FormatDescription&				operator=(const FormatDescription& that) { if (this != &that) this->Reset(that.Get()) ; return *this; }
		FormatDescription&				operator=(CMFormatDescriptionRef ref) { Reset(ref); return *this; }
		void							Reset(CMFormatDescriptionRef ref = NULL, bool retain = true, bool release = true) { if (NULL != mRef and mRelease) CFRelease(mRef); if (retain and NULL != ref) CFRetain(ref); mRef = ref; mRelease = release;}
										~FormatDescription() { Reset(); }

	// Creaters of underlying CMFormatDescriptionRefs
	public:
		static CMFormatDescriptionRef	AudioFormatDescriptionCreate(CFAllocatorRef allocator, const AudioStreamBasicDescription *asbd, size_t layoutSize, const AudioChannelLayout *layout, size_t magicCookieSize, const void *magicCookie, CFDictionaryRef extensions)
		{
			CMFormatDescriptionRef format;
			OSStatus err = CMAudioFormatDescriptionCreate(allocator, asbd, layoutSize, layout, magicCookieSize, magicCookie, extensions, &format);
			ThrowIfError(err, CAException(err), "CMIO::CMA::FormatDescription::AudioFormatDescriptionCreate: CMFormatDescriptionCreateFromFormatDescription() failed");
			return format;
		}

		static CMFormatDescriptionRef	VideoFormatDescriptionCreate(CFAllocatorRef allocator, CMVideoCodecType codecType, int32_t width, int32_t height, CFDictionaryRef extensions)
		{
			CMFormatDescriptionRef format;
			OSStatus err = CMVideoFormatDescriptionCreate(allocator, codecType, width,  height, extensions, &format);
			ThrowIfError(err, CAException(err), "CMIO::CMA::FormatDescription::VideoFormatDescriptionCreate: CMVideoFormatDescriptionCreate() failed");
			return format;
		}

		static CMFormatDescriptionRef	VideoFormatDescriptionCreateForImageBuffer(CFAllocatorRef allocator, CVImageBufferRef imageBuffer)
		{
			CMFormatDescriptionRef format;
			OSStatus err = CMVideoFormatDescriptionCreateForImageBuffer(allocator, imageBuffer, &format);
			ThrowIfError(err, CAException(err), "CMIO::CMA::FormatDescription::VideoFormatDescriptionCreateForImageBuffer: CMVideoFormatDescriptionCreateForImageBuffer() failed");
			return format;
		}

		static CMFormatDescriptionRef	MuxedFormatDescriptionCreate(CFAllocatorRef allocator, FourCharCode muxType, CFDictionaryRef extensions)
		{
			CMFormatDescriptionRef format;
			OSStatus err = CMMuxedFormatDescriptionCreate(allocator, muxType, extensions, &format);
			ThrowIfError(err, CAException(err), "CMIO::CMA::FormatDescription::MuxedFormatDescriptionCreate: CMMuxedFormatDescriptionCreate() failed");
			return format;
		}

	private:
		CMFormatDescriptionRef			mRef;
		bool							mRelease;

	// Attributes
	public:
		bool							IsValid() const { return mRef != NULL; }
		bool							WillRelease() const { return mRelease; }
		void							ShouldRelease(bool release) { mRelease = release; }

	// Value Access
	public:
		operator						CMFormatDescriptionRef() const	{ return mRef; }
		CMFormatDescriptionRef			Get() const						{ return mRef; }
		CMFormatDescriptionRef*		GetAddress()					{ return &mRef; }
		
	// Operations
	public:
		CFStringRef CopyName()
		{
			CACFMutableString constructedName("", false);
			CACFString name(static_cast<CFStringRef>(CMFormatDescriptionGetExtension(mRef, kCMFormatDescriptionExtension_FormatName)), false);
			if (not name.IsValid())
			{
				// Just construct the name from the media subtype
				FourCharCode subtype = CMFormatDescriptionGetMediaSubType(mRef);
				
				if (1 <= subtype and subtype <= 40)
				{
					// The old QuickDraw values are being used for the depth
					constructedName.Append(CACFString(CFStringCreateWithFormat(NULL, NULL, CFSTR("%s %d"), subtype > 32 ? "Mono" : "RGB", subtype > 32 ? subtype - 32 : subtype)).GetCFString());
				}
				else
				{
					// Swizzle the four char code so that it will be in the proper byte order for the CFStringCreateWithBytes() call
					subtype = EndianU32_NtoB(subtype);
					constructedName.Append(CACFString(CFStringCreateWithBytes(NULL, reinterpret_cast<UInt8*>(&subtype), 4, kCFStringEncodingASCII, false)).GetCFString());
				}
			}
			else 
			{
				constructedName.Append(name.GetCFString());
			}
			
			// If the media type is video, append the dimensions to the constructed name
			if (kCMMediaType_Video == CMFormatDescriptionGetMediaType(mRef))
			{
				CMVideoDimensions videoDimensions = CMVideoFormatDescriptionGetDimensions(mRef);
				constructedName.Append(CACFString(CFStringCreateWithFormat(NULL, NULL, CFSTR(" (%d x %d)"), (int)videoDimensions.width, (int)videoDimensions.height)).GetCFString());
			}	
			
			// Return the constructed string (no need to retain it since it was created above)
			return constructedName.GetCFMutableString();
		}
		
		CFStringRef CopyNameDimensionsFirst()
		{
			CACFMutableString constructedName("", false);

			// If the media type is video, start with the dimensions to the constructed name
			if (kCMMediaType_Video == CMFormatDescriptionGetMediaType(mRef))
			{
				CMVideoDimensions videoDimensions = CMVideoFormatDescriptionGetDimensions(mRef);
				constructedName.Append(CACFString(CFStringCreateWithFormat(NULL, NULL, CFSTR("%d x %d "), (int)videoDimensions.width, (int)videoDimensions.height)).GetCFString());
			}	
			
			CACFString name(static_cast<CFStringRef>(CMFormatDescriptionGetExtension(mRef, kCMFormatDescriptionExtension_FormatName)), false);
			if (not name.IsValid())
			{
				// Just construct the name from the media subtype
				FourCharCode subtype = CMFormatDescriptionGetMediaSubType(mRef);
				
				if (1 <= subtype and subtype <= 40)
				{
					// The old QuickDraw values are being used for the depth
					constructedName.Append(CACFString(CFStringCreateWithFormat(NULL, NULL, CFSTR("%s %d"), subtype > 32 ? "Mono" : "RGB", subtype > 32 ? subtype - 32 : subtype)).GetCFString());
				}
				else
				{
					// Swizzle the four char code so that it will be in the proper byte order for the CFStringCreateWithBytes() call
					subtype = EndianU32_NtoB(subtype);
					constructedName.Append(CACFString(CFStringCreateWithBytes(NULL, reinterpret_cast<UInt8*>(&subtype), 4, kCFStringEncodingASCII, false)).GetCFString());
				}
			}
			else 
			{
				constructedName.Append(name.GetCFString());
			}
			
			// Return the constructed string (no need to retain it since it was created above)
			return constructedName.GetCFMutableString();
		}
	};
}}
#endif
