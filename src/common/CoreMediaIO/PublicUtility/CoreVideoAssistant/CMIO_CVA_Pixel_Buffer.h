/*
	    File: CMIO_CVA_Pixel_Buffer.h
	Abstract: C++ wrapper for CVPixelBufferRef
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

#if !defined(__CMIO_CVA_Pixel_Buffer_h__)
#define __CMIO_CVA_Pixel_Buffer_h__

// Super Class Include
#include "CMIO_CVA_Image_Buffer.h"

// CA Public Utility
#include "CAException.h"

// System Includes
#include <CoreVideo/CVPixelBuffer.h>

namespace CMIO { namespace CVA { namespace Pixel
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Buffer
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Buffer : public Image::Buffer
	{
	// Construction/Destruction
	public:
									Buffer(CVPixelBufferRef ref = NULL, bool retain = false, bool release = true) : Image::Buffer(ref, retain, release) {}
		virtual						~Buffer() { Reset(); }
		Buffer&						operator=(const Buffer& that) { if (this != &that) this->Reset(that.Get(), true, that.WillRelease());  return *this; }

	public:
		static CVPixelBufferRef		CreateWithBytes(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType, void *baseAddress, size_t bytesPerRow,CVPixelBufferReleaseBytesCallback releaseCallback, void *releaseRefCon, CFDictionaryRef pixelBufferAttributes)
									{
										CVPixelBufferRef pixelBufferRef;
										CVReturn err = CVPixelBufferCreateWithBytes(allocator, width, height, pixelFormatType, baseAddress,bytesPerRow, releaseCallback, releaseRefCon, pixelBufferAttributes, &pixelBufferRef);
										ThrowIfError(err, CAException(err), "CMIO::CVA::Pixel::Buffer::CreateWithBytes: CVPixelBufferCreateWithBytes() failed");
										return pixelBufferRef; 
									}
	
		static CVPixelBufferRef		Create(CFAllocatorRef allocator, size_t width, size_t height, OSType pixelFormatType, CFDictionaryRef pixelBufferAttributes)
									{
										CVPixelBufferRef pixelBufferRef;
										CVReturn err = CVPixelBufferCreate(allocator, width, height, pixelFormatType, pixelBufferAttributes, &pixelBufferRef);
										ThrowIfError(err, CAException(err), "CMIO::PixelBuffer::Create: CVPixelBufferCreate() failed");
										return pixelBufferRef; 
									}

		static CVPixelBufferRef		CreateFromIOSurface(CFAllocatorRef allocator, IOSurfaceRef surface, CFDictionaryRef pixelBufferAttributes)
									{
										CVPixelBufferRef pixelBufferRef;
										CVReturn err = CVPixelBufferCreateWithIOSurface(allocator, surface, pixelBufferAttributes, &pixelBufferRef);
										ThrowIfError(err, CAException(err), "CMIO::CVA::Pixel::Buffer::CreateFromIOSurface: CVPixelBufferCreateWithIOSurface() failed");
										return pixelBufferRef; 
									}

	// Value Access
	public:
		operator					CVPixelBufferRef() const { return Image::Buffer::Get(); }
		CVPixelBufferRef			Get() const { return Image::Buffer::Get(); }

	// Operations
	public:
		void						LockBaseAddress(CVOptionFlags lockFlags) { CVReturn err = CVPixelBufferLockBaseAddress(Get(), lockFlags); ThrowIfError(err, CAException(err), "CMIO::CVA::Pixel::Buffer::LockBaseAddres: CVPixelBufferLockBaseAddress() failed"); }
		void						UnlockBaseAddress(CVOptionFlags unlockFlags) { CVReturn err = CVPixelBufferUnlockBaseAddress(Get(), unlockFlags); ThrowIfError(err, CAException(err), "CMIO::CVA::Pixel::Buffer::UnlockBaseAddress: CVPixelBufferUnlockBaseAddress() failed"); }

	// Helper class to manage locking and then unlocking automatically when going out of scope
	public:
		class BaseAddressLocker
		{
			// Construction/Destruction
			public:
									BaseAddressLocker(Buffer& buffer, CVOptionFlags lockFlags) : mBuffer(buffer), mLockFlags(lockFlags) { mBuffer.LockBaseAddress(mLockFlags); }
									~BaseAddressLocker() { mBuffer.UnlockBaseAddress(mLockFlags); }
			
			private:
									BaseAddressLocker(const BaseAddressLocker&);
				BaseAddressLocker&	operator=(const BaseAddressLocker&);
					
			//	Implementation
			private:
				Buffer&				mBuffer;
				CVOptionFlags		mLockFlags;
		};
	};
}}}
#endif
