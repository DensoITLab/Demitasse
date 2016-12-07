//
//  ios_utils.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/04.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_UTILS__
#define __DEMITASSE_IOS_UTILS__

#import <UIKit/UIKit.h>

#import "blob.h"

@interface demitasseUtils : NSObject

+ (nullable Blob *)convertUIImage:(nonnull const UIImage *)image;
+ (nullable Blob *)load_mean_image:(nonnull const NSString *)filename normalize:(BOOL)normalize;

@end

#endif /* __DEMITASSE_IOS_UTILS__ */
