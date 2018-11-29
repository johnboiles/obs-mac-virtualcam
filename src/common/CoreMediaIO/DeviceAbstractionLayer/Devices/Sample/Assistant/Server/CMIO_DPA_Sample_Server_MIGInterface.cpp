//
//  CMIODPASampleServer.cpp
//  Sample Assistant
//
//  Created by Tamás Lustyik on 2018. 11. 29..
//

// MIG Server Interface
#include "CMIODPASampleServer.h"

// Internal Includes
#include "CMIO_DPA_Sample_Server_Assistant.h"
#include "CMIO_DPA_Sample_Shared.h"
#include "CMIO_PropertyAddress.h"

// As a convenience, use the CMIO::DPA::Sample::Server namespace
using namespace CMIO::DPA::Sample;
using namespace CMIO::DPA::Sample::Server;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleConnect()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleConnect(mach_port_t servicePort, pid_t client, mach_port_t* clientSendPort)
{
	return Assistant::Instance()->Connect(client, clientSendPort);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDisconnect()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDisconnect(mach_port_t client)
{
	return Assistant::Instance()->Disconnect(client);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetDeviceStates()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetDeviceStates(mach_port_t client, mach_port_t messagePort, DeviceState** deviceStates, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetDeviceStates(client, messagePort, deviceStates, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetProperties()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetProperties(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, CMIOObjectPropertyAddress matchAddress, CMIO::PropertyAddress** addresses, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetProperties(client, guid, messagePort, time, matchAddress, addresses, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetPropertyState()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetPropertyState(mach_port_t client, UInt64 guid, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetPropertyState(client, guid, address, qualifier, qualifierLength, data, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleSetPropertyState()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleSetPropertyState(mach_port_t client, UInt64 guid, UInt32 sendChangedNotifications, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length)
{
	return Assistant::Instance()->SetPropertyState(client, guid, sendChangedNotifications, address, qualifier, qualifierLength, data, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetControls()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetControls(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetControls(client, guid, messagePort, time, controlChanges, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleSetControl()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleSetControl(mach_port_t client, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue)
{
	return Assistant::Instance()->SetControl(client, guid, controlID, value, newValue);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleProcessRS422Command()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleProcessRS422Command(mach_port_t client, UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount)
{
	return Assistant::Instance()->ProcessRS422Command(guid, command, commandLength, responseLength, responseUsed, response, responseCount);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStartStream()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStartStream(mach_port_t client, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StartStream(client, guid, messagePort, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStopStream()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStopStream(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StopStream(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleGetControlList()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleGetControlList(mach_port_t client, UInt64 guid, UInt8** data, mach_msg_type_number_t* length)
{
	return Assistant::Instance()->GetControlList(client, guid, data, length);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStartDeckThreads()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStartDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StartDeckThreads(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleStopDeckThreads()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleStopDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->StopDeckThreads(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckPlay()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckPlay(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->DeckPlay(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckStop()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckStop(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element)
{
	return Assistant::Instance()->DeckStop(client, guid, scope, element);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckPlay()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckJog(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed)
{
	return Assistant::Instance()->DeckJog(client, guid, scope, element, speed);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// CMIODPASampleDeckPlay()
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
kern_return_t CMIODPASampleDeckCueTo(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, UInt32 playOnCue)
{
	return Assistant::Instance()->DeckCueTo(client, guid, scope, element, requestedTimecode, playOnCue);
}
