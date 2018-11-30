//
//  CMIO_DPA_Sample_Server_MIGInterface.h
//  Sample
//
//  Created by Tamás Lustyik on 2018. 11. 30..
//

#ifndef CMIO_DPA_Sample_Server_MIGInterface_h
#define CMIO_DPA_Sample_Server_MIGInterface_h

// Internal Includes
#include "CMIO_DPA_Sample_Shared.h"
#include "CMIO_PropertyAddress.h"

namespace CMIO { namespace DPA { namespace Sample {

    // abstract interface
    class MIGInterface {
    public:
        static MIGInterface* Instance() { return sInstance; }

    protected:
        MIGInterface() { sInstance = this; }
        
    public:
        virtual kern_return_t Connect(mach_port_t servicePort, pid_t client, mach_port_t* clientSendPort) = 0;
        virtual kern_return_t Disconnect(mach_port_t client) = 0;
        virtual kern_return_t GetDeviceStates(mach_port_t client, mach_port_t messagePort, DeviceState** deviceStates, mach_msg_type_number_t* length) = 0;
        virtual kern_return_t GetProperties(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, CMIOObjectPropertyAddress matchAddress, CMIO::PropertyAddress** addresses, mach_msg_type_number_t* length) = 0;
        virtual kern_return_t GetPropertyState(mach_port_t client, UInt64 guid, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, UInt8** data, mach_msg_type_number_t* length) = 0;
        virtual kern_return_t SetPropertyState(mach_port_t client, UInt64 guid, UInt32 sendChangedNotifications, CMIOObjectPropertyAddress address, UInt8* qualifier, mach_msg_type_number_t qualifierLength, Byte* data, mach_msg_type_number_t length) = 0;
        virtual kern_return_t GetControls(mach_port_t client, UInt64 guid, mach_port_t messagePort, UInt64 time, ControlChanges** controlChanges, mach_msg_type_number_t* length) = 0;
        virtual kern_return_t SetControl(mach_port_t client, UInt64 guid, UInt32 controlID, UInt32 value, UInt32* newValue) = 0;
        virtual kern_return_t ProcessRS422Command(mach_port_t client, UInt64 guid, ByteArray512 command, mach_msg_type_number_t commandLength, UInt32 responseLength, UInt32 *responseUsed, UInt8** response, mach_msg_type_number_t *responseCount) = 0;
        virtual kern_return_t StartStream(mach_port_t client, UInt64 guid, mach_port_t messagePort, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) = 0;
        virtual kern_return_t StopStream(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) = 0;
        virtual kern_return_t GetControlList(mach_port_t client, UInt64 guid, UInt8** data, mach_msg_type_number_t* length) = 0;
        virtual kern_return_t StartDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) = 0;
        virtual kern_return_t StopDeckThreads(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) = 0;
        virtual kern_return_t DeckPlay(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) = 0;
        virtual kern_return_t DeckStop(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element) = 0;
        virtual kern_return_t DeckJog(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, SInt32 speed) = 0;
        virtual kern_return_t DeckCueTo(mach_port_t client, UInt64 guid, CMIOObjectPropertyScope scope, CMIOObjectPropertyElement element, Float64 requestedTimecode, UInt32 playOnCue) = 0;
        
    private:
        // shared instance
        static MIGInterface* sInstance;
    };
    
}}}

#endif /* CMIO_DPA_Sample_Server_MIGInterface_h */
