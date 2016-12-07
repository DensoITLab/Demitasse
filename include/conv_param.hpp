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
 #ifndef __DEMITASSE_CONV_PARAM__
#define __DEMITASSE_CONV_PARAM__

#include <math.h>

namespace demitasse {

struct ConvParam {
  unsigned int ksize;
  unsigned int stride;
  unsigned int padding = 0;
  unsigned int in_maps;                                 // # of input maps
  unsigned int out_maps;                                // # of output maps

  unsigned int out_x_nodes(unsigned int in_x_nodes) const {
    return ((in_x_nodes + (2 * padding) - ksize) / stride) + 1;
  };

  unsigned int out_y_nodes(unsigned int in_y_nodes) const {
    return ((in_y_nodes + (2 * padding) - ksize) / stride) + 1;
  };

  bool isNeedIm2Col(int in_x_nodes, int in_y_nodes) {
    bool need_im2col = true;
    if (in_x_nodes == ksize && in_y_nodes == ksize) {
      need_im2col = false;
    }
    return need_im2col;
  }

};

}

#endif /* __DEMITASSE_CONV_PARAM__ */
