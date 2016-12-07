//
//  blob.h
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#ifndef __DEMITASSE_IOS_BLOB__
#define __DEMITASSE_IOS_BLOB__

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, BlobType) {
    BlobTypeFloat   = 0, // real (single)
    BlobTypeBinary  = 1, // binary
    BlobTypeTernary = 2  // ternary
};

@interface Blob : NSObject

- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0 ext1:(unsigned int)ext1 ext2:(unsigned int)ext2 ext3:(unsigned int)ext3;
- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0 ext1:(unsigned int)ext1 ext2:(unsigned int)ext2;
- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0 ext1:(unsigned int)ext1;
- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0;
- (void)dealloc;

- (nullable Blob *)createSameExtents;
- (nullable Blob *)copy;

- (nullable float *)rawFloatData;
- (nullable uint8_t *)rawBinaryData;
- (nullable uint8_t *)rawTernaryStar;
- (nullable uint8_t *)rawTernaryMask;
- (nullable uint32_t *)rawTernaryNNZ;

- (unsigned int)dimensions;
- (unsigned int)extent:(unsigned int)dim;
- (unsigned int)stride:(unsigned int)dim;
- (size_t)rawDataBytes;

// float& operator()(int idx0, int idx1 = 0, int idx2 = 0, int idx3 = 0) const;
- (float)getValue:(int)idx0;
- (float)getValue:(int)idx0 idx1:(int)idx1;
- (float)getValue:(int)idx0 idx1:(int)idx1 idx2:(int)idx2;
- (float)getValue:(int)idx0 idx1:(int)idx1 idx2:(int)idx2 idx3:(int)idx3;

- (bool)isEqualExtents:(nonnull const Blob *)base;

- (nullable Blob *)copyAt:(int)index;
- (nullable Blob *)copyAt:(int)index num:(int)num;

@end

#endif /* __DEMITASSE_IOS_BLOB__ */
