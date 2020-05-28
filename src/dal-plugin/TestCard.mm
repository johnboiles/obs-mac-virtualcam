//
//  TestCard.m
//  dal-plugin
//
//  Created by John Boiles  on 5/8/20.
//

#import "TestCard.h"

#import <AppKit/AppKit.h>

// This code was generated by Trial version of PaintCode, therefore cannot be used for commercial purposes.
// http://www.paintcodeapp.com

void DrawTestCardWithFrame(CGContextRef context, NSRect frame)
{
    if (context == NULL) {
        context = NSGraphicsContext.currentContext.CGContext;
    }

    BOOL showText = (frame.size.width >= 1280 && frame.size.height >= 720);

    CGFloat centerWidth = floor(frame.size.width * 0.70840 - 0.25) - floor(frame.size.width * 0.29199 - 0.25);
    NSRect center = NSMakeRect(NSMinX(frame) + floor(frame.size.width * 0.29199 - 0.25) + 0.75, NSMinY(frame) + floor((frame.size.height - centerWidth) / 2), centerWidth, centerWidth);

    // Paste in PaintCode code below

    //// Color Declarations
    NSColor* fillColor = [NSColor colorWithRed: 0.125 green: 0.176 blue: 0.435 alpha: 1];
    NSColor* fillColor2 = [NSColor colorWithRed: 0.086 green: 0.141 blue: 0.345 alpha: 1];
    NSColor* fillColor3 = [NSColor colorWithRed: 0.047 green: 0.086 blue: 0.2 alpha: 1];
    NSColor* strokeColor = [NSColor colorWithRed: 1 green: 1 blue: 1 alpha: 1];
    NSColor* fillColor4 = [NSColor colorWithRed: 0 green: 0 blue: 0 alpha: 0.62];
    NSColor* fillColor5 = [NSColor colorWithRed: 0.188 green: 0.18 blue: 0.192 alpha: 1];
    NSColor* fillColor6 = [NSColor colorWithRed: 0.769 green: 0.761 blue: 0.769 alpha: 1];
    NSColor* textForeground = [NSColor colorWithRed: 1 green: 1 blue: 1 alpha: 1];
    NSColor* fillColor7 = [NSColor colorWithRed: 1 green: 1 blue: 1 alpha: 1];
    NSColor* fillColor8 = [NSColor colorWithRed: 0 green: 0 blue: 0.753 alpha: 1];
    NSColor* fillColor9 = [NSColor colorWithRed: 0 green: 0.753 blue: 0 alpha: 1];
    NSColor* fillColor10 = [NSColor colorWithRed: 0.753 green: 0 blue: 0 alpha: 1];


    //// Subframes
    NSRect background = NSMakeRect(NSMinX(frame), NSMinY(frame), frame.size.width, frame.size.height);
    NSRect regularText = NSMakeRect(NSMinX(frame) + floor((frame.size.width - 274.23) * 0.04061 - 0.34) + 0.84, NSMinY(frame) + floor((frame.size.height - 352.53) * 0.42711 - 0.45) + 0.95, 274.23, 352.53);
    NSRect rGB = NSMakeRect(NSMinX(frame), NSMinY(frame) - 0.98, floor((frame.size.width) * 0.03223 + 0.24) + 0.26, floor((frame.size.height + 0.98) * 0.02185 - 1.25) + 1.75);
    NSRect topRight = NSMakeRect(NSMinX(frame) + frame.size.width - 99.46, NSMinY(frame) + 0.04, 93.42, 93.52);
    NSRect bottomLeft = NSMakeRect(NSMinX(frame) + 1.75, NSMinY(frame) + frame.size.height - 94.55, 93.42, 93.51);


    //// Background
    {
        //// Rectangle Drawing
        NSBezierPath* rectanglePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(background) + floor(background.size.width * 0.00000 + 0.5), NSMinY(background) + floor(background.size.height * 0.00000 + 0.5), floor(background.size.width * 1.00000 + 0.5) - floor(background.size.width * 0.00000 + 0.5), floor(background.size.height * 0.24874 + 0.41) - floor(background.size.height * 0.00000 + 0.5) + 0.09)];
        [fillColor setFill];
        [rectanglePath fill];


        //// Rectangle 2 Drawing
        NSBezierPath* rectangle2Path = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(background) + floor(background.size.width * 0.00000 + 0.5), NSMinY(background) + floor(background.size.height * 0.24874 + 0.41) + 0.09, floor(background.size.width * 1.00000 + 0.5) - floor(background.size.width * 0.00000 + 0.5), floor(background.size.height * 1.00000 + 0.5) - floor(background.size.height * 0.24874 + 0.41) - 0.09)];
        [fillColor setFill];
        [rectangle2Path fill];


        //// Bezier Drawing
        NSBezierPath* bezierPath = [NSBezierPath bezierPath];
        [bezierPath moveToPoint: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 0.49645 * background.size.height)];
        [bezierPath curveToPoint: NSMakePoint(NSMinX(background) + 0.71753 * background.size.width, NSMinY(background) + 0.62275 * background.size.height) controlPoint1: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 0.48916 * background.size.height) controlPoint2: NSMakePoint(NSMinX(background) + 0.71753 * background.size.width, NSMinY(background) + 0.62275 * background.size.height)];
        [bezierPath curveToPoint: NSMakePoint(NSMinX(background) + 0.00000 * background.size.width, NSMinY(background) + 0.28446 * background.size.height) controlPoint1: NSMakePoint(NSMinX(background) + 0.71753 * background.size.width, NSMinY(background) + 0.62275 * background.size.height) controlPoint2: NSMakePoint(NSMinX(background) + 0.15141 * background.size.width, NSMinY(background) + 0.36204 * background.size.height)];
        [bezierPath lineToPoint: NSMakePoint(NSMinX(background) + 0.00000 * background.size.width, NSMinY(background) + 0.24823 * background.size.height)];
        [bezierPath lineToPoint: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 0.24823 * background.size.height)];
        [bezierPath lineToPoint: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 0.49645 * background.size.height)];
        [bezierPath closePath];
        [fillColor2 setFill];
        [bezierPath fill];


        //// Bezier 2 Drawing
        NSBezierPath* bezier2Path = [NSBezierPath bezierPath];
        [bezier2Path moveToPoint: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 0.49645 * background.size.height)];
        [bezier2Path lineToPoint: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 1.00000 * background.size.height)];
        [bezier2Path lineToPoint: NSMakePoint(NSMinX(background) + 0.00000 * background.size.width, NSMinY(background) + 1.00000 * background.size.height)];
        [bezier2Path lineToPoint: NSMakePoint(NSMinX(background) + 0.00000 * background.size.width, NSMinY(background) + 0.78019 * background.size.height)];
        [bezier2Path lineToPoint: NSMakePoint(NSMinX(background) + 1.00000 * background.size.width, NSMinY(background) + 0.49645 * background.size.height)];
        [bezier2Path closePath];
        [fillColor3 setFill];
        [bezier2Path fill];
    }


    //// Center
    {
        //// Oval Drawing
        NSBezierPath* ovalPath = [NSBezierPath bezierPathWithOvalInRect: NSMakeRect(NSMinX(center) + floor(center.size.width * 0.00000 + 0.5), NSMinY(center) + floor(center.size.height * 0.00000 + 0.5), floor(center.size.width * 1.00000 + 0.5) - floor(center.size.width * 0.00000 + 0.5), floor(center.size.height * 1.00000 + 0.5) - floor(center.size.height * 0.00000 + 0.5))];
        [fillColor4 setFill];
        [ovalPath fill];
        [strokeColor setStroke];
        ovalPath.lineWidth = 2;
        [ovalPath stroke];


        //// Rectangle 3 Drawing
        NSBezierPath* rectangle3Path = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(center) + floor(center.size.width * 0.15572 + 0.5), NSMinY(center) + floor(center.size.height * 0.15572 + 0.5), floor(center.size.width * 0.84428 + 0.5) - floor(center.size.width * 0.15572 + 0.5), floor(center.size.height * 0.84428 + 0.5) - floor(center.size.height * 0.15572 + 0.5))];
        [strokeColor setStroke];
        rectangle3Path.lineWidth = 2;
        [rectangle3Path stroke];


        //// Oval 2 Drawing
        NSBezierPath* oval2Path = [NSBezierPath bezierPathWithOvalInRect: NSMakeRect(NSMinX(center) + floor(center.size.width * 0.37715 + 0.48) + 0.02, NSMinY(center) + floor(center.size.height * 0.37715 + 0.48) + 0.02, floor(center.size.width * 0.62285 - 0.48) - floor(center.size.width * 0.37715 + 0.48) + 0.96, floor(center.size.height * 0.62285 - 0.48) - floor(center.size.height * 0.37715 + 0.48) + 0.96)];
        [fillColor5 setFill];
        [oval2Path fill];
        [strokeColor setStroke];
        oval2Path.lineWidth = 3.47;
        [oval2Path stroke];


        //// Bezier 3 Drawing
        NSBezierPath* bezier3Path = [NSBezierPath bezierPath];
        [bezier3Path moveToPoint: NSMakePoint(NSMinX(center) + 0.43774 * center.size.width, NSMinY(center) + 0.43569 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.43777 * center.size.width, NSMinY(center) + 0.43553 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.47053 * center.size.width, NSMinY(center) + 0.39283 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.44169 * center.size.width, NSMinY(center) + 0.41708 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.45373 * center.size.width, NSMinY(center) + 0.40139 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.46134 * center.size.width, NSMinY(center) + 0.40126 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.46749 * center.size.width, NSMinY(center) + 0.39584 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.46401 * center.size.width, NSMinY(center) + 0.39809 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.46106 * center.size.width, NSMinY(center) + 0.40157 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.44863 * center.size.width, NSMinY(center) + 0.44638 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.45011 * center.size.width, NSMinY(center) + 0.41373 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.44551 * center.size.width, NSMinY(center) + 0.43032 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.49915 * center.size.width, NSMinY(center) + 0.48885 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.45226 * center.size.width, NSMinY(center) + 0.46965 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.47476 * center.size.width, NSMinY(center) + 0.48919 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.54568 * center.size.width, NSMinY(center) + 0.46244 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.51803 * center.size.width, NSMinY(center) + 0.48969 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.53644 * center.size.width, NSMinY(center) + 0.47885 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.54557 * center.size.width, NSMinY(center) + 0.46243 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.59607 * center.size.width, NSMinY(center) + 0.48985 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.56578 * center.size.width, NSMinY(center) + 0.46314 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.58447 * center.size.width, NSMinY(center) + 0.47329 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.60657 * center.size.width, NSMinY(center) + 0.51871 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.60175 * center.size.width, NSMinY(center) + 0.49825 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.60626 * center.size.width, NSMinY(center) + 0.50827 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.60649 * center.size.width, NSMinY(center) + 0.51837 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.58067 * center.size.width, NSMinY(center) + 0.48533 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.60272 * center.size.width, NSMinY(center) + 0.50431 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.59340 * center.size.width, NSMinY(center) + 0.49239 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.58087 * center.size.width, NSMinY(center) + 0.48544 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.50993 * center.size.width, NSMinY(center) + 0.50579 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.55566 * center.size.width, NSMinY(center) + 0.47147 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.52390 * center.size.width, NSMinY(center) + 0.48058 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.50541 * center.size.width, NSMinY(center) + 0.51670 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.50801 * center.size.width, NSMinY(center) + 0.50924 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.50649 * center.size.width, NSMinY(center) + 0.51291 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.50550 * center.size.width, NSMinY(center) + 0.51637 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.50977 * center.size.width, NSMinY(center) + 0.55699 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.50161 * center.size.width, NSMinY(center) + 0.52995 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.50314 * center.size.width, NSMinY(center) + 0.54451 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.50940 * center.size.width, NSMinY(center) + 0.55691 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.46769 * center.size.width, NSMinY(center) + 0.58611 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.50026 * center.size.width, NSMinY(center) + 0.57211 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.48510 * center.size.width, NSMinY(center) + 0.58273 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.42781 * center.size.width, NSMinY(center) + 0.58115 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.45419 * center.size.width, NSMinY(center) + 0.58894 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.44020 * center.size.width, NSMinY(center) + 0.58679 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.46195 * center.size.width, NSMinY(center) + 0.58102 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.43890 * center.size.width, NSMinY(center) + 0.58437 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.45094 * center.size.width, NSMinY(center) + 0.58490 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.46219 * center.size.width, NSMinY(center) + 0.58094 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.49448 * center.size.width, NSMinY(center) + 0.54921 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.47719 * center.size.width, NSMinY(center) + 0.57578 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.48905 * center.size.width, NSMinY(center) + 0.56412 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.48799 * center.size.width, NSMinY(center) + 0.50104 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.50027 * center.size.width, NSMinY(center) + 0.53367 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.49801 * center.size.width, NSMinY(center) + 0.51472 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.48819 * center.size.width, NSMinY(center) + 0.50132 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.45603 * center.size.width, NSMinY(center) + 0.48003 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.48055 * center.size.width, NSMinY(center) + 0.49041 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.46906 * center.size.width, NSMinY(center) + 0.48280 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.44359 * center.size.width, NSMinY(center) + 0.47857 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.45180 * center.size.width, NSMinY(center) + 0.47927 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.44769 * center.size.width, NSMinY(center) + 0.47895 * center.size.height)];
        [bezier3Path curveToPoint: NSMakePoint(NSMinX(center) + 0.43794 * center.size.width, NSMinY(center) + 0.43581 * center.size.height) controlPoint1: NSMakePoint(NSMinX(center) + 0.43705 * center.size.width, NSMinY(center) + 0.46540 * center.size.height) controlPoint2: NSMakePoint(NSMinX(center) + 0.43465 * center.size.width, NSMinY(center) + 0.45011 * center.size.height)];
        [bezier3Path lineToPoint: NSMakePoint(NSMinX(center) + 0.43774 * center.size.width, NSMinY(center) + 0.43569 * center.size.height)];
        [bezier3Path closePath];
        [fillColor6 setFill];
        [bezier3Path fill];
    }


    if (showText)
    {
        //// MirroredText
        {
            [NSGraphicsContext saveGraphicsState];
            CGContextTranslateCTM(context, NSMinX(frame) + 0.96057 * frame.size.width, NSMinY(frame) + 0.42824 * frame.size.height);
            CGContextScaleCTM(context, -1, 1);



            //// Label Drawing
            NSRect labelRect = NSMakeRect(-0.15, -30.85, 264.59, 40);
            NSMutableParagraphStyle* labelStyle = [[NSMutableParagraphStyle alloc] init];
            labelStyle.alignment = NSTextAlignmentLeft;
            NSDictionary* labelFontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: labelStyle};

            [@"OBS Virtual Cam " drawInRect: NSOffsetRect(labelRect, 0, 0) withAttributes: labelFontAttributes];


            //// Label 2 Drawing
            NSRect label2Rect = NSMakeRect(-0.15, 7.75, 264.68, 40);
            NSMutableParagraphStyle* label2Style = [[NSMutableParagraphStyle alloc] init];
            label2Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label2FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label2Style};

            [@"is inactive." drawInRect: NSOffsetRect(label2Rect, 0, 0) withAttributes: label2FontAttributes];


            //// Label 3 Drawing
            NSRect label3Rect = NSMakeRect(-0.15, 84.95, 245.51, 39);
            NSMutableParagraphStyle* label3Style = [[NSMutableParagraphStyle alloc] init];
            label3Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label3FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label3Style};

            [@"Choose Tools > " drawInRect: NSOffsetRect(label3Rect, 0, 0) withAttributes: label3FontAttributes];


            //// Label 4 Drawing
            NSRect label4Rect = NSMakeRect(-0.15, 123.55, 269.53, 39);
            NSMutableParagraphStyle* label4Style = [[NSMutableParagraphStyle alloc] init];
            label4Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label4FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label4Style};

            [@"Start Virtual " drawInRect: NSOffsetRect(label4Rect, 0, 0) withAttributes: label4FontAttributes];


            //// Label 5 Drawing
            NSRect label5Rect = NSMakeRect(-0.15, 162.15, 126.45, 39);
            NSMutableParagraphStyle* label5Style = [[NSMutableParagraphStyle alloc] init];
            label5Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label5FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label5Style};

            [@"Camera." drawInRect: NSOffsetRect(label5Rect, 0, 0) withAttributes: label5FontAttributes];


            //// Label 6 Drawing
            NSRect label6Rect = NSMakeRect(-0.15, -152.38, 296.53, 81);
            NSMutableParagraphStyle* label6Style = [[NSMutableParagraphStyle alloc] init];
            label6Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label6FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 66], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label6Style};

            [@"Mirrorred" drawInRect: NSOffsetRect(label6Rect, 0, 0) withAttributes: label6FontAttributes];



            [NSGraphicsContext restoreGraphicsState];
        }


        //// RegularText
        {
            //// Label 7 Drawing
            NSRect label7Rect = NSMakeRect(NSMinX(regularText) + 4.7, NSMinY(regularText) + 121.53, 264.59, 40);
            NSMutableParagraphStyle* label7Style = [[NSMutableParagraphStyle alloc] init];
            label7Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label7FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label7Style};

            [@"OBS Virtual Cam " drawInRect: NSOffsetRect(label7Rect, 0, 0) withAttributes: label7FontAttributes];


            //// Label 8 Drawing
            NSRect label8Rect = NSMakeRect(NSMinX(regularText) + 4.7, NSMinY(regularText) + 160.13, 269.46, 39);
            NSMutableParagraphStyle* label8Style = [[NSMutableParagraphStyle alloc] init];
            label8Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label8FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label8Style};

            [@"is inactive." drawInRect: NSOffsetRect(label8Rect, 0, 0) withAttributes: label8FontAttributes];


            //// Label 9 Drawing
            NSRect label9Rect = NSMakeRect(NSMinX(regularText) + 4.7, NSMinY(regularText) + 236.33, 245.51, 39);
            NSMutableParagraphStyle* label9Style = [[NSMutableParagraphStyle alloc] init];
            label9Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label9FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label9Style};

            [@"Choose Tools > " drawInRect: NSOffsetRect(label9Rect, 0, 0) withAttributes: label9FontAttributes];


            //// Label 10 Drawing
            NSRect label10Rect = NSMakeRect(NSMinX(regularText) + 4.7, NSMinY(regularText) + 274.93, 269.53, 39);
            NSMutableParagraphStyle* label10Style = [[NSMutableParagraphStyle alloc] init];
            label10Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label10FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label10Style};

            [@"Start Virtual " drawInRect: NSOffsetRect(label10Rect, 0, 0) withAttributes: label10FontAttributes];


            //// Label 11 Drawing
            NSRect label11Rect = NSMakeRect(NSMinX(regularText) + 4.7, NSMinY(regularText) + 313.53, 126.45, 39);
            NSMutableParagraphStyle* label11Style = [[NSMutableParagraphStyle alloc] init];
            label11Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label11FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 32], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label11Style};

            [@"Camera." drawInRect: NSOffsetRect(label11Rect, 0, 0) withAttributes: label11FontAttributes];


            //// Label 12 Drawing
            NSRect label12Rect = NSMakeRect(NSMinX(regularText), NSMinY(regularText), 248.31, 81);
            NSMutableParagraphStyle* label12Style = [[NSMutableParagraphStyle alloc] init];
            label12Style.alignment = NSTextAlignmentLeft;
            NSDictionary* label12FontAttributes = @{NSFontAttributeName: [NSFont fontWithName: @"Helvetica-Bold" size: 66], NSForegroundColorAttributeName: textForeground, NSParagraphStyleAttributeName: label12Style};

            [@"Regular" drawInRect: NSOffsetRect(label12Rect, 0, 0) withAttributes: label12FontAttributes];
        }
    }


    //// RGB
    {
        //// Blue Drawing
        NSBezierPath* bluePath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rGB) + floor(rGB.size.width * 0.67475 - 0.34) + 0.84, NSMinY(rGB) + floor(rGB.size.height * 0.00000 + 0.5), floor(rGB.size.width * 1.00000 + 0.24) - floor(rGB.size.width * 0.67475 - 0.34) - 0.58, floor(rGB.size.height * 1.00000 - 0.25) - floor(rGB.size.height * 0.00000 + 0.5) + 0.75)];
        [fillColor8 setFill];
        [bluePath fill];


        //// Green Drawing
        NSBezierPath* greenPath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rGB) + floor(rGB.size.width * 0.32525 + 0.08) + 0.42, NSMinY(rGB) + floor(rGB.size.height * 0.00000 + 0.5), floor(rGB.size.width * 0.67475 - 0.34) - floor(rGB.size.width * 0.32525 + 0.08) + 0.42, floor(rGB.size.height * 1.00000 - 0.25) - floor(rGB.size.height * 0.00000 + 0.5) + 0.75)];
        [fillColor9 setFill];
        [greenPath fill];


        //// Red Drawing
        NSBezierPath* redPath = [NSBezierPath bezierPathWithRect: NSMakeRect(NSMinX(rGB) + floor(rGB.size.width * 0.00000 + 0.5), NSMinY(rGB) + floor(rGB.size.height * 0.00000 + 0.5), floor(rGB.size.width * 0.32525 + 0.08) - floor(rGB.size.width * 0.00000 + 0.5) + 0.42, floor(rGB.size.height * 1.00000 - 0.25) - floor(rGB.size.height * 0.00000 + 0.5) + 0.75)];
        [fillColor10 setFill];
        [redPath fill];
    }


    //// TopRight
    {
        //// Bezier 7 Drawing
        NSBezierPath* bezier7Path = [NSBezierPath bezierPath];
        [bezier7Path moveToPoint: NSMakePoint(NSMinX(topRight) + 31.28, NSMinY(topRight) + 54.38)];
        [bezier7Path curveToPoint: NSMakePoint(NSMinX(topRight) + 23.82, NSMinY(topRight) + 61.91) controlPoint1: NSMakePoint(NSMinX(topRight) + 26.23, NSMinY(topRight) + 55.91) controlPoint2: NSMakePoint(NSMinX(topRight) + 25.34, NSMinY(topRight) + 56.81)];
        [bezier7Path lineToPoint: NSMakePoint(NSMinX(topRight), NSMinY(topRight) + 61.91)];
        [bezier7Path lineToPoint: NSMakePoint(NSMinX(topRight) + 30.91, NSMinY(topRight))];
        [bezier7Path lineToPoint: NSMakePoint(NSMinX(topRight) + 31.28, NSMinY(topRight) + 0.08)];
        [bezier7Path lineToPoint: NSMakePoint(NSMinX(topRight) + 31.28, NSMinY(topRight) + 54.38)];
        [bezier7Path closePath];
        [fillColor7 setFill];
        [bezier7Path fill];


        //// Bezier 8 Drawing
        NSBezierPath* bezier8Path = [NSBezierPath bezierPath];
        [bezier8Path moveToPoint: NSMakePoint(NSMinX(topRight) + 93.42, NSMinY(topRight) + 62.55)];
        [bezier8Path lineToPoint: NSMakePoint(NSMinX(topRight) + 31.61, NSMinY(topRight) + 93.52)];
        [bezier8Path lineToPoint: NSMakePoint(NSMinX(topRight) + 31.61, NSMinY(topRight) + 69.52)];
        [bezier8Path curveToPoint: NSMakePoint(NSMinX(topRight) + 38.98, NSMinY(topRight) + 62.1) controlPoint1: NSMakePoint(NSMinX(topRight) + 35.92, NSMinY(topRight) + 68.97) controlPoint2: NSMakePoint(NSMinX(topRight) + 38.42, NSMinY(topRight) + 66.47)];
        [bezier8Path lineToPoint: NSMakePoint(NSMinX(topRight) + 93.28, NSMinY(topRight) + 62.1)];
        [bezier8Path lineToPoint: NSMakePoint(NSMinX(topRight) + 93.42, NSMinY(topRight) + 62.55)];
        [bezier8Path closePath];
        [fillColor7 setFill];
        [bezier8Path fill];


        //// Bezier 9 Drawing
        NSBezierPath* bezier9Path = [NSBezierPath bezierPath];
        [bezier9Path moveToPoint: NSMakePoint(NSMinX(topRight) + 31.54, NSMinY(topRight) + 65.21)];
        [bezier9Path curveToPoint: NSMakePoint(NSMinX(topRight) + 28.38, NSMinY(topRight) + 62.07) controlPoint1: NSMakePoint(NSMinX(topRight) + 29.66, NSMinY(topRight) + 64.99) controlPoint2: NSMakePoint(NSMinX(topRight) + 28.39, NSMinY(topRight) + 64.01)];
        [bezier9Path lineToPoint: NSMakePoint(NSMinX(topRight) + 28.38, NSMinY(topRight) + 62.05)];
        [bezier9Path curveToPoint: NSMakePoint(NSMinX(topRight) + 31.52, NSMinY(topRight) + 59) controlPoint1: NSMakePoint(NSMinX(topRight) + 28.41, NSMinY(topRight) + 60.34) controlPoint2: NSMakePoint(NSMinX(topRight) + 29.82, NSMinY(topRight) + 58.97)];
        [bezier9Path curveToPoint: NSMakePoint(NSMinX(topRight) + 34.56, NSMinY(topRight) + 61.94) controlPoint1: NSMakePoint(NSMinX(topRight) + 33.15, NSMinY(topRight) + 59.03) controlPoint2: NSMakePoint(NSMinX(topRight) + 34.47, NSMinY(topRight) + 60.31)];
        [bezier9Path curveToPoint: NSMakePoint(NSMinX(topRight) + 31.54, NSMinY(topRight) + 65.21) controlPoint1: NSMakePoint(NSMinX(topRight) + 34.65, NSMinY(topRight) + 63.82) controlPoint2: NSMakePoint(NSMinX(topRight) + 33.42, NSMinY(topRight) + 64.86)];
        [bezier9Path closePath];
        [fillColor7 setFill];
        [bezier9Path fill];
    }


    //// BottomLeft
    {
        //// Bezier 4 Drawing
        NSBezierPath* bezier4Path = [NSBezierPath bezierPath];
        [bezier4Path moveToPoint: NSMakePoint(NSMinX(bottomLeft) + 62.14, NSMinY(bottomLeft) + 39.13)];
        [bezier4Path curveToPoint: NSMakePoint(NSMinX(bottomLeft) + 69.6, NSMinY(bottomLeft) + 31.6) controlPoint1: NSMakePoint(NSMinX(bottomLeft) + 67.18, NSMinY(bottomLeft) + 37.6) controlPoint2: NSMakePoint(NSMinX(bottomLeft) + 68.08, NSMinY(bottomLeft) + 36.71)];
        [bezier4Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 93.42, NSMinY(bottomLeft) + 31.6)];
        [bezier4Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 62.51, NSMinY(bottomLeft) + 93.51)];
        [bezier4Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 62.14, NSMinY(bottomLeft) + 93.43)];
        [bezier4Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 62.14, NSMinY(bottomLeft) + 39.13)];
        [bezier4Path closePath];
        [fillColor7 setFill];
        [bezier4Path fill];


        //// Bezier 5 Drawing
        NSBezierPath* bezier5Path = [NSBezierPath bezierPath];
        [bezier5Path moveToPoint: NSMakePoint(NSMinX(bottomLeft), NSMinY(bottomLeft) + 30.96)];
        [bezier5Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 61.81, NSMinY(bottomLeft))];
        [bezier5Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 61.81, NSMinY(bottomLeft) + 24.02)];
        [bezier5Path curveToPoint: NSMakePoint(NSMinX(bottomLeft) + 54.44, NSMinY(bottomLeft) + 31.44) controlPoint1: NSMakePoint(NSMinX(bottomLeft) + 57.49, NSMinY(bottomLeft) + 24.57) controlPoint2: NSMakePoint(NSMinX(bottomLeft) + 54.99, NSMinY(bottomLeft) + 27.07)];
        [bezier5Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 0.14, NSMinY(bottomLeft) + 31.44)];
        [bezier5Path lineToPoint: NSMakePoint(NSMinX(bottomLeft), NSMinY(bottomLeft) + 30.96)];
        [bezier5Path closePath];
        [fillColor7 setFill];
        [bezier5Path fill];


        //// Bezier 6 Drawing
        NSBezierPath* bezier6Path = [NSBezierPath bezierPath];
        [bezier6Path moveToPoint: NSMakePoint(NSMinX(bottomLeft) + 61.88, NSMinY(bottomLeft) + 28.3)];
        [bezier6Path curveToPoint: NSMakePoint(NSMinX(bottomLeft) + 65.04, NSMinY(bottomLeft) + 31.45) controlPoint1: NSMakePoint(NSMinX(bottomLeft) + 63.75, NSMinY(bottomLeft) + 28.53) controlPoint2: NSMakePoint(NSMinX(bottomLeft) + 65.03, NSMinY(bottomLeft) + 29.51)];
        [bezier6Path lineToPoint: NSMakePoint(NSMinX(bottomLeft) + 65.04, NSMinY(bottomLeft) + 31.47)];
        [bezier6Path curveToPoint: NSMakePoint(NSMinX(bottomLeft) + 61.89, NSMinY(bottomLeft) + 34.52) controlPoint1: NSMakePoint(NSMinX(bottomLeft) + 65.01, NSMinY(bottomLeft) + 33.18) controlPoint2: NSMakePoint(NSMinX(bottomLeft) + 63.6, NSMinY(bottomLeft) + 34.55)];
        [bezier6Path curveToPoint: NSMakePoint(NSMinX(bottomLeft) + 58.86, NSMinY(bottomLeft) + 31.58) controlPoint1: NSMakePoint(NSMinX(bottomLeft) + 60.27, NSMinY(bottomLeft) + 34.49) controlPoint2: NSMakePoint(NSMinX(bottomLeft) + 58.95, NSMinY(bottomLeft) + 33.21)];
        [bezier6Path curveToPoint: NSMakePoint(NSMinX(bottomLeft) + 61.88, NSMinY(bottomLeft) + 28.3) controlPoint1: NSMakePoint(NSMinX(bottomLeft) + 58.77, NSMinY(bottomLeft) + 29.7) controlPoint2: NSMakePoint(NSMinX(bottomLeft) + 60, NSMinY(bottomLeft) + 28.66)];
        [bezier6Path closePath];
        [fillColor7 setFill];
        [bezier6Path fill];
    }
}

