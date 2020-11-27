//
//  OBSDALCMSampleBufferUtils.h
//  dal-plugin
//
//  Created by John Boiles  on 5/8/20.
//

#include <CoreMediaIO/CMIOSampleBuffer.h>

OSStatus OBSDALCMSampleBufferCreateFromData(NSSize size, CMSampleTimingInfo timingInfo, UInt64 sequenceNumber, NSData *data, CMSampleBufferRef *sampleBuffer);

OSStatus OBSDALCMSampleBufferCreateFromDataNoCopy(NSSize size, CMSampleTimingInfo timingInfo, UInt64 sequenceNumber, NSData *data, CMSampleBufferRef *sampleBuffer);

CMSampleTimingInfo OBSDALCMSampleTimingInfoForTimestamp(uint64_t timestampNanos, uint32_t fpsNumerator, uint32_t fpsDenominator);
