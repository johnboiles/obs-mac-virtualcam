//
//  PlugInInterface.mm
//  CMIOMinimalSample
//
//  This file implements the CMIO DAL plugin interface
//
//  Created by John Boiles  on 4/9/20.
//
//  CMIOMinimalSample is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  CMIOMinimalSample is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with CMIOMinimalSample. If not, see <http://www.gnu.org/licenses/>.

#import <CoreFoundation/CFUUID.h>
#import "PlugInInterface.h"
#import "PlugIn.h"
#import "Device.h"
#import "Stream.h"
#import "Logging.h"

#pragma mark Plug-In Operations

// Don't think I need to do this with ObjC
ULONG HardwarePlugIn_AddRef(CMIOHardwarePlugInRef self) {
    DLogFunc(@"");
    return 1;
}

// Don't think I need to do this with ObjC
ULONG HardwarePlugIn_Release(CMIOHardwarePlugInRef self) {
    DLogFunc(@"");
    return 1;
}

HRESULT HardwarePlugIn_QueryInterface(CMIOHardwarePlugInRef self, REFIID uuid, LPVOID* interface) {
    DLogFunc(@"");

    if (!interface) {
        DLogFunc(@"Received an empty interface");
        return E_POINTER;
    }

    // Set the returned interface to NULL
    *interface = NULL;

    // Create a CoreFoundation UUIDRef for the requested interface.
    CFUUIDRef cfUuid = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, uuid);
    CFStringRef uuidString = CFUUIDCreateString(NULL, cfUuid);
    CFStringRef hardwarePluginUuid = CFUUIDCreateString(NULL, kCMIOHardwarePlugInInterfaceID);

    if (CFEqual(uuidString, hardwarePluginUuid)) {
        // Return the interface;
        *interface = PlugInRef();
        return kCMIOHardwareNoError;
    } else {
        DLogFunc(@"Queried for some weird UUID %@", uuidString);
    }
    
    return E_NOINTERFACE;
}

// I think this is deprecated, seems that HardwarePlugIn_InitializeWithObjectID gets called instead
OSStatus HardwarePlugIn_Initialize(CMIOHardwarePlugInRef self) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareUnspecifiedError;
}

OSStatus HardwarePlugIn_InitializeWithObjectID(CMIOHardwarePlugInRef self, CMIOObjectID objectID) {
    DLogFunc(@"self=%p", self);

    OSStatus error = kCMIOHardwareNoError;

    PlugIn *plugIn = [PlugIn SharedPlugIn];
    CMIOObjectID plugInId;
    CMIOObjectCreate(self, kCMIOObjectSystemObject, kCMIOPlugInClassID, &plugInId);
    plugIn.objectId = plugInId;
    [[ObjectStore SharedObjectStore] setObject:plugIn forObjectId:plugInId];

    Device *device = [Device SharedDevice];
    CMIOObjectID deviceId;
    CMIOObjectCreate(self, kCMIOObjectSystemObject, kCMIODeviceClassID, &deviceId);
    device.objectId = deviceId;
    [[ObjectStore SharedObjectStore] setObject:device forObjectId:deviceId];

    Stream *stream = [[Stream alloc] init];
    CMIOObjectID streamId;
    CMIOObjectCreate(self, deviceId, kCMIOStreamClassID, &streamId);
    stream.objectId = streamId;
    [[ObjectStore SharedObjectStore] setObject:stream forObjectId:streamId];
    device.streamId = streamId;

    error = CMIOObjectsPublishedAndDied(PlugInRef(), kCMIOObjectSystemObject, 1, &deviceId, 0, 0);
    if (error != kCMIOHardwareNoError) {
        DLog(@"CMIOObjectsPublishedAndDied plugin/device Error %d", error);
        return error;
    }
    error = CMIOObjectsPublishedAndDied(PlugInRef(), deviceId, 1, &streamId, 0, 0);
    if (error != kCMIOHardwareNoError) {
        DLog(@"CMIOObjectsPublishedAndDied device/stream Error %d", error);
        return error;
    }

    return error;
}

