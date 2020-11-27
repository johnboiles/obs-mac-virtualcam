//
//  OBSDALPlugIn.h
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

#import <Foundation/Foundation.h>
#import <CoreMediaIO/CMIOHardwarePlugIn.h>

#import "OBSDALObjectStore.h"
#import "OBSDALMachClient.h"
#import "OBSDALStream.h"

#define kTestCardWidthKey @"obs-mac-virtualcam-test-card-width"
#define kTestCardHeightKey @"obs-mac-virtualcam-test-card-height"
#define kTestCardFPSKey @"obs-mac-virtualcam-test-card-fps"

NS_ASSUME_NONNULL_BEGIN

@interface OBSDALPlugIn : NSObject <CMIOObject>

@property CMIOObjectID objectId;
@property (readonly) OBSDALMachClient *machClient;
@property OBSDALStream *stream;

+ (OBSDALPlugIn *)SharedOBSDALPlugIn;

- (void)initialize;

- (void)teardown;

- (void)startStream;

- (void)stopStream;

@end

NS_ASSUME_NONNULL_END
