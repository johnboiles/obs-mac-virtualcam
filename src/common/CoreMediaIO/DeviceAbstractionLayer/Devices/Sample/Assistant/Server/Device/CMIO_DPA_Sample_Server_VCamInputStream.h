/*
	    File: CMIO_DPA_Sample_Server_VCamInputStream.h
	Abstract: n/a
	 Version: 1.2
	
*/

#if !defined(__CMIO_DPA_Sample_Server_VCamInputStream_h__)
#define __CMIO_DPA_Sample_Server_VCamInputStream_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "CMIO_DPA_Sample_Server_Stream.h"


namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// VCamInputStream
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    class VCamInputStream: public Stream
	{
	// Construction/Destruction
    public:
                                        VCamInputStream(Device* device, CFDictionaryRef streamDictionary, CMIOObjectPropertyScope scope);

	private:
		VCamInputStream&				operator=(VCamInputStream& that); // Don't allow copying

    // Format & Frame Rate
    protected:
        void							SetStreamFormat(IOVideoStreamDescription *newStreamFormat) override;

	// Management
	public:
        virtual void                    Start(Client client, mach_port_t messagePort, UInt32 initialDiscontinuityFlags) override;
        virtual void                    Stop(Client client) override;

        void							FrameArrived(size_t frameSize, uint8_t* frameData, UInt64 vbiTime);
	};
    
}}}}
#endif
