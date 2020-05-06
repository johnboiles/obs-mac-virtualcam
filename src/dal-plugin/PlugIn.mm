//
//  PlugIn.mm
//  CMIOMinimalSample
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

#import "PlugIn.h"

#import <CoreMediaIO/CMIOHardwarePlugin.h>

#import "Logging.h"

@interface PlugIn () {
    MachClient *_machClient;
    dispatch_source_t _machConnectDispatchSource;
    BOOL _connected;
}

@end


@implementation PlugIn

+ (PlugIn *)SharedPlugIn {
    static PlugIn *sPlugIn = nil;
    static dispatch_once_t sOnceToken;
    dispatch_once(&sOnceToken, ^{
        sPlugIn = [[self alloc] init];
    });
    return sPlugIn;
}

- (instancetype)init {
    if (self = [super init]) {
        _machConnectDispatchSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
        dispatch_time_t startTime = dispatch_time(DISPATCH_TIME_NOW, 0);
        uint64_t intervalTime = (int64_t)(5 * NSEC_PER_SEC);
        dispatch_source_set_timer(_machConnectDispatchSource, startTime, intervalTime, 0);
        __weak typeof(self) wself = self;
        dispatch_source_set_event_handler(_machConnectDispatchSource, ^{
            if (![[wself machClient] isConnected]) {
                DLog(@"Server is not available");
                return;
            }
            if (!_connected) {
                DLog(@"Attempting connection");
                [[wself machClient] sendConnectMessage];
                _connected = YES;
            }
        });
        dispatch_resume(_machConnectDispatchSource);
    }
    return self;
}

- (void)initialize {
}

- (void)teardown {
}

- (MachClient *)machClient {
    if (_machClient == nil) {
        _machClient = [[MachClient alloc] init];
    }
    return _machClient;
}

#pragma mark - CMIOObject

- (BOOL)hasPropertyWithAddress:(CMIOObjectPropertyAddress)address {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            return true;
        default:
            DLog(@"PlugIn unhandled hasPropertyWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return false;
    };
}

- (BOOL)isPropertySettableWithAddress:(CMIOObjectPropertyAddress)address {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            return false;
        default:
            DLog(@"PlugIn unhandled isPropertySettableWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return false;
    };
}

- (UInt32)getPropertyDataSizeWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(const void*)qualifierData {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            return sizeof(CFStringRef);
        default:
            DLog(@"PlugIn unhandled getPropertyDataSizeWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return 0;
    };
}

- (void)getPropertyDataWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData dataSize:(UInt32)dataSize dataUsed:(nonnull UInt32 *)dataUsed data:(nonnull void *)data {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            *static_cast<CFStringRef*>(data) = CFSTR("CMIOMinimalSample Plugin");
            *dataUsed = sizeof(CFStringRef);
            return;
        default:
            DLog(@"PlugIn unhandled getPropertyDataWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return;
        };
}

- (void)setPropertyDataWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData dataSize:(UInt32)dataSize data:(nonnull const void *)data {
    DLog(@"PlugIn unhandled setPropertyDataWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
}

@end
