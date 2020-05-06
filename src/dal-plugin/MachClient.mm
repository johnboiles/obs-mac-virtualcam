//
//  MachClient.m
//  dal-plugin
//
//  Created by John Boiles  on 5/5/20.
//

#import "MachClient.h"
#import "MachProtocol.h"
#import "Logging.h"

@interface MachClient () <NSPortDelegate> {
    NSPort *_receivePort;
}
@end


@implementation MachClient

- (NSPort *)serverPort {
    // See note in MachServer.mm and don't judge me
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return [[NSMachBootstrapServer sharedInstance] portForName:MACH_SERVICE_NAME];
    #pragma clang diagnostic pop
}

- (BOOL)isConnected {
    return [self serverPort] != nil;
}

- (NSPort *)receivePort {
    if (_receivePort == nil) {
        _receivePort = [NSMachPort port];
        _receivePort.delegate = self;
        // TODO: BAD. Need to shut this down eventually.
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
            NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
            [runLoop addPort:_receivePort forMode:NSDefaultRunLoopMode];
            [[NSRunLoop currentRunLoop] run];
        });
        DLog(@"Initialized port %d", ((NSMachPort *)_receivePort).machPort);
    }
    return _receivePort;
}

- (void)sendConnectMessage {
    DLogFunc(@"");

    NSPort *sendPort = [self serverPort];
    if (sendPort == nil) {
        DLog(@"Unable to connect to server port");
        return;
    }

    NSPortMessage *message = [[NSPortMessage alloc]
                              initWithSendPort:sendPort
                              receivePort:self.receivePort
                              components:nil];
    message.msgid = MachMsgIdConnect;

    NSDate *timeout = [NSDate dateWithTimeIntervalSinceNow:5.0];
    if (![message sendBeforeDate:timeout]) {
        DLog(@"Send failed");
    }
}

- (void)handlePortMessage:(NSPortMessage *)message {
    DLogFunc(@"");
    NSArray *components = message.components;
    switch (message.msgid) {
        case MachMsgIdConnect:
            DLog(@"Received connect response");
            break;
        case MachMsgIdFrame:
            DLog(@"Received frame response");
            if (components.count > 0) {
                NSString *dataString = [[NSString alloc] initWithData:components[0] encoding:NSUTF8StringEncoding];
                DLog(@"Received frame response: \"%@\"", dataString);
            }
            break;
        default:
            DLog(@"Received unexpected response msgid %u", (unsigned)message.msgid);
            break;
    }
}

@end
