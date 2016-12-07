//
//  blob.mm
//  demitasseLib
//
//  Created by Satoshi Kondo on 2016/04/01.
//  Copyright © 2016年 DENSO IT LABORATORY, INC. All rights reserved.
//

#import "blob.h"
#include "blob.hpp"


@implementation Blob {
    bool         hasOwnedObj_;
    demitasse::Blob *obj_;
}

- (demitasse::Blob::BlobType)blobType:(BlobType)type {
    demitasse::Blob::BlobType aType = demitasse::Blob::BlobType::Float;
    
    if (type == BlobTypeFloat) {
        aType = demitasse::Blob::BlobType::Float;
    } else if (type == BlobTypeBinary) {
        aType = demitasse::Blob::BlobType::Binary;
    } else if (type == BlobTypeTernary) {
        aType = demitasse::Blob::BlobType::Ternary;
    }
    
    return aType;
}

- (nullable id)initWithConstBlob:(nonnull const demitasse::Blob *)newObj
{
    if (self = [super init]) {
        obj_         = (demitasse::Blob *)newObj;
        hasOwnedObj_ = false;
    }

    return self;
}

- (nullable id)initWithBlob:(nonnull demitasse::Blob *)newObj
{
    if (self = [super init]) {
        obj_         = newObj;
        hasOwnedObj_ = true;
    }
    return self;
}

- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0 ext1:(unsigned int)ext1 ext2:(unsigned int)ext2 ext3:(unsigned int)ext3
{
    if (self = [super init]) {
        demitasse::Blob::BlobType aType = [self blobType:type];
        obj_ = new demitasse::Blob(aType, ext0, ext1, ext2, ext3);
        if (obj_ == nil) {
            return nil;
        }
        hasOwnedObj_ = true;
    }
    
    return self;
}


- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0 ext1:(unsigned int)ext1 ext2:(unsigned int)ext2
{
    if (self = [super init]) {
        demitasse::Blob::BlobType aType = [self blobType:type];
        obj_ = new demitasse::Blob(aType, ext0, ext1, ext2);
        if (obj_ == nil) {
            return nil;
        }
        hasOwnedObj_ = true;
    }
    
    return self;
}


- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0 ext1:(unsigned int)ext1
{
    if (self = [super init]) {
        demitasse::Blob::BlobType aType = [self blobType:type];
        obj_ = new demitasse::Blob(aType, ext0, ext1);
        if (obj_ == nil) {
            return nil;
        }
        hasOwnedObj_ = true;
    }

    return self;
}


- (nullable id)init:(BlobType)type ext0:(unsigned int)ext0
{
    if (self = [super init]) {
        demitasse::Blob::BlobType aType = [self blobType:type];
        obj_ = new demitasse::Blob(aType, ext0);
        if (obj_ == nil) {
            return nil;
        }
        hasOwnedObj_ = true;
    }

    return self;
}


- (void)dealloc {
    if (hasOwnedObj_ && obj_ != nil) {
        delete obj_;
    }
}

- (nullable Blob *)createSameExtents
{
    Blob *aNewObj = nil;

    demitasse::Blob *newObj = obj_->createSameExtents();
    if (newObj != nil) {
        aNewObj = [[Blob alloc] initWithBlob:newObj];
    }

    return aNewObj;
}

- (nullable Blob *)copy
{
    Blob *aNewObj = nil;

    demitasse::Blob *newObj = obj_->copy();
    if (newObj != nil) {
        aNewObj = [[Blob alloc] initWithBlob:newObj];
    }

    return aNewObj;
}

- (nullable float *)rawFloatData {
    return obj_->rawData().RealSingle.data;
}

- (nullable uint8_t *)rawBinaryData {
    return obj_->rawData().Binary.data;
}

- (nullable uint8_t *)rawTernaryStar {
    return obj_->rawData().Ternary.star;
}

- (nullable uint8_t *)rawTernaryMask {
    return obj_->rawData().Ternary.mask;
}

- (nullable uint32_t *)rawTernaryNNZ {
    return obj_->rawData().Ternary.nnz;
}

- (unsigned int)dimensions {
    return obj_->dimensions();
}

- (unsigned int)extent:(unsigned int)dim {
    return obj_->extent(dim);
}

- (unsigned int)stride:(unsigned int)dim {
    return obj_->stride(dim);
}

- (size_t)rawDataBytes {
    return obj_->rawDataBytes();
}


- (float)getValue:(int)idx0 {
    float value = obj_->getValue(idx0, 0, 0, 0);
    return value;
}

- (float)getValue:(int)idx0 idx1:(int)idx1 {
    float value = obj_->getValue(idx0, idx1, 0, 0);
    return value;
}

- (float)getValue:(int)idx0 idx1:(int)idx1 idx2:(int)idx2 {
    float value = obj_->getValue(idx0, idx1, idx2, 0);
    return value;
}

- (float)getValue:(int)idx0 idx1:(int)idx1 idx2:(int)idx2 idx3:(int)idx3 {
    float value = obj_->getValue(idx0, idx1, idx2, idx3);
    return value;
}

- (bool)isEqualExtents:(const Blob *)base {
    return obj_->isEqualExtents(base->obj_);
}

- (nullable Blob *)copyAt:(int)index
{
    Blob *aNewObj = nil;

    demitasse::Blob *newObj = obj_->copyAt(index);
    if (newObj != nil) {
        aNewObj = [[Blob alloc] initWithBlob:newObj];
    }
    return aNewObj;
}

- (nullable Blob *)copyAt:(int)index num:(int)num
{
    Blob *aNewObj = nil;

    demitasse::Blob *newObj = obj_->copyAt(index, num);
    if (newObj != nil) {
        aNewObj = [[Blob alloc] initWithBlob:newObj];
    }

    return aNewObj;
}

@end
