/*
	    File: SamplePrefix.h
	Abstract: Items used to facilitate building on Lion or Mountain Lion with the same sources.
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

#if !defined(__SamplePrefix_h__)
#define __SamplePrefix_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#include <sys/errno.h>

#pragma pack(push, 4)

#if !defined(MAC_OS_X_VERSION_10_8)
	#define MAC_OS_X_VERSION_10_8 1080
#endif

#if !defined(MAC_OS_X_VERSION_10_9)
	#define MAC_OS_X_VERSION_10_9 1090
#endif


#if (MAC_OS_X_VERSION_MAX_ALLOWED <  MAC_OS_X_VERSION_10_8)
	// Additional Controls that are defined in 10.8.
	// This definition is merely included as a convenience in order to avoid cluttering the code with conditional compilation blocks.
	// It would not be possible to create an instance of this control under 10.7.
	enum
	{
		kCMIONoiseReductionControlClassID	= 's2nr',
	};


	// Additional properties that are defined in 10.8.
	// This definition is merely included as a convenience in order to avoid cluttering the code with conditional compilation blocks.
	// No client would attempt to manipulate the property under 10.7
	enum
	{
		kCMIOStreamPropertyMinimumFrameRate	= 'mfrt',
	};

#endif

#if (MAC_OS_X_VERSION_MAX_ALLOWED <  MAC_OS_X_VERSION_10_9)
	// Additional properties
	// These definitions are merely placeholders. If this ever results in a duplicate definition error, simply delete the offending item here
	enum
	{
		kCMIOStreamPropertyOutputBuffersNeededForDroplessPlayback	= 'miff',
        kCMIOStreamPropertyPreferredFormatDescription				= 'prfd',
        kCMIOStreamPropertyPreferredFrameRate                  		= 'prfr',
		kCMIOStreamPropertyFrameRateRanges							= 'frrg',
	};

#endif

#pragma pack(pop)
#endif