void DrawDialWithFrame(NSRect frame, CGFloat rotation)
{
    //// General Declarations
    CGContextRef context = NSGraphicsContext.currentContext.CGContext;

    //// Oval 3 Drawing
    NSBezierPath* oval3Path = [NSBezierPath bezierPathWithOvalInRect: NSMakeRect(NSMinX(frame) + frame.size.width - 133, NSMinY(frame) + 30, 98, 98)];
    [NSColor.grayColor setFill];
    [oval3Path fill];


    //// Bezier 10 Drawing
    [NSGraphicsContext saveGraphicsState];
    CGContextTranslateCTM(context, NSMaxX(frame) - 83.5, NSMinY(frame) + 79.5);
    CGContextRotateCTM(context, rotation * M_PI/180);

    NSBezierPath* bezier10Path = [NSBezierPath bezierPath];
    [bezier10Path moveToPoint: NSMakePoint(-0, -0)];
    [bezier10Path lineToPoint: NSMakePoint(-0, 48)];
    [NSColor.blackColor setStroke];
    bezier10Path.lineWidth = 2;
    [bezier10Path stroke];

    [NSGraphicsContext restoreGraphicsState];
}


NSImage *ImageOfTestCardWithSize(NSSize imageSize)
{
    return [NSImage imageWithSize: imageSize flipped: YES drawingHandler: ^(__unused NSRect dstRect)
    {
        DrawTestCardWithFrame(nil, NSMakeRect(0, 0, imageSize.width, imageSize.height));
        return YES;
    }];
}
