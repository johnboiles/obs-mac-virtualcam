//
//  CMIO_DPA_Sample_VCamServer.cpp
//  Sample Assistant
//
//  Created by Tamás Lustyik on 2018. 11. 30..
//

// Internal Includes
#include "CMIO_DPA_Sample_Server_VCamAssistant.h"

// MIG Server Interface
#include "CMIODPASampleServer.h"

// System Includes
#include <servers/bootstrap.h>


namespace
{
	// As a convenience, use the CMIO::DPA::Sample namespace.  This will allow convienient access in the anonymous namespace as well as in the CMIODPASampleXXX() public interface to the server
	using namespace CMIO::DPA::Sample;

	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// MessagesAndNotifications()
	//	This handles messages from the client and the MACH_NOTIFY_NO_SENDERS notification on the client ports.
	//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	boolean_t MessagesAndNotifications(mach_msg_header_t* request, mach_msg_header_t* reply)
	{
		// Invoke the MIG created CMIODPASampleServer() to see if this is one of the client messages it handles
		boolean_t processed = CMIODPASampleServer(request, reply);
		
		// If CMIODPASampleServer() did not process the message see if it is a MACH_NOTIFY_NO_SENDERS notification
		if (not processed and MACH_NOTIFY_NO_SENDERS == request->msgh_id)
		{
			Server::VCamAssistant::Instance()->ClientDied(request->msgh_local_port);
			processed = true;
		}
		
		return processed;
	}

}


#pragma mark -

// As a convenience, use the CMIO::DPA::Sample::Server namespace
using namespace CMIO::DPA::Sample::Server;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// main()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int main()
{
	// Don't allow any exceptions to escape
	try
	{
		// Check in with the bootstrap port under the agreed upon name to get the servicePort with receive rights
		mach_port_t servicePort;
		name_t serviceName = "com.apple.cmio.DPA.SampleVCam";
		kern_return_t err = bootstrap_check_in(bootstrap_port, serviceName, &servicePort);
		if (BOOTSTRAP_SUCCESS != err)
		{
			DebugMessage("bootstrap_check_in() failed: 0x%x", err);
			exit(43);
		}
	
		#if 0
			// Wait forever until the Debugger can attach to the Assistant process
			bool waiting = true;
			while (waiting)
			{
				sleep(1);
			}
		#endif

		// Add the service port to the Assistant's port set
		mach_port_t portSet = VCamAssistant::Instance()->GetPortSet();
		err = mach_port_move_member(mach_task_self(), servicePort, portSet);
		if (KERN_SUCCESS != err)
		{
			DebugMessage("Unable to add service port to port set: 0x%x", err);
			exit(2);
		}

		// Service incoming messages from the clients and notifications which were signed up for
		while (true)
		{
			(void) mach_msg_server(MessagesAndNotifications, 8192, portSet, MACH_MSG_OPTION_NONE);
		}
	}
	catch (const CAException& exception)
	{
		exit(exception.GetError());
	}
	catch (...)
	{
		DebugMessage("Terminated by an an unknown exception");
		exit(44);
	}
}
