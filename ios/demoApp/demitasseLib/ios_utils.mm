//
//  ios_utils.m
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/04.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ios_utils.h"

#include "blob_protected.h"
#include "utils.hpp"

@implementation demitasseUtils

+ (nullable Blob *)convertUIImage:(nonnull const UIImage *)image
{
    Blob *aImage = nil;
    
    // convert RGB -> BGR and value range from (0 ~ 255) to (0.0 ~ 1.0)
    
    return aImage;
}


+ (nullable Blob *)load_mean_image:(nonnull const NSString *)filename normalize:(BOOL)normalize
{
    Blob *aMean = nil;

    const std::string  name = [filename UTF8String];
    bool aNormalize = normalize;
    
    auto mean = demitasse::Utils::load_mean_image(name, aNormalize);
    if (mean != nil) {
        aMean = [[Blob alloc] initWithBlob:mean];
    }

    return aMean;
}

@end
