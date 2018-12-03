/*
	    File: CMIO_DPA_Sample_Server_VCamDevice.h
	Abstract: n/a
	 Version: 1.2
	
*/

#if !defined(__CMIO_DPA_Sample_Server_VCamDevice_h__)
#define __CMIO_DPA_Sample_Server_VCamDevice_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "CMIO_DPA_Sample_Server_Device.h"

namespace CMIO { namespace DPA { namespace Sample { namespace Server
{
    class VCamInputStream;
    
    class VCamDevice: public Device
	{
	public:

    #pragma mark -
	#pragma mark Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Device
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Construction/Destruction
	public:
									VCamDevice();
		virtual						~VCamDevice();
	
	private:
		VCamDevice&					operator=(VCamDevice& that); // Don't allow copying
        
    private:
        VCamInputStream*            mInputStream;
        void						CreateStreams();
        
    private:
        pthread_t                   mThread;
        FILE*                       mSequenceFile;
        size_t                      mFrameIndex;
        size_t                      mFrameSize;
        size_t                      mFrameCount;
        
        static void*                EmitFrame(void*);
	};
}}}}
#endif
