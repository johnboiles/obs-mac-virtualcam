//
//  Stream.mm
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

#import "Stream.h"
#import "Logging.h"
#import <AppKit/AppKit.h>
#import <mach/mach_time.h>

@interface Stream () {
    CMSimpleQueueRef _queue;
}

@property CMIODeviceStreamQueueAlteredProc alteredProc;
@property void * alteredRefCon;
@property (readonly) CMSimpleQueueRef queue;
@property NSTimer *frameTimer;

@end

@implementation Stream

- (void)startServingFrames {
    self.frameTimer = [NSTimer scheduledTimerWithTimeInterval:0.03 target:self selector:@selector(fillFrame) userInfo:nil repeats:YES];
}

- (CMSimpleQueueRef)queue {
    if (_queue == NULL) {
        CMSimpleQueueCreate(kCFAllocatorDefault, 10, &_queue);
    }
    return _queue;
}

- (CMSimpleQueueRef)copyBufferQueueWithAlteredProc:(CMIODeviceStreamQueueAlteredProc)alteredProc alteredRefCon:(void *)alteredRefCon {
    self.alteredProc = alteredProc;
    self.alteredRefCon = alteredRefCon;

    [self startServingFrames];

    return self.queue;
}

- (CVPixelBufferRef)CVPixelBufferRefFromUiImage:(NSImage *)img {

    CGSize size = img.size;
    NSRect rect = NSMakeRect(0, 0, img.size.width, img.size.height);
    CGImageRef image = [img CGImageForProposedRect:&rect context:nil hints:nil];

    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey,
                             [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, nil];
    CVPixelBufferRef pxbuffer = NULL;
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, size.width, size.height, kCVPixelFormatType_32ARGB, (__bridge CFDictionaryRef) options, &pxbuffer);

    NSParameterAssert(status == kCVReturnSuccess && pxbuffer != NULL);

    CVPixelBufferLockBaseAddress(pxbuffer, 0);
    void *pxdata = CVPixelBufferGetBaseAddress(pxbuffer);
    NSParameterAssert(pxdata != NULL);

    CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(pxdata, size.width, size.height, 8, 4*size.width, rgbColorSpace, kCGImageAlphaPremultipliedFirst);
    NSParameterAssert(context);

    CGContextDrawImage(context, CGRectMake(0, 0, CGImageGetWidth(image), CGImageGetHeight(image)), image);

    CGColorSpaceRelease(rgbColorSpace);
    CGContextRelease(context);

    CVPixelBufferUnlockBaseAddress(pxbuffer, 0);

    return pxbuffer;
}

- (void)fillFrame {
    if (CMSimpleQueueGetFullness(self.queue) == 1.0) {
        DLog(@"Queue is full, bailing out");
    }

    NSBundle *bundle = [NSBundle bundleForClass:[self class]];
    NSImage *myImage = [bundle imageForResource:@"hi"];
    
    CVPixelBufferRef pixelBuffer = [self CVPixelBufferRefFromUiImage:myImage];
    CMTimeScale scale = CMTimeScale(NSEC_PER_SEC);
    UInt64 time = mach_absolute_time();
    CMTime pts = CMTimeMake(time, scale);
    CMSampleTimingInfo timing;
    timing.duration = kCMTimeInvalid;
    timing.presentationTimeStamp = pts;
    timing.decodeTimeStamp = kCMTimeInvalid;
    
    CMFormatDescriptionRef format;
    CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, pixelBuffer, &format);
    
    CMSampleBufferRef buffer;
    CMSampleBufferCreateReadyWithImageBuffer(kCFAllocatorDefault, pixelBuffer, format, &timing, &buffer);

    CMSimpleQueueEnqueue(self.queue, buffer);

    // Inform the clients that the queue has been altered
    if (self.alteredProc != NULL) {
        (self.alteredProc)(1, buffer, self.alteredRefCon);
    }
}

#pragma mark - CMIOObject

