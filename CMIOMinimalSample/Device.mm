//
//  Device.mm
//  CMIOMinimalSample
//
//  Created by John Boiles  on 4/10/20.
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

#import "Device.h"
#import <CoreFoundation/CoreFoundation.h>
#include <IOKit/audio/IOAudioTypes.h>
#import "PlugIn.h"
#import "Logging.h"

@interface Device ()
@property BOOL excludeNonDALAccess;
@property pid_t masterPid;
@end


@implementation Device

+ (Device *)SharedDevice {
    static Device *sDevice = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sDevice = [[self alloc] init];
    });
    return sDevice;
}

// Note that the DAL's API calls HasProperty before calling GetPropertyDataSize. This means that it can be assumed that address is valid for the property involved.
- (UInt32)getPropertyDataSizeWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData {
    UInt32 answer = 0;

    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            answer = sizeof(CFStringRef);
            break;
            
        case kCMIOObjectPropertyManufacturer:
            answer = sizeof(CFStringRef);
            break;
            
        case kCMIOObjectPropertyElementCategoryName:
            answer = sizeof(CFStringRef);
            break;
            
        case kCMIOObjectPropertyElementNumberName:
            answer = sizeof(CFStringRef);
            break;
            
        case kCMIODevicePropertyPlugIn:
            answer = sizeof(CMIOObjectID);
            break;
            
        case kCMIODevicePropertyDeviceUID:
            answer = sizeof(CFStringRef);
            break;
            
        case kCMIODevicePropertyModelUID:
            answer = sizeof(CFStringRef);
            break;
            
        case kCMIODevicePropertyTransportType:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceIsAlive:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceHasChanged:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceIsRunning:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceIsRunningSomewhere:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceCanBeDefaultDevice:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyHogMode:
            answer = sizeof(pid_t);
            break;
            
        case kCMIODevicePropertyLatency:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyStreams:
            // Only one stream
            answer = sizeof(CMIOStreamID) * 1;
            break;
            
        case kCMIODevicePropertyStreamConfiguration:
            // Only one stream
            answer = sizeof(UInt32) + (sizeof(UInt32) * 1);
            break;
            
        case kCMIODevicePropertyExcludeNonDALAccess:
            answer = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyCanProcessAVCCommand:
            answer = sizeof(Boolean);
            break;
            
        case kCMIODevicePropertyCanProcessRS422Command:
            answer = sizeof(Boolean);
            break;
            
        case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
            answer = sizeof(CFStringRef);
            break;

        case kCMIODevicePropertyDeviceMaster:
            answer = sizeof(pid_t);
            break;
            
        default:
            DLog(@"Device unhandled getPropertyDataSizeWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            break;
    };

    return answer;
}

// Note that the DAL's API calls HasProperty before calling GetPropertyData. This means that it can be assumed that address is valid for the property involved.
- (void)getPropertyDataWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData dataSize:(UInt32)dataSize dataUsed:(nonnull UInt32 *)dataUsed data:(nonnull void *)data {

    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            *static_cast<CFStringRef*>(data) = CFSTR("CMIOMinimalSample Device");
            *dataUsed = sizeof(CFStringRef);
            break;
        case kCMIOObjectPropertyManufacturer:
            *static_cast<CFStringRef*>(data) = CFSTR("johnboiles");
            *dataUsed = sizeof(CFStringRef);
            break;
            
        case kCMIOObjectPropertyElementCategoryName:
            *static_cast<CFStringRef*>(data) = CFSTR("?? wtf");
            *dataUsed = sizeof(CFStringRef);
            break;
            
        case kCMIOObjectPropertyElementNumberName:
            *static_cast<CFStringRef*>(data) = CFSTR("element number name");
            *dataUsed = sizeof(CFStringRef);
            break;
            
        case kCMIODevicePropertyPlugIn:
            *static_cast<CMIOObjectID*>(data) = [[PlugIn SharedPlugIn] objectId];
            *dataUsed = sizeof(CMIOObjectID);
            break;
            
        case kCMIODevicePropertyDeviceUID:
            *static_cast<CFStringRef*>(data) = CFSTR("CMIO Simple Device");
            *dataUsed = sizeof(CFStringRef);
            break;
            
        case kCMIODevicePropertyModelUID:
            *static_cast<CFStringRef*>(data) = CFSTR("CMIO Simple Model");
            *dataUsed = sizeof(CFStringRef);
            break;
            
        case kCMIODevicePropertyTransportType:
            *static_cast<UInt32*>(data) = kIOAudioDeviceTransportTypePCI;
//            *static_cast<UInt32*>(data) = kIOAudioDeviceTransportTypeBuiltIn;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceIsAlive:
            *static_cast<UInt32*>(data) = 1;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceHasChanged:
            *static_cast<UInt32*>(data) = 0;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceIsRunning:
            *static_cast<UInt32*>(data) = 1;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceIsRunningSomewhere:
            *static_cast<UInt32*>(data) = 1;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyDeviceCanBeDefaultDevice:
            *static_cast<UInt32*>(data) = 1;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyHogMode:
            *static_cast<pid_t*>(data) = -1;
            *dataUsed = sizeof(pid_t);
            break;
            
        case kCMIODevicePropertyLatency:
            *static_cast<UInt32*>(data) = 0;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyStreams:
            *static_cast<CMIOObjectID*>(data) = self.streamId;
            *dataUsed = sizeof(CMIOObjectID);
            break;
            
        case kCMIODevicePropertyStreamConfiguration:
            DLog(@"kCMIODevicePropertyStreamConfiguration");
            break;

        case kCMIODevicePropertyExcludeNonDALAccess:
            *static_cast<UInt32*>(data) = 0;
            *dataUsed = sizeof(UInt32);
            break;
            
        case kCMIODevicePropertyCanProcessAVCCommand:
            *static_cast<Boolean*>(data) = false;
            *dataUsed = sizeof(Boolean);
            break;
            
        case kCMIODevicePropertyCanProcessRS422Command:
            *static_cast<Boolean*>(data) = false;
            *dataUsed = sizeof(Boolean);
            break;

        case kCMIODevicePropertyDeviceMaster:
            *static_cast<pid_t*>(data) = self.masterPid;
            *dataUsed = sizeof(pid_t);
            break;

        default:
            DLog(@"Device unhandled getPropertyDataWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            break;
    };
}

