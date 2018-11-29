//
//  CMIO_DP_SampleVCam_PlugInInterface.cpp
//  Sample Plugin
//
//  Created by Tamás Lustyik on 2018. 11. 28..
//

#include "CMIO_DP_SampleVCam_PlugIn.h"

// Public Utility Includes
#include "CMIODebugMacros.h"

// CA Public Utility Includes
#include "CAAutoDisposer.h"
#include "CAException.h"
#include "CACFString.h"

// System Includes
#include <CoreMediaIO/CMIOHardwarePlugin.h>
#include <IOKit/IOMessage.h>
#include <servers/bootstrap.h>


extern "C"
{
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// AppleCMIODPSampleVCamPlugIn()
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void* AppleCMIODPSampleVCamPlugIn(CFAllocatorRef allocator, CFUUIDRef requestedTypeUUID);
	void* AppleCMIODPSampleVCamPlugIn(CFAllocatorRef allocator, CFUUIDRef requestedTypeUUID)
	{
		if (not CFEqual(requestedTypeUUID, kCMIOHardwarePlugInTypeID))
			return 0;
		
		try
		{
			// Before going any further, make sure the SampleAssistant process is registerred with Mach's bootstrap service.  Normally, this would be done by having an appropriately
			// configured plist in /Library/LaunchDaemons, but if that is done then the process will be owned by root, thus complicating the debugging process.  Therefore, in the event that the
			// plist is missing (as would be the case for most debugging efforts) attempt to register the SampleAssistant now.  It will fail gracefully if allready registered.
			mach_port_t assistantServicePort;
			name_t assistantServiceName = "com.apple.cmio.DPA.SampleVCam";
			kern_return_t err = bootstrap_look_up(bootstrap_port, assistantServiceName, &assistantServicePort);
			if (BOOTSTRAP_SUCCESS != err)
			{
				// Create an URL to SampleAssistant that resides at "/Library/CoreMediaIO/Plug-Ins/DAL/Sample.plugin/Contents/Resources/SampleAssistant"
				CACFURL assistantURL(CFURLCreateWithFileSystemPath(NULL, CFSTR("/Library/CoreMediaIO/Plug-Ins/DAL/SampleVCam.plugin/Contents/Resources/SampleVCamAssistant"), kCFURLPOSIXPathStyle, false));
				ThrowIf(not assistantURL.IsValid(), CAException(-1), "AppleCMIODPSampleVCamPlugIn: unable to create URL for the SampleVCamAssistant");

				// Get the maximum size of the of the file system representation of the SampleAssistant's absolute path
				CFIndex length = CFStringGetMaximumSizeOfFileSystemRepresentation(CACFString(CFURLCopyFileSystemPath(CACFURL(CFURLCopyAbsoluteURL(assistantURL.GetCFObject())).GetCFObject(), kCFURLPOSIXPathStyle)).GetCFString());

				// Get the file system representation
				CAAutoFree<char> path(length);
				(void) CFURLGetFileSystemRepresentation(assistantURL.GetCFObject(), true, reinterpret_cast<UInt8*>(path.get()), length);

				mach_port_t assistantServerPort;
				err = bootstrap_create_server(bootstrap_port, path, getuid(), true, &assistantServerPort);
				ThrowIf(BOOTSTRAP_SUCCESS != err, CAException(err), "AppleCMIODPSampleVCamPlugIn: couldn't create server");
				
				err = bootstrap_check_in(assistantServerPort, assistantServiceName, &assistantServicePort);

				// The server port is no longer needed so get rid of it
				(void) mach_port_deallocate(mach_task_self(), assistantServerPort);

				// Make sure the call to bootstrap_create_service() succeeded
				ThrowIf(BOOTSTRAP_SUCCESS != err, CAException(err), "AppleCMIODPSampleVCamPlugIn: couldn't create SampleVCamAssistant service port");
			}

			// The service port is not longer needed so get rid of it
			(void) mach_port_deallocate(mach_task_self(), assistantServicePort);


			CMIO::DP::Sample::PlugIn* plugIn = new CMIO::DP::Sample::PlugIn(requestedTypeUUID);
			plugIn->Retain();
			return plugIn->GetInterface();
		}
		catch (...)
		{
			return NULL;
		}
	}
}