- (UInt32)getPropertyDataSizeWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData {
    switch (address.mSelector) {
        case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
            return sizeof(CMTime);
        case kCMIOStreamPropertyOutputBuffersNeededForThrottledPlayback:
            return sizeof(UInt32);
        case kCMIOObjectPropertyName:
            return sizeof(CFStringRef);
        case kCMIOObjectPropertyManufacturer:
            return sizeof(CFStringRef);
        case kCMIOObjectPropertyElementName:
            return sizeof(CFStringRef);
        case kCMIOObjectPropertyElementCategoryName:
            return sizeof(CFStringRef);
        case kCMIOObjectPropertyElementNumberName:
            return sizeof(CFStringRef);
        case kCMIOStreamPropertyDirection:
            return sizeof(UInt32);
        case kCMIOStreamPropertyTerminalType:
            return sizeof(UInt32);
        case kCMIOStreamPropertyStartingChannel:
            return sizeof(UInt32);
        case kCMIOStreamPropertyLatency:
            return sizeof(UInt32);
        default:
            DLog(@"Stream unhandled getPropertyDataSizeWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return 0;
    };
}

- (void)getPropertyDataWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData dataSize:(UInt32)dataSize dataUsed:(nonnull UInt32 *)dataUsed data:(nonnull void *)data {
    switch (address.mSelector) {
        case kCMIOObjectPropertyName:
            *static_cast<CFStringRef*>(data) = CFSTR("CMIOMinimalSample Stream");
            *dataUsed = sizeof(CFStringRef);
            break;
        case kCMIOObjectPropertyElementName:
            *static_cast<CFStringRef*>(data) = CFSTR("CMIOMinimalSample Stream Element");
            *dataUsed = sizeof(CFStringRef);
            break;
        case kCMIOObjectPropertyManufacturer:
        case kCMIOObjectPropertyElementCategoryName:
        case kCMIOObjectPropertyElementNumberName:
        case kCMIOStreamPropertyTerminalType:
        case kCMIOStreamPropertyStartingChannel:
        case kCMIOStreamPropertyLatency:
        case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
        case kCMIOStreamPropertyOutputBuffersNeededForThrottledPlayback:
            DLog(@"TODO: %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            break;
        case kCMIOStreamPropertyDirection:
            *static_cast<UInt32*>(data) = 1;
            *dataUsed = sizeof(UInt32);
            break;
        default:
            DLog(@"Stream unhandled getPropertyDataWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
    };
}

- (BOOL)hasPropertyWithAddress:(CMIOObjectPropertyAddress)address {
    switch (address.mSelector){
        case kCMIOObjectPropertyName:
        case kCMIOObjectPropertyElementName:
            return true;
        case kCMIOObjectPropertyManufacturer:
        case kCMIOObjectPropertyElementCategoryName:
        case kCMIOObjectPropertyElementNumberName:
        case kCMIOStreamPropertyDirection:
        case kCMIOStreamPropertyTerminalType:
        case kCMIOStreamPropertyStartingChannel:
        case kCMIOStreamPropertyLatency:
        case kCMIOStreamPropertyInitialPresentationTimeStampForLinkedAndSyncedAudio:
        case kCMIOStreamPropertyOutputBuffersNeededForThrottledPlayback:
            DLog(@"TODO: %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return false;
        default:
            DLog(@"Stream unhandled hasPropertyWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
            return false;
    };
}

- (BOOL)isPropertySettableWithAddress:(CMIOObjectPropertyAddress)address {
    DLog(@"Stream unhandled isPropertySettableWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
    return false;
}

- (void)setPropertyDataWithAddress:(CMIOObjectPropertyAddress)address qualifierDataSize:(UInt32)qualifierDataSize qualifierData:(nonnull const void *)qualifierData dataSize:(UInt32)dataSize data:(nonnull const void *)data {
    DLog(@"Stream unhandled setPropertyDataWithAddress for %@", [ObjectStore StringFromPropertySelector:address.mSelector]);
}

@end