- (BOOL)hasPropertyWithAddress:(CMIOObjectPropertyAddress)address {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
        case kCMIOObjectPropertyManufacturer:
        case kCMIOObjectPropertyElementCategoryName:
        case kCMIOObjectPropertyElementNumberName:
        case kCMIODevicePropertyPlugIn:
        case kCMIODevicePropertyDeviceUID:
        case kCMIODevicePropertyModelUID:
        case kCMIODevicePropertyTransportType:
        case kCMIODevicePropertyDeviceIsAlive:
        case kCMIODevicePropertyDeviceHasChanged:
        case kCMIODevicePropertyDeviceIsRunning:
        case kCMIODevicePropertyDeviceIsRunningSomewhere:
        case kCMIODevicePropertyDeviceCanBeDefaultDevice:
        case kCMIODevicePropertyHogMode:
        case kCMIODevicePropertyLatency:
        case kCMIODevicePropertyStreams:
        case kCMIODevicePropertyExcludeNonDALAccess:
        case kCMIODevicePropertyCanProcessAVCCommand:
        case kCMIODevicePropertyCanProcessRS422Command:
        case kCMIODevicePropertyDeviceMaster:
            return true;
        case kCMIODevicePropertyStreamConfiguration:
        case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
            return false;
        default:
            DLog(@"Device unhandled hasPropertyWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return false;
    };
}

- (BOOL)isPropertySettableWithAddress:(CMIOObjectPropertyAddress)address {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
        case kCMIOObjectPropertyManufacturer:
        case kCMIOObjectPropertyElementCategoryName:
        case kCMIOObjectPropertyElementNumberName:
        case kCMIODevicePropertyPlugIn:
        case kCMIODevicePropertyDeviceUID:
        case kCMIODevicePropertyModelUID:
        case kCMIODevicePropertyTransportType:
        case kCMIODevicePropertyDeviceIsAlive:
        case kCMIODevicePropertyDeviceHasChanged:
        case kCMIODevicePropertyDeviceIsRunning:
        case kCMIODevicePropertyDeviceIsRunningSomewhere:
        case kCMIODevicePropertyDeviceCanBeDefaultDevice:
        case kCMIODevicePropertyHogMode:
        case kCMIODevicePropertyLatency:
        case kCMIODevicePropertyStreams:
        case kCMIODevicePropertyStreamConfiguration:
        case kCMIODevicePropertyCanProcessAVCCommand:
        case kCMIODevicePropertyCanProcessRS422Command:
        case kCMIODevicePropertyLinkedCoreAudioDeviceUID:
            return false;
        case kCMIODevicePropertyExcludeNonDALAccess:
        case kCMIODevicePropertyDeviceMaster:
            return true;
        default:
            DLog(@"Device unhandled isPropertySettableWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return false;
    };
}

- (void)setPropertyDataWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData dataSize:(UInt32)dataSize data:(nonnull const void *)data {
                
    switch (address.mSelector) {
        case kCMIODevicePropertyExcludeNonDALAccess:
            self.excludeNonDALAccess = (*static_cast<const UInt32*>(data) != 0);
            break;
        case kCMIODevicePropertyDeviceMaster:
            self.masterPid = *static_cast<const pid_t*>(data);
            break;
        default:
            DLog(@"Device unhandled setPropertyDataWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            break;
    };
}

@end
