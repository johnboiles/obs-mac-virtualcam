//
//  MachServer.h
//  obs-mac-virtualcam
//
//  Created by John Boiles  on 5/5/20.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MachServer : NSObject

- (void)run;

/*!
 Will eventually be used for sending frames to all connected clients
 */
- (void)sendFrame;

@end

NS_ASSUME_NONNULL_END
