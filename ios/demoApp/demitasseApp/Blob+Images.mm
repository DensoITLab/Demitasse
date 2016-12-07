//
//  Blob+Images.mm
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/04/05.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Blob+Images.h"

@implementation Blob (Images)

+ (Blob*)createBlobFromCIImage:(CIImage*)ciimage size:(CGSize)newSize orientation:(UIImageOrientation)orientation normalize:(BOOL)normalize
{
    CGRect  imageRect = [ciimage extent];

    CGFloat ratio = 1.0;

    if (imageRect.size.width > imageRect.size.height) {
        ratio = newSize.height / imageRect.size.height;
    } else {
        ratio = newSize.width / imageRect.size.width;
    }

    // resize and centering, clop
    CIImage *resizedImage = [ciimage imageByApplyingTransform:CGAffineTransformMakeScale(ratio, ratio)];
    CGRect   resizedRect  = [resizedImage extent];
    CGRect   cropRect = CGRectMake((resizedRect.size.width  - newSize.width)  * 0.5,
                                   (resizedRect.size.height - newSize.height) * 0.5,
                                   newSize.width, newSize.height);

    CIImage *croppedImage = [resizedImage imageByCroppingToRect:cropRect];

    int rotate = 6; // UIImageOrientationRight -> Right, Top

    switch (orientation) {
        case UIImageOrientationDown:
            rotate = 3; // Bottom, Rigth
            break;

        case UIImageOrientationLeft:
            rotate = 8; // Left, Bottom
            break;

        case UIImageOrientationUp:
            rotate = 1; // Top, Left
            break;

        case UIImageOrientationRight:
        default:
            rotate = 6; // Right, Top
            break;
    }

    CIImage *rotatedImage = [croppedImage imageByApplyingOrientation:rotate];

    // create memory buffer and draw CIImage
    size_t   bitsPerComponent = 8;
    size_t   bytesPerRow      = 4 * newSize.width;
    size_t   bufferSize       = bytesPerRow * newSize.height;
    UInt8   *buffer           = (UInt8*)malloc(bufferSize);

    CGBitmapInfo    bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

    CGContextRef  cgContext   = CGBitmapContextCreate(buffer,
                                                      newSize.width,
                                                      newSize.height,
                                                      bitsPerComponent,
                                                      bytesPerRow,
                                                      colorSpace,
                                                      bitmapInfo);

    CIContext    *ciContext   = [CIContext contextWithCGContext:cgContext options:nil];

    [ciContext drawImage: rotatedImage
                  inRect: CGRectMake(0, 0, newSize.width, newSize.height)
                fromRect: CGRectMake(0, 0, newSize.width, newSize.height)];

    // create blob and copy pixcel data
    Blob  *blob = [[Blob alloc] init:BlobTypeFloat ext0:3 ext1:224 ext2:224 ext3:1];
    float *p    = blob.rawFloatData;
    UInt8 *d    = buffer;

    for (int j = 0; j < 224; ++j) {
        for (int i = 0; i < 224; ++i) {
            float b = *(d + 0);
            float g = *(d + 1);
            float r = *(d + 2);

            if (normalize) {
                b = b / 255.0;
                r = r / 255.0;
                g = g / 255.0;
            }

            *p     = b; // B
            *(p+1) = g; // G
            *(p+2) = r; // R

            p += 3;  // BGR
            d += 4;  // BGRA
        }
    }

    CGContextRelease(cgContext);
    CGColorSpaceRelease(colorSpace);

    free((void*)buffer);

    return blob;
}


- (NSArray*)getTopNIndex:(int)max
{
    if (self.dimensions > 2 || [self extent:0] < 1) {
        return nil;
    }

    NSMutableArray *result = [[NSMutableArray alloc] init];

    float *p    = self.rawFloatData;
    int    size = [self extent:0];

    [result addObject:[NSNumber numberWithInt:0]];

    for (int i = 1; i < size; ++i) {
        [result enumerateObjectsUsingBlock:^(NSNumber* obj, NSUInteger idx, BOOL *stop) {
            float value = p[[obj intValue]];
            if (value < p[i]) {
                [result insertObject:[NSNumber numberWithFloat:i] atIndex:idx];
                *stop = YES;
            }
        }];

        if ([result count] > max) {
            [result removeLastObject];
        }
    }

    return result;
}

- (void)subBlob:(const Blob*)blob
{
    float *m    = blob.rawFloatData;
    float *p    = self.rawFloatData;

    int    size = [blob extent:0] * [blob extent:1] * [blob extent:2];

    for (int i = 0; i < size; ++i) {
        *p = *p - *m;
        p++;
        m++;
    }
}



@end
