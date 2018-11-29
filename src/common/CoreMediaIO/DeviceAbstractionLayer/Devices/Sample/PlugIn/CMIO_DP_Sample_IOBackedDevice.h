/*
	    File: CMIO_DP_Sample_IOBackedDevice.h
	Abstract: n/a
	 Version: 1.2
		
*/

#if !defined(__CMIO_DP_Sample_IOBackedDevice_h__)
#define __CMIO_DP_Sample_IOBackedDevice_h__

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Super Class Includes
#include "CMIO_DP_Sample_Device.h"

// Public Utility Includes
#include "CMIO_IOKA_Object.h"


namespace CMIO { namespace DP { namespace Sample
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// IOBackedDevice
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    class IOBackedDevice : public Device
	{
	// Construction/Destruction
	public:
        IOBackedDevice(PlugIn& plugIn, CMIODeviceID deviceID, mach_port_t assistantPort, UInt64 guid, const io_string_t registryPath);

	protected:
		IOKA::Object mRegistryEntry;	// The IOKit registry entry for the device

    // Controls
    public:
        virtual CFDictionaryRef CopyControlDictionaryByControlID(UInt32 controlID) const override;

    protected:
        virtual void CreateRegistryControls() override;
	};
}}}

#endif
