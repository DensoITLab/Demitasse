//
//  Blob+Images.h
//  demitasseApp
//
//  Created by Satoshi Kondo on 2016/04/05.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef Blob_Images_h
#define Blob_Images_h

#import <UIKit/UIKit.h>

#import "blob.h"

@interface Blob (Images)

+ (Blob*)createBlobFromCIImage:(CIImage*)image size:(CGSize)size orientation:(UIImageOrientation)orientation normalize:(BOOL)normalize;
- (NSArray*)getTopNIndex:(int)max;

- (void)subBlob:(const Blob*)blob; // used for sub mean

@end

#endif /* Blob_Images_h */
