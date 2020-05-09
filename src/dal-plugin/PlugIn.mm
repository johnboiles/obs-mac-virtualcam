//
//  PlugIn.mm
//  obs-mac-virtualcam
//
//  Created by John Boiles  on 4/9/20.
//
//  obs-mac-virtualcam is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  obs-mac-virtualcam is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with obs-mac-virtualcam. If not, see <http://www.gnu.org/licenses/>.

#import "PlugIn.h"

#import <CoreMediaIO/CMIOHardwarePlugin.h>

#import "Logging.h"

@interface PlugIn () <MachClientDelegate> {
    MachClient *_machClient;
    dispatch_source_t _machConnectDispatchSource;
}
@property NSTimer *disconnectTimer;
@property BOOL connected;

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
        uint64_t intervalTime = (int64_t)(1 * NSEC_PER_SEC);
        dispatch_source_set_timer(_machConnectDispatchSource, startTime, intervalTime, 0);
        __weak typeof(self) wself = self;
        dispatch_source_set_event_handler(_machConnectDispatchSource, ^{
            if (![[wself machClient] isServerAvailable]) {
                DLog(@"Server is not available");
            } else if (!_connected) {
                DLog(@"Attempting connection");
                [[wself machClient] connectToServer];
            }
        });
    }
    return self;
}

- (void)startStream {
    DLogFunc(@"");
    dispatch_resume(_machConnectDispatchSource);
    [self.stream startServingDefaultFrames];
    [self.disconnectTimer invalidate];
}

- (void)stopStream {
    DLogFunc(@"");
    dispatch_suspend(_machConnectDispatchSource);
    [self.stream stopServingDefaultFrames];
    [self.disconnectTimer invalidate];
    _connected = false;
}

- (void)initialize {
}

- (void)teardown {
}

- (MachClient *)machClient {
    if (_machClient == nil) {
        _machClient = [[MachClient alloc] init];
        _machClient.delegate = self;
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
            *static_cast<CFStringRef*>(data) = CFSTR("OBS Virtual Camera Plugin");
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

#pragma mark - MachClientDelegate

- (void)receivedFrameWithSize:(NSSize)size timestamp:(uint64_t)timestamp frameData:(nonnull NSData *)frameData {
    if (!_connected) {
        _connected = YES;
        dispatch_suspend(_machConnectDispatchSource);
        [self.stream stopServingDefaultFrames];
    }

    // After 5 seconds of not receiving frames, give up on waiting for frames and go back to the static frame
    __weak typeof(self) weakSelf = self;
    [self.disconnectTimer invalidate];
    self.disconnectTimer = [NSTimer scheduledTimerWithTimeInterval:5 repeats:NO block:^(NSTimer * _Nonnull timer) {
        weakSelf.connected = NO;
        [weakSelf startStream];
    }];

    [self.stream queueFrameWithSize:size timestamp:timestamp frameData:frameData];
}

- (void)receivedStop {
    DLogFunc(@"");
    _connected = NO;
    [self startStream];
}

@end
