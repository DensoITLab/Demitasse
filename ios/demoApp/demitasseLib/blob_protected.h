//
//  blob_protected.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_BLOB_PROTECTED__
#define __DEMITASSE_IOS_BLOB_PROTECTED__

#import "blob.h"

#include "blob.hpp"

@interface Blob() {
  @public demitasse::Blob *obj_;
}

- (nullable id)initWithBlob:(nonnull demitasse::Blob *)newObj;
- (nullable id)initWithConstBlob:(nonnull const demitasse::Blob *)newObj;

@end

#endif /* __DEMITASSE_IOS_BLOB_PROTECTED__ */
