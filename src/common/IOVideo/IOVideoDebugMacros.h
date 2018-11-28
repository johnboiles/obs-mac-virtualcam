/*
	    File: IOVideoDebugMacros.h
	Abstract: Macros
				These macros are only slightly different from the ones in CMIODebugMacros.h. We split them out here for two reasons.
				First, IOLog doesn't have a file argument. This makes it incompatible with CMIODebugPrintf.h. Second, we want to be darn sure about what code we compile into a kext.
	
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

#if !defined(__IOVideoDeviceDebugMacros_h__)
#define __IOVideoDeviceDebugMacros_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <IOKit/IOLib.h>

#if __BIG_ENDIAN__
	#define	IOVD_4CCToCString(the4CC)	{ ((char*)&the4CC)[0], ((char*)&the4CC)[1], ((char*)&the4CC)[2], ((char*)&the4CC)[3], 0 }
#else
	#define	IOVD_4CCToCString(the4CC)	{ ((char*)&the4CC)[3], ((char*)&the4CC)[2], ((char*)&the4CC)[1], ((char*)&the4CC)[0], 0 }
#endif

#if VideoDevice_Debug

#pragma mark Debug Macros

#define	DebugMessage(message, ...)	IOLog(message, ## __VA_ARGS__)

#define	Assert(inCondition, inMessage)													\
			if (!(inCondition))															\
			{																			\
				DebugMessage(inMessage);												\
			}

#define	AssertNoError(inError, inMessage)												\
			{																			\
				SInt32 __Err = (inError);												\
				if (__Err != 0)															\
				{																		\
					char __4CC[5] = IOVD_4CCToCString(__Err);								\
					DebugMessageN2(inMessage ", Error: %d (%s)", (int)__Err, __4CC);		\
				}																		\
			}

#define	AssertNoKernelError(inError, inMessage)											\
			{																			\
				unsigned int __Err = (unsigned int)(inError);							\
				if (__Err != 0)															\
				{																		\
					DebugMessageN1(inMessage ", Error: 0x%X", __Err);					\
				}																		\
			}

#define	FailIf(inCondition, inHandler, inMessage)										\
			if (inCondition)																\
			{																			\
				DebugMessage(inMessage);												\
				goto inHandler;															\
			}

#define	FailIfNULL(inPointer, inHandler, inMessage)										\
			if ((inPointer) == NULL)														\
			{																			\
				DebugMessage(inMessage);												\
				goto inHandler;															\
			}

#define	FailIfError(inError, inHandler, inMessage)										\
			{																			\
				SInt32 __Err = (inError);												\
				if (__Err != 0)															\
				{																		\
					DebugMessageN2(inMessage ", Error: %ld (0x%X)", __Err, (unsigned int)__Err);		\
					goto inHandler;														\
				}																		\
			}

#define	FailIfWithAction(inCondition, inAction, inHandler, inMessage)					\
			if (inCondition)																\
			{																			\
				DebugMessage(inMessage);												\
				{ inAction; }															\
				goto inHandler;															\
			}

#define	FailIfNULLWithAction(inPointer, inAction, inHandler, inMessage)					\
			if ((inPointer) == NULL)														\
			{																			\
				DebugMessage(inMessage);												\
				{ inAction; }															\
				goto inHandler;															\
			}

#define	FailIfErrorWithAction(inError, inAction, inHandler, inMessage)					\
			{																			\
				SInt32 __Err = (inError);												\
				if (__Err != 0)															\
				{																		\
					DebugMessageN2(inMessage ", Error: %ld (0x%X)", __Err, __Err);		\
					{ inAction; }														\
					goto inHandler;														\
				}																		\
			}

#else

#pragma mark Release Macros

#define	DebugMessage(message, ...)

#define	Assert(inCondition, inMessage)													\
			if (!(inCondition))															\
			{																			\
			}

#define	AssertNoError(inError, inMessage)												\
			{																			\
				SInt32 __Err = (inError);												\
				if (__Err != 0)															\
				{																		\
				}																		\
			}

#define	AssertNoKernelError(inError, inMessage)											\
			{																			\
				unsigned int __Err = (unsigned int)(inError);							\
				if (__Err != 0)															\
				{																		\
				}																		\
			}

#define	FailIf(inCondition, inHandler, inMessage)										\
			if (inCondition)																\
			{																			\
				goto inHandler;															\
			}

#define	FailIfNULL(inPointer, inHandler, inMessage)										\
			if ((inPointer) == NULL)														\
			{																			\
				goto inHandler;															\
			}

#define	FailIfError(inError, inHandler, inMessage)										\
			if ((inError) != 0)															\
			{																			\
				goto inHandler;															\
			}

#define	FailIfWithAction(inCondition, inAction, inHandler, inMessage)					\
			if (inCondition)																\
			{																			\
				{ inAction; }															\
				goto inHandler;															\
			}

#define	FailIfNULLWithAction(inPointer, inAction, inHandler, inMessage)					\
			if ((inPointer) == NULL)														\
			{																			\
				{ inAction; }															\
				goto inHandler;															\
			}

#define	FailIfErrorWithAction(inError, inAction, inHandler, inMessage)					\
			if ((inError) != 0)															\
			{																			\
				{ inAction; }															\
				goto inHandler;															\
			}

#endif

#endif
