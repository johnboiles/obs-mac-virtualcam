//
//  MachServer.m
//  mac-virtualcam
//
//  Created by John Boiles  on 5/5/20.
//

#import "MachServer.h"
#import <Foundation/Foundation.h>
#include <obs-module.h>
#include "MachProtocol.h"

@interface MachServer () <NSPortDelegate>
@property NSPort *port;
@property NSMutableSet *clientPorts;
@end


@implementation MachServer

- (id)init {
    if (self = [super init]) {
        self.clientPorts = [[NSMutableSet alloc] init];
    }
    return self;
}

- (void)run {
    if (self.port != nil) {
        blog(LOG_DEBUG, "VIRTUALCAM mach server already running!");
        return;
    }

    // It's a bummer this is deprecated. The replacement, NSXPCConnection, seems to require
    // an assistant process that lives inside the .app bundle. This would be more modern, but adds
    // complexity and I think makes it impossible to just run the `obs` binary from the commandline.
    // So let's stick with NSMachBootstrapServer at least until it fully goes away.
    // At that point we can decide between NSXPCConnection and using the CoreFoundation versions of
    // these APIs (which are, interestingly, not deprecated)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    self.port = [[NSMachBootstrapServer sharedInstance] servicePortWithName:@MACH_SERVICE_NAME];
    #pragma clang diagnostic pop
    if (self.port == nil) {
        // This probably means another instance is running.
        blog(LOG_ERROR, "VIRTUALCAM Unable to open mach server port.");
        return;
    }

    self.port.delegate = self;

    NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
    [runLoop addPort:self.port forMode:NSDefaultRunLoopMode];

    blog(LOG_DEBUG, "VIRTUALCAM mach server running!");
}

- (void)handlePortMessage:(NSPortMessage *)message {
    switch (message.msgid) {
        case MachMsgIdConnect:
            if (message.sendPort != nil) {
                blog(LOG_DEBUG, "VIRTUALCAM mach server received connect message from port %d!", ((NSMachPort *)message.sendPort).machPort);
                [self.clientPorts addObject:message.sendPort];
            }
            break;
        default:
            blog(LOG_ERROR, "Unexpected mach message ID %u", (unsigned)message.msgid);
            break;
    }
}

- (void)sendMessageToClientsWithMsgId:(uint32_t)msgId components:(nullable NSArray *)components {
    if ([self.clientPorts count] <= 0) {
        return;
    }

    NSMutableSet *removedPorts = [NSMutableSet set];

    for (NSPort *port in self.clientPorts) {
        @try {
            NSPortMessage *message = [[NSPortMessage alloc] initWithSendPort:port receivePort:nil components:components];
            message.msgid = msgId;
            if (![message sendBeforeDate:[NSDate dateWithTimeIntervalSinceNow:1.0]]) {
                blog(LOG_DEBUG, "VIRTUALCAM failed to send message to %d, removing it from the clients!", ((NSMachPort *)port).machPort);
                [removedPorts addObject:port];
            }
        } @catch (NSException *exception) {
            blog(LOG_DEBUG, "VIRTUALCAM failed to send message (exception) to %d, removing it from the clients!", ((NSMachPort *)port).machPort);
            [removedPorts addObject:port];
        }
    }

    // Remove dead ports if necessary
    [self.clientPorts minusSet:removedPorts];
}

- (void)sendFrameWithSize:(NSSize)size timestamp:(uint64_t)timestamp frameBytes:(uint8_t *)frameBytes {
    if ([self.clientPorts count] <= 0) {
        return;
    }

    CGFloat width = size.width;
    NSData *widthData = [NSData dataWithBytes:&width length:sizeof(width)];
    CGFloat height = size.height;
    NSData *heightData = [NSData dataWithBytes:&height length:sizeof(height)];
    NSData *timestampData = [NSData dataWithBytes:&timestamp length:sizeof(timestamp)];
    // TODO: I wonder if we used the CF apis for Mach IPC if we could avoid extra memory copies
    NSData *frameData = [NSData dataWithBytes:(void *)frameBytes length:size.width * size.height * 2];
    // Seems to cause a crash, sometimes
    // NSData *frameData = [NSData dataWithBytesNoCopy:(void *)frameBytes length:size.width * size.height * 2];

    [self sendMessageToClientsWithMsgId:MachMsgIdFrame components:@[widthData, heightData, timestampData, frameData]];
}

- (void)stop {
    blog(LOG_DEBUG, "VIRTUALCAM sending stop message to %lu clients", self.clientPorts.count);
    [self sendMessageToClientsWithMsgId:MachMsgIdStop components:nil];
}

@end
