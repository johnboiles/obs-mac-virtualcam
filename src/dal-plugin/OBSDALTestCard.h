//
//  OBSDALTestCard.h
//  dal-plugin
//
//  Created by John Boiles  on 5/8/20.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

void OBSDALDrawTestCardWithFrame(CGContextRef context, NSRect frame);
void OBSDALDrawDialWithFrame(NSRect frame, CGFloat rotation);

NSImage *OBSDALImageOfTestCardWithSize(NSSize imageSize);
