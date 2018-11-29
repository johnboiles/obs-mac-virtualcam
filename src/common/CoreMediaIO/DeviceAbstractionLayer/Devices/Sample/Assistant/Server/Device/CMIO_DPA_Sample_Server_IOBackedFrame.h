/*
	    File: CMIO_DPA_Sample_Server_IOBackedFrame.h
	Abstract: n/a
	 Version: 1.2
	
*/

#if !defined(__CMIO_DPA_Sample_Server_IOBackedFrame_h__)
#define __CMIO_DPA_Sample_Server_IOBackedFrame_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "CMIO_DPA_Sample_Server_Frame.h"

// System Includes
#include <IOKit/stream/IOStreamLib.h>
#include <CoreMediaIO/CMIOSampleBuffer.h>

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IOBackedFrame
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    class IOBackedFrame: public Frame
	{		
	public:
	// Construction/Destruction
        IOBackedFrame(IOStreamRef stream,
                      FrameType frameType,
                      UInt64 hostTime,
                      const CMA::SampleBuffer::TimingInfo& timingInfo,
                      UInt32 discontinuityFlags,
                      UInt32 droppedFrameCount,
                      UInt64 firstFrameTime,
                      IOStreamBufferID bufferID,
                      size_t size,
                      void* data);
        ~IOBackedFrame();
		
	private:
		IOStreamRef						mStream;
		IOBackedFrame&					operator=(IOBackedFrame& that);							// Unimplemented - don't allow copying

	protected:
		IOStreamBufferID				mBufferID;
	};
}}}}

#endif
