/*
	    File: CMIO_IOSA_Assistance.h
	Abstract: The IOStream Assisistant (IOSA) consists of various objects to facilitate using the "Acquisition is Initialization" design pattern.
				NOTE: the IOSA is sparsely implemented, meaning that it does not attempt to provide wrappers/access to all IOStream features, but just those needed by the IOStream plugin.
	
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

#if !defined(__CMIO_IOSA_Assistance_h__)
#define __CMIO_IOSA_Assistance_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CA Public Utility Includes
#include "CMIODebugMacros.h"
#include "CAException.h"

// System Includes
#include <IOKit/stream/IOStreamLib.h>

namespace CMIO { namespace IOSA
{

    IOCFPlugInInterface** 			AllocatePlugIn(io_service_t service);
    IOStreamRef                     AllocateStream(IOCFPlugInInterface** plugIn);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // PlugIn
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    class PlugIn
    {
	// Construction/Destruction
	public:
		explicit				PlugIn(IOCFPlugInInterface** plugIn) : mPlugin(NULL) { Reset(plugIn); }
								~PlugIn() { Reset(NULL); }
		void					Reset(IOCFPlugInInterface** plugIn = NULL) { if (NULL != mPlugin) IODestroyPlugInInterface(mPlugin); mPlugin = plugIn; }
        
	private:
		PlugIn&					operator=(PlugIn& that); // Don't allow copying
		IOCFPlugInInterface**	mPlugin;
		
	// Value Access
	public:
		operator				IOCFPlugInInterface**()	const { return mPlugin; }
		IOCFPlugInInterface**	Get() const { return mPlugin; }													
    };
    
    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Stream
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	class Stream
	{
	// Construction/Destruction
	public:
					Stream(IOStreamRef stream) : mStream(NULL), mOpened(false) { Reset(stream); }
					~Stream() { Reset(); }
		void		Reset(IOStreamRef stream = NULL, CFRunLoopRef runLoop = NULL)
					{
						if (NULL != mStream)
						{
							Close();
							(**mStream).Release(mStream);
							mStream = NULL;
						} 
						
						if (NULL != stream)
						{
							mStream = stream;
						}
					}
        
	private:
		Stream&		operator=(Stream& that); // Don't allow copying
		IOStreamRef	mStream;
		bool		mOpened;
		
	// Attributes
	public:
		bool		IsValid() const { return NULL != mStream; }

   // Operations
	public:
		void		Open() { IOReturn ioReturn = (**mStream).Open(mStream, 0); ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::IOSA::Stream::Open() failed"); mOpened = true; }
		void		Close() { if (not mOpened) return; IOReturn ioReturn = (**mStream).Close(mStream); DebugMessageIfError(ioReturn, "CMIO::IOSA::Stream::Close() failed"); mOpened = false; }
         void		AddToRunLoop(CFRunLoopRef runLoop) { IOReturn ioReturn = (**mStream).AddToRunLoop(mStream, runLoop); ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::IOSA::Stream::AddToRunLoop() failed"); }
        void		RemoveFromRunLoop(CFRunLoopRef runLoop) { IOReturn ioReturn = (**mStream).RemoveFromRunLoop(mStream, runLoop); ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::IOSA::Stream::RemoveFromRunLoop() failed"); }
        void		SetOutputCallback(IOStreamOutputCallback outputCallback, void* context) { IOReturn ioReturn = (**mStream).SetOutputCallback(mStream, outputCallback, context); ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::IOSA::Stream::SetOutputCallback() failed"); }
		void		Start() { IOReturn ioReturn = (**mStream).StartStream(mStream); ThrowIfKernelError(ioReturn, CAException(ioReturn), "CMIO::IOSA::Stream::Start() failed"); }
		void		Stop() { IOReturn ioReturn = (**mStream).StopStream(mStream); DebugMessageIfError(ioReturn, "CMIO::IOSA::Stream::Stop() failed"); }
		void*		GetDataBuffer(IOStreamBufferID bufferID) { return (**mStream).GetDataBuffer(mStream, bufferID); }
		void*		GetControlBuffer(IOStreamBufferID bufferID) { return (**mStream).GetControlBuffer(mStream, bufferID); }

	// Value Access
	public:
		operator	IOStreamRef() const { return mStream; }
		IOStreamRef	Get() const { return mStream; }													
		bool		Opened() const { return mOpened; }													
	};
}}

#endif