OSStatus HardwarePlugIn_Teardown(CMIOHardwarePlugInRef self) {
    DLogFunc(@"self=%p", self);

    OSStatus error = kCMIOHardwareNoError;

    PlugIn *plugIn = [PlugIn SharedPlugIn];
    [plugIn teardown];

    return error;
}

#pragma mark CMIOObject Operations

// I think this is just a debug thing
void HardwarePlugIn_ObjectShow(CMIOHardwarePlugInRef self, CMIOObjectID objectID) {
    DLogFunc(@"self=%p", self);
}

Boolean  HardwarePlugIn_ObjectHasProperty(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address) {

    NSObject<CMIOObject> *object = [ObjectStore GetObjectWithId:objectID];
    
    Boolean answer = [object hasPropertyWithAddress:*address];

    DLogFunc(@"%@(%d) %@ self=%p hasProperty=%d", NSStringFromClass([object class]), objectID, [ObjectStore StringFromPropertySelector:address->mSelector], self, answer);

    return answer;
}

OSStatus HardwarePlugIn_ObjectIsPropertySettable(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, Boolean* isSettable) {

    NSObject<CMIOObject> *object = [ObjectStore GetObjectWithId:objectID];

    *isSettable = [object isPropertySettableWithAddress:*address];

    DLogFunc(@"%@(%d) %@ self=%p settable=%d", NSStringFromClass([object class]), objectID, [ObjectStore StringFromPropertySelector:address->mSelector], self, *isSettable);

    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_ObjectGetPropertyDataSize(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, UInt32 qualifierDataSize, const void* qualifierData, UInt32* dataSize) {

    NSObject<CMIOObject> *object = [ObjectStore GetObjectWithId:objectID];
    
    *dataSize = [object getPropertyDataSizeWithAddress:*address qualifierDataSize:qualifierDataSize qualifierData:qualifierData];

    DLogFunc(@"%@(%d) %@ self=%p size=%d", NSStringFromClass([object class]), objectID, [ObjectStore StringFromPropertySelector:address->mSelector], self, *dataSize);

    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_ObjectGetPropertyData(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, UInt32* dataUsed, void* data) {

    NSObject<CMIOObject> *object = [ObjectStore GetObjectWithId:objectID];

    [object getPropertyDataWithAddress:*address qualifierDataSize:qualifierDataSize qualifierData:qualifierData dataSize:dataSize dataUsed:dataUsed data:data];

    UInt32 *dataInt = (UInt32 *)data;
    DLogFunc(@"%@(%d) %@ self=%p data(int)=%d", NSStringFromClass([object class]), objectID, [ObjectStore StringFromPropertySelector:address->mSelector], self, *dataInt);

    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_ObjectSetPropertyData(CMIOHardwarePlugInRef self, CMIOObjectID objectID, const CMIOObjectPropertyAddress* address, UInt32 qualifierDataSize, const void* qualifierData, UInt32 dataSize, const void* data) {

    NSObject<CMIOObject> *object = [ObjectStore GetObjectWithId:objectID];

    UInt32 *dataInt = (UInt32 *)data;
    DLogFunc(@"%@(%d) %@ self=%p data(int)=%d", NSStringFromClass([object class]), objectID, [ObjectStore StringFromPropertySelector:address->mSelector], self, *dataInt);

    [object setPropertyDataWithAddress:*address qualifierDataSize:qualifierDataSize qualifierData:qualifierData dataSize:dataSize data:data];

    return kCMIOHardwareNoError;
}

#pragma mark CMIOStream Operations
OSStatus HardwarePlugIn_StreamCopyBufferQueue(CMIOHardwarePlugInRef self, CMIOStreamID streamID, CMIODeviceStreamQueueAlteredProc queueAlteredProc, void* queueAlteredRefCon, CMSimpleQueueRef* queue) {

    Stream *stream = (Stream *)[ObjectStore GetObjectWithId:streamID];
    DLogFunc(@"%@ (id=%d) self=%p", stream, streamID, self);

    *queue = [stream copyBufferQueueWithAlteredProc:queueAlteredProc alteredRefCon:queueAlteredRefCon];

    return kCMIOHardwareNoError;
}

#pragma mark CMIODevice Operations
OSStatus HardwarePlugIn_DeviceStartStream(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIOStreamID streamID) {
    DLogFunc(@"self=%p", self);

    Stream *stream = (Stream *)[ObjectStore GetObjectWithId:streamID];
    [stream startServingFrames];

    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_DeviceSuspend(CMIOHardwarePlugInRef self, CMIODeviceID deviceID) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_DeviceResume(CMIOHardwarePlugInRef self, CMIODeviceID deviceID) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_DeviceStopStream(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIOStreamID streamID) {
    DLogFunc(@"self=%p", self);

    Stream *stream = (Stream *)[ObjectStore GetObjectWithId:streamID];
    [stream stopServingFrames];

    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_DeviceProcessAVCCommand(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIODeviceAVCCommand* ioAVCCommand) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_DeviceProcessRS422Command(CMIOHardwarePlugInRef self, CMIODeviceID deviceID, CMIODeviceRS422Command* ioRS422Command) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareNoError;
}

OSStatus HardwarePlugIn_StreamDeckPlay(CMIOHardwarePlugInRef self, CMIOStreamID streamID) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareIllegalOperationError;
}

OSStatus HardwarePlugIn_StreamDeckStop(CMIOHardwarePlugInRef self,CMIOStreamID streamID) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareIllegalOperationError;
}

OSStatus HardwarePlugIn_StreamDeckJog(CMIOHardwarePlugInRef self, CMIOStreamID streamID, SInt32 speed) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareIllegalOperationError;
}

OSStatus HardwarePlugIn_StreamDeckCueTo(CMIOHardwarePlugInRef self, CMIOStreamID streamID, Float64 requestedTimecode, Boolean playOnCue) {
    DLogFunc(@"self=%p", self);
    return kCMIOHardwareIllegalOperationError;
}

static CMIOHardwarePlugInInterface sInterface = {
    // Padding for COM
    NULL,
    
    // IUnknown Routines
    (HRESULT (*)(void*, CFUUIDBytes, void**))HardwarePlugIn_QueryInterface,
    (ULONG (*)(void*))HardwarePlugIn_AddRef,
    (ULONG (*)(void*))HardwarePlugIn_Release,
    
    // DAL Plug-In Routines
    HardwarePlugIn_Initialize,
    HardwarePlugIn_InitializeWithObjectID,
    HardwarePlugIn_Teardown,
    HardwarePlugIn_ObjectShow,
    HardwarePlugIn_ObjectHasProperty,
    HardwarePlugIn_ObjectIsPropertySettable,
    HardwarePlugIn_ObjectGetPropertyDataSize,
    HardwarePlugIn_ObjectGetPropertyData,
    HardwarePlugIn_ObjectSetPropertyData,
    HardwarePlugIn_DeviceSuspend,
    HardwarePlugIn_DeviceResume,
    HardwarePlugIn_DeviceStartStream,
    HardwarePlugIn_DeviceStopStream,
    HardwarePlugIn_DeviceProcessAVCCommand,
    HardwarePlugIn_DeviceProcessRS422Command,
    HardwarePlugIn_StreamCopyBufferQueue,
    HardwarePlugIn_StreamDeckPlay,
    HardwarePlugIn_StreamDeckStop,
    HardwarePlugIn_StreamDeckJog,
    HardwarePlugIn_StreamDeckCueTo
};

static CMIOHardwarePlugInInterface* sInterfacePtr = &sInterface;
static CMIOHardwarePlugInRef sPlugInRef = &sInterfacePtr;

CMIOHardwarePlugInRef PlugInRef() {
    return sPlugInRef;
}
