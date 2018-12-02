/*
	    File: CMIO_DPA_Sample_Server_IOBackedStream.h
	Abstract: n/a
	 Version: 1.2
	
*/

#if !defined(__CMIO_DPA_Sample_Server_IOBackedStream_h__)
#define __CMIO_DPA_Sample_Server_IOBackedStream_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "CMIO_DPA_Sample_Server_Stream.h"

// Public Utility Includes
#include "CMIO_IOKA_Object.h"
#include "CMIO_IOSA_Assistance.h"

// System Includes
#include <IOKit/stream/IOStreamLib.h>

// Standard Library Includes
#include <list>


namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IOBackedStream
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    class IOBackedStream: public Stream
	{
	// Construction/Destruction
	public:
										IOBackedStream(Device* device, IOKA::Object& registryEntry, CFDictionaryRef streamDictionary, CMIOObjectPropertyScope scope);
		virtual							~IOBackedStream();

	private:
		IOBackedStream&					operator=(Stream& that); // Don't allow copying

	protected:
		IOKA::Object					mRegistryEntry;				// The IOKit registry entry for the device

	// Attributes & Properties
	public:
		IOSA::Stream&					GetIOSAStream() { return mIOSAStream; }

	protected:
        IOSA::PlugIn					mIOSPAlugIn;				// IOSA wrapper kIOStreamLibTypeID's IOCFPlugInInterface** 
        IOSA::Stream					mIOSAStream;				// IOSA wrapper for IOVideoStreamRef

        virtual void					SetStreamFormat(IOVideoStreamDescription *newStreamFormat) override;

	// Management
	public:
        virtual void                    Start(Client client, mach_port_t messagePort, UInt32 initialDiscontinuityFlags) override;
        virtual void                    Stop(Client client) override;

        static void						StreamOutputCallback(IOStreamRef /*streamRef*/, IOBackedStream& stream);
		void							FrameArrived(IOStreamBufferQueueEntry& entry);

	protected:
		typedef std::list<IOStreamBufferQueueEntry> IOStreamBufferQueueEntryFreeList;
		IOStreamBufferQueueEntryFreeList	mFreeList;
	};
    
}}}}
#endif
