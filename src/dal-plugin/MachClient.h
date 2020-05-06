//
//  MachClient.h
//  dal-plugin
//
//  Created by John Boiles  on 5/5/20.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MachClient : NSObject

- (BOOL)isConnected;

- (void)sendConnectMessage;

@end

NS_ASSUME_NONNULL_END
