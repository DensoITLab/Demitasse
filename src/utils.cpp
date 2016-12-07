/*
Copyright (C) 2016 Denso IT Laboratory, Inc.
All Rights Reserved

Denso IT Laboratory, Inc. retains sole and exclusive ownership of all
intellectual property rights including copyrights and patents related to this
Software.

Permission is hereby granted, free of charge, to any person obtaining a copy
of the Software and accompanying documentation to use, copy, modify, merge,
publish, or distribute the Software or software derived from it for
non-commercial purposes, such as academic study, education and personal use,
subject to the following conditions:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <string>
#include <iostream>
#include <fstream>

#include "blob.hpp"
#include "utils.hpp"

using demitasse::Utils;
using demitasse::Blob;

Blob* Utils::load_mean_image(const std::string& blobfile, bool normalize) {
  size_t buffsize    = 3 * 256 * 256; // mean blob size

  std::ifstream *ifs = new std::ifstream(blobfile, std::ios::binary);
  if (ifs == nullptr) {
    printf("Mean file is not exists.\n");
    return nullptr;
  }
  char* buff = (char*)malloc(buffsize * sizeof(float));
  ifs->read((char*)buff, buffsize * sizeof(float));
  ifs->close();

  auto   image = new Blob(Blob::BlobType::Float, 3, 256, 256);
  float* p     = (float*)buff;

  // RGB -> BGR
  for (int c = 0; c < 3; ++c) {
    for (int j = 0; j < 256; ++j) {
      for (int i = 0; i < 256; ++i) {
        if (normalize) {
          image->setValue(*p / (float)255.0, 2 - c, i, j);
        } else {
          image->setValue(*p, 2 - c, i, j);
        }
        p++;
      }
    }
  }

  free((char*)buff);

  auto mean = demitasse::Utils::resize_image(image, 224, 224);

  return mean;
}

Blob* Utils::resize_image(const Blob* img, int width, int height) {
  int    ch     = img->extent(0);
  int    max_x  = img->extent(1);
  int    max_y  = img->extent(2);

  Blob*  result = new Blob(Blob::BlobType::Float, ch, width, height);

  float* p      = result->rawData().RealSingle.data;

  float  wf     = (float)max_x / (float)width;
  float  hf     = (float)max_y / (float)height;

  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {

      int   xx  = (int)(floor(i * wf));
      int   yy  = (int)(floor(j * hf));

      float d10 = (i * wf) - xx;
      float d00 = 1.0f - d10;
      float d11 = (j * hf) - yy;
      float d01 = 1.0f - d11;

      // clamp
      int x0 = std::max(std::min(xx, max_x - 1), 0);
      int x1 = std::max(std::min(xx + 1, max_x - 1), 0);

      int y0 = std::max(std::min(yy, max_y - 1), 0);
      int y1 = std::max(std::min(yy + 1, max_y - 1), 0);

      for (int c = 0; c < ch; ++c) {
        float fx00 = img->getValue(c, x0, y0) * d00;
        float fx10 = img->getValue(c, x1, y0) * d10;
        float fx01 = img->getValue(c, x0, y1) * d00;
        float fx11 = img->getValue(c, x1, y1) * d10;

        *p = ((fx00 + fx10) * d01) + ((fx01 + fx11) * d11);
        p++;
      }
    }
  }

  return result;
}

#if USE_LIBPNG
#include "png.h"

Blob* Utils::load_png_image(const std::string& file_name, bool normalize) {
  Blob* image = nullptr;

  FILE* fp    = fopen(file_name.c_str(), "rb");
  if (!fp) {
    // can not open
    return nullptr;
  }

  // check file header
  png_byte header[8];
  if (fread(header, sizeof(header), 1, fp) != 1) {
    return nullptr;
  }

  if (png_sig_cmp((png_bytep)header, 0, sizeof(header))) {
    return nullptr;
  }

  // prepare structure
  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    return nullptr;
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    png_destroy_read_struct(&png, (png_infopp)NULL, (png_infopp)NULL);
    return nullptr;
  }

/*
   png_infop end_info = png_create_info_struct(png);
   if (!end_info) {
    png_destroy_read_struct(&png, &info, (png_infopp)NULL);
    return nullptr;
   }
 */

  // register longjmp handler
  if (setjmp(png_jmpbuf(png))) {
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return nullptr;
  }

  // set file stream
  png_init_io(png, fp);
  png_set_sig_bytes(png, sizeof(header));

  // read data
  int png_transforms = PNG_TRANSFORM_PACKING
                       | PNG_TRANSFORM_STRIP_16
                       | PNG_TRANSFORM_STRIP_ALPHA
                       | PNG_TRANSFORM_BGR;
  png_read_png(png, info, png_transforms, NULL);

  // image info and row pointers
  int width       = png_get_image_width(png, info);
  int height      = png_get_image_height(png, info);

  png_bytepp rows = png_get_rows(png, info);
  png_bytep  row  = nullptr;
  float*     p    = nullptr;
  switch (png_get_color_type(png, info)) {
  case PNG_COLOR_TYPE_GRAY:
    image = new demitasse::Blob(Blob::BlobType::Float, 1, width, height);
    p     = image->rawData().RealSingle.data;

    for (int j = 0; j < height; ++j) {
      row = rows[j];

      for (int i = 0; i < width; ++i) {
        *p = (float)(*row);
        if (normalize) {
          *p /= 255.0;   // 0 ~ 255 -> 0.0 ~ 1.0
        }
        ++row;
        ++p;
      }
    }
    break;

  case PNG_COLOR_TYPE_RGB:
    image = new Blob(Blob::BlobType::Float, 3, width, height);
    p     = image->rawData().RealSingle.data;
    for (int j = 0; j < height; ++j) {
      row = rows[j];

      for (int i = 0; i < width; ++i) {
        // B
        *p = (float)(*row);
        if (normalize) {
          *p /= 255.0;   // 0 ~ 255 -> 0.0 ~ 1.0
        }
        ++row;
        ++p;

        // G
        *p = (float)(*row);
        if (normalize) {
          *p /= 255.0;   // 0 ~ 255 -> 0.0 ~ 1.0
        }
        ++row;
        ++p;

        // R
        *p = (float)(*row);
        if (normalize) {
          *p /= 255.0;   // 0 ~ 255 -> 0.0 ~ 1.0
        }
        ++row;
        ++p;
      }
    }
    break;

  default:
    // unsupported color types
    break;
  }

  png_destroy_read_struct(&png, &info, NULL);

  return image;
}

#else

Blob* load_png_image(const std::string &file_name, bool normalize) {
  return NULL;
}

#endif
