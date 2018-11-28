/*
	    File: CMIODebugMacros.h
	Abstract: Debug macros for CMIO and variations of some CADebugMacros.h to extend or correct some of them:

		* Extended
		DebugPrint()
		DebugMessage()
		ThrowIfKernelError()

		* Added
		DebugMessageIf()
		DebugMessageLevelIf()
		DebugMessageIfError()
		DebugMessageLevelIfError()

		* Correct
		FailIfError()

		*** IMPORTANT NOTE : one cannot use both CMIODebugMacros.cpp and CADebugMacros.cpp at the same time, because they have identical function definitions, with the difference
		being the inclusion of a version on DebugPrint().
	
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

#if !defined(__CMIO_Debug__)
#define __CMIO_Debug__

#include "CADebugMacros.h"
#include <CoreServices/CoreServices.h>
#include <mach/mach_error.h>

#if !defined(USE_DTRACE_LOGGING)
    #define USE_DTRACE_LOGGING			0
#endif

#if USE_DTRACE_LOGGING
	#include <CoreMediaIO/CMIOGlobalDTrace.h>
#endif

#if !defined(ALWAYS_DO_CMIO_DEBUG_MSG)
    #define ALWAYS_DO_CMIO_DEBUG_MSG		1
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if USE_DTRACE_LOGGING
	// CMIODebug()
	//	Reports true if the DTrace probe com_apple_cmio_CMIODebugMacros is currently enabled.
	//	This MUST be true for any module specific DTracing to be active.
	Boolean CMIODebug(void);
#else
	// CMIODebug()
	//	Reports true if 'defaults write com.apple.cmio Debug true' has been set via Terminal in the current user account prior to running the application.
	//	This MUST be true for any module specific debugging to be logged.
	Boolean CMIODebug(void);
 #endif

// CMIOModuleDebugLevel()
//	Reports true if 'defaults write com.apple.cmio {moduleKey} {levelValue}' has been set via terminal prior to running the application and true == CMIODebug() and debugLevel <= levelValue.
//	There are two special values that can be used for {levelValue} :
//		-1 == always report true
//		 0 == never report true, 
Boolean CMIOModuleDebugLevel(CFStringRef moduleKey, CFIndex debugLevel);


// CMIOModuleDebugLevelExclusive()
//  Reports true if 'defaults write com.apple.cmio {moduleKey} {levelValue}' has been set via terminal prior to running the application and true == CMIODebug() and debugLevel == levelValue.
Boolean CMIOModuleDebugLevelExclusive(CFStringRef moduleKey, CFIndex debugLevel);

// CMIOModuleDebug()
//	A convenience version of CMIOModuleDebugLevel() where 1 == debugLevel
#define CMIOModuleDebug(moduleKey)	CMIOModuleDebugLevel(moduleKey, 1)


// CMIOPreferencesGetBooleanValue()
//  Allow direct access to a boolean value in the cmio prefs (com.apple.cmio.plist).
Boolean CMIOPreferencesGetBooleanValue(CFStringRef key, Boolean *keyExistsAndHasValidFormat);


// CMIOPreferencesGetIntegerValue()
//  Allow direct access to an integer value in the cmio prefs (com.apple.cmio.plist).
CFIndex CMIOPreferencesGetIntegerValue(CFStringRef key, Boolean *keyExistsAndHasValidFormat);

// CMIOPreferencesGetFloat32Value()
//  Allow direct access to an Float32 value in the cmio prefs (com.apple.cmio.plist).
	Float32 CMIOPreferencesGetFloat32Value(CFStringRef key, Boolean *keyExistsAndHasValidFormat);

// CMIOPreferencesGetDictionary()
//  Allow direct access to an integer value in the cmio prefs (com.apple.cmio.plist).
CFDictionaryRef CMIOPreferencesGetDictionary(CFStringRef key, Boolean *keyExistsAndHasValidFormat);




// CMIOFILE()
//	A helper which just returns the leaf name from the standard __FILE__ macro
const char * CMIOFILE(const char * inStr);

// CMIOAsctime()
//	A helper which removes the trailing \n from the asctime() result
char * CMIOAsctime(void);

#if defined(__cplusplus)
}
#endif

//
// Logging macros that ALWAYS log, irrespective of the DEBUG state
//
#include <syslog.h>

#if USE_DTRACE_LOGGING
	#define cmio_syslog(...)	do {char tempstr[1024]; snprintf(tempstr, 1024, __VA_ARGS__); syslog(LOG_NOTICE, "%s %s:%d:%s %s", CMIOAsctime(), CMIOFILE(__FILE__), __LINE__, __func__, tempstr); } while (0)
	#define cmio_msg(...)	do {char tempstr[1024], tempstr2[1280]; snprintf(tempstr, 1024, __VA_ARGS__); snprintf(tempstr2, 1280, "%s %s:%d:%s %s\n", CMIOAsctime(), CMIOFILE(__FILE__), __LINE__, __func__, tempstr); CMIOGlobalDTraceFireProbe(tempstr2); } while (0)
#else
	#define cmio_syslog(...)	do {char tempstr[1024]; snprintf(tempstr, 1024, __VA_ARGS__); syslog(LOG_NOTICE, "%s %s:%d:%s %s", CMIOAsctime(), CMIOFILE(__FILE__), __LINE__, __func__, tempstr); } while (0)
	#define cmio_msg(...)	do {char tempstr[1024]; snprintf(tempstr, 1024, __VA_ARGS__); fprintf(stderr, "%s %s:%d:%s %s\n", CMIOAsctime(), CMIOFILE(__FILE__), __LINE__, __func__, tempstr); } while (0)
#endif

//
// Logging / assert macros that log / assert when debugging, or melt away to nothing otherwise
//
#if (DEBUG || ALWAYS_DO_CMIO_DEBUG_MSG)

	#define cmio_debug_syslog(...)	cmio_syslog(__VA_ARGS__)	
	#define cmio_debug_msg(...)		cmio_msg(__VA_ARGS__)
	
	#include <assert.h>
	#define ASSERT(x)				assert(x)
	
#else

	#define cmio_debug_syslog(...)
	#define cmio_debug_msg(...)
	
	#define ASSERT(x)
	
#endif


//
// Flow control macros
//	Note that these macros have hardcoded variable names & goto labels so their usage is not appropriate under all cirumstances
//
#define BAILSETERR(x)		do {err = (x); if (err) { cmio_debug_msg("### Err %d", (int)err); goto bail; }} while (0)
#define BAILIFTRUE(x, code)	do {if ((x)) { err = code; if (err) { cmio_debug_msg("### Err %d", (int)err); } goto bail; }} while (0)
#define BAILERR(x)			do {OSStatus tErr = (x); if (tErr) { cmio_debug_msg("### Err %d", (int)tErr); goto bail; } } while (0)


//
// Log-on-error (when debugging) macros
//
#define DEBUGERR(x)			do {OSStatus tErr = (x); if (tErr) { cmio_debug_msg("### Err %d", (int)tErr); }} while (0)



//
// Overrides of CADebugMacros
//
#if (DEBUG || ALWAYS_DO_CMIO_DEBUG_MSG)

	// If no module key has been defined, then have all the messages dump into a default module
	#if !defined(kCMIOModuleKey)
		#define kCMIOModuleKey	"CMIO_DefaultModule.Debug"
	#endif

	// CMIO debugging print routines
	#include "CMIODebugPrintf.h"
	
	// DebugPrint()
	//	Logs a message to the indicated file if true == CMIOModuleDebugLevel(moduleKey, debugLevel).  Used by CMIODebugPrintf.h's "DebugPrintfRtn" macro.
	void DebugPrint(FILE* file, CFStringRef moduleKey, CFIndex debugLevel, const char *message, ...);

	// DebugSysLogPrint()
	// Invokes syslog() with the indicated log level if true == CMIOModuleDebugLevel(moduleKey, debugLevel).  Used by CMIODebugPrintf.h's "DebugPrintfRtn" macro.
	void DebugSysLogPrint(int logLevel, CFStringRef moduleKey, CFIndex debugLevel, const char *message, ...);

	// Undefine & redefine (verbatim) CA's "FlushRtn" macro definition might have used "DebugPrintfFile" which CMIODebugPrintf.h has redefined
	#if	(CoreAudio_FlushDebugMessages && !CoreAudio_UseSysLog) || defined(CoreAudio_UseSideFile)
			#define	FlushRtn	;fflush(DebugPrintfFile)
		#else
			#define	FlushRtn
	#endif

	// Undefine CA's messaging macros so they can be redefined to log only when true == CMIOModuleDebugLevel(moduleKey, debugLevel).
	// Also, C99's variable # of macro arguements will be used so N versions don't have be defined.
	#undef DebugMessage
	#undef DebugMessageN1
	#undef DebugMessageN2
	#undef DebugMessageN3
	#undef DebugMessageN4
	#undef DebugMessageN5
	#undef DebugMessageN6
	#undef DebugMessageN7
	#undef DebugMessageN8
	#undef DebugMessageN9

	// Redfine the message macros
	#define	DebugMessage(message, ...)									DebugPrintfRtn(DebugPrintfFile, CFSTR(kCMIOModuleKey), 1, message DebugPrintfLineEnding, ## __VA_ARGS__) FlushRtn
	#define DebugMessageLevel(level, message, ...)						DebugPrintfRtn(DebugPrintfFile, CFSTR(kCMIOModuleKey), level, message DebugPrintfLineEnding, ## __VA_ARGS__) FlushRtn
	#define DebugMessageN1(message, N1)									DebugMessage(message, N1)
	#define DebugMessageN2(message, N1, N2)								DebugMessage(message, N1, N2)
	#define DebugMessageN3(message, N1, N2, N3)							DebugMessage(message, N1, N2, N3)
	#define DebugMessageN4(message, N1, N2, N3, N4)						DebugMessage(message, N1, N2, N3, N4)
	#define DebugMessageN5(message, N1, N2, N3, N4, N5)					DebugMessage(message, N1, N2, N3, N4, N5)
	#define DebugMessageN6(message, N1, N2, N3, N4, N5, N6)				DebugMessage(message, N1, N2, N3, N4, N5, N6)
	#define DebugMessageN7(message, N1, N2, N3, N4, N5, N6, N7)			DebugMessage(message, N1, N2, N3, N4, N5, N6, N7)
	#define DebugMessageN8(message, N1, N2, N3, N4, N5, N6, N7, N8)		DebugMessage(message, N1, N2, N3, N4, N5, N6, N7, N8)
	#define DebugMessageN9(message, N1, N2, N3, N4, N5, N6, N7, N8, N9)	DebugMessage(message, N1, N2, N3, N4, N5, N6, N7, N8, N9)

	// Define a conditional version of DebugMessage
	#define	DebugMessageIf(condition, message, ...)			\
			{												\
				if (condition)								\
				{											\
					DebugMessage(message, ## __VA_ARGS__);	\
				}											\
			}

	// Define a conditional version of DebugMessageLevel
	#define	DebugMessageLevelIf(level, condition, message, ...)			\
			{															\
				if (condition)											\
				{														\
					DebugMessageLevel(level, message, ## __VA_ARGS__);	\
				}														\
			}


	// Define a convenience version of DebugMessageIf() which will print out the 4CC version of the error code
	#define	DebugMessageIfError(error, message, ...)												\
			{																					\
				SInt32 __Err = (error);															\
				if(__Err != 0)																	\
				{																				\
					char __4CC[5] = CA4CCToCString(__Err);										\
					DebugMessage(message ", Error: %ld (%s) ", ## __VA_ARGS__ , __Err, __4CC);	\
				}																				\
			}

	// Define a convenience version of DebugMessageLevelIf() which will print out the 4CC version of the error code
	#define	DebugMessageLevelIfError(level, error, message, ...)												\
			{																								\
				SInt32 __Err = (error);																		\
				if(__Err != 0)																				\
				{																							\
					char __4CC[5] = CA4CCToCString(__Err);													\
					DebugMessageLevel(level, message ", Error: %ld (%s) ", ## __VA_ARGS__ , __Err, __4CC);	\
				}																							\
			}

	// Undefine and re-define the "FailIfError" macro.  This is simply needed since the version in CADebugMacros.h has a typo and can be removed as soon as that is fixed
	#undef FailIfError
	#define	FailIfError(error, handler, message)								\
			{																	\
				SInt32 __Err = (error);											\
				if(__Err != 0)													\
				{																\
					char __4CC[5] = CA4CCToCString(__Err);						\
					DebugMessage(message ", Error: %ld (%s)", __Err, __4CC);	\
					STOP;														\
					goto handler;												\
				}																\
			}

	// Undefine and re-define the "ThrowIfKernelError" macro.
	// This is simply needed since the version in CADebugMacros.h doesn't print out the mach_error_string(), so this could be removed when that convenience factor is added
	#undef ThrowIfKernelError
	#define	ThrowIfKernelError(kernelError, exception, message)										\
			{																						\
				kern_return_t __Err = (kernelError);												\
				if(__Err != 0)																		\
				{																					\
					DebugMessage(message ", Error: 0x%X (%s)", __Err, mach_error_string(__Err));	\
					Throw(exception);																\
				}																					\
			}
#else

	// Not debugging, so have locally defined macros melt away.  Nothing needs to be done to have the other CA overrides melt away, since CADebugMacros.h handled that.
	#define DebugPrint(...)
	#define DebugSysLogPrint(...)
	#define DebugMessageLevel(...)
	#define DebugMessageIf(...)
	#define DebugMessageLevelIf(...)
	#define DebugMessageIfError(...)
	#define DebugMessageLevelIfError(...)
	
#endif

#endif // __CMIO_Debug__
