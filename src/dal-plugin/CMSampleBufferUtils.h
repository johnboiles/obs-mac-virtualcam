//
//  CMSampleBufferUtils.h
//  dal-plugin
//
//  Created by John Boiles  on 5/8/20.
//

#include <CoreMediaIO/CMIOSampleBuffer.h>

OSStatus CMSampleBufferCreateFromData(NSSize size, CMSampleTimingInfo timingInfo, UInt64 sequenceNumber, NSData *data, CMSampleBufferRef *sampleBuffer);

OSStatus CMSampleBufferCreateFromDataNoCopy(NSSize size, CMSampleTimingInfo timingInfo, UInt64 sequenceNumber, NSData *data, CMSampleBufferRef *sampleBuffer);
