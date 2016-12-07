
// parallel for x and ch
inline void conv2d_filter(
    uniform int out_maps,
    uniform int wsize_align,
    uniform float data[],
    const uniform float weight[],
    const uniform float bias[],
    uniform float result[]) {
  prefetch_l1((void * uniform)data);
  prefetch_nt((void * uniform)weight);

  for (uniform int k = 0; k < out_maps; ++k) {
    const uniform float * uniform w = &weight[(wsize_align * k)];

    float z = 0.0f;
    foreach (i = 0 ... wsize_align) {
      z += data[i] * w[i];
    }
    result[k] = reduce_add(z) + bias[k];
  }
}

inline void init_data(uniform int wsize_align, uniform float data[]) {
  foreach (i = 0 ... wsize_align) {
    data[i] = 0.0f;
  }
}

inline void load_clamped_data(
    uniform int in_x_max,
    uniform int in_y_max,
    uniform int vectorize,
    uniform int ksize,
    uniform int x,
    uniform int y,
    const uniform float dd[],
    uniform float result[]) {
/*
  foreach (j = 0 ... ksize, i = 0 ... vectorize) {
    int xx = x + i;
    int yy = y + j;

    bool in_range = and(and(yy >= 0, yy < in_y_max),
                        and(xx >= 0, xx < in_x_max));

    float value = 0.0f;
    if (in_range) {
      int src_idx = (yy * in_x_max);
      value = dd[src_idx];
    }

    int dst_idx = (j * vectorize) + i;
    result[dst_idx] = value;
  }
*/

  for (uniform int j = 0; j < ksize; ++j) {
    uniform int yy = y + j;
    if (yy >= 0 && yy < in_y_max) {
      if (x >= 0 && x + vectorize < in_x_max) {
        uniform int src_idx = yy * in_x_max + x;
        uniform int dst_idx = (j * vectorize);
        foreach (i = 0 ... vectorize) {
          result[dst_idx + i] = dd[src_idx + i];
        }
      } else {
        uniform int src_idx = yy * in_x_max + x;
        uniform int dst_idx = (j * vectorize);
        foreach (i = 0 ... vectorize) {
          int xx = x + i;
          if (xx >= 0 && xx < in_x_max) {
            result[dst_idx + i] = dd[src_idx + i];
          } else {
            result[dst_idx + i] = 0.0f;
          }
        }
      }
    } else {
      uniform int dst_idx = (j * vectorize);
      foreach (i = 0 ... vectorize) {
        result[dst_idx + i] = 0.0f;
      }
    }
  }
}

task void conv_x(
    uniform int in_x_nodes,
    uniform int in_y_nodes,
    uniform int out_x_nodes,
    uniform int in_maps,
    uniform int out_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    const uniform float data[],
    const uniform float weight[],
    const uniform float bias[],
    uniform float result[]) {
  uniform int vectorize   = ksize * in_maps;
  uniform int wsize       = vectorize * ksize;
  uniform int wsize_align = select(wsize % 16 > 0, ((wsize / 16) + 1) * 16, wsize);

  uniform int y  = (taskIndex * stride) - padding;

  // create aligned memory
  uniform float * uniform d = uniform new uniform float[wsize_align];
  init_data(wsize_align, d);

  uniform int x   = -(padding * in_maps);
  uniform int idx = taskIndex * out_x_nodes * out_maps;

  for (uniform int i = 0; i < out_x_nodes; ++i) {
    // copy data to allocated array to align memory layout
    load_clamped_data(in_x_nodes, in_y_nodes, vectorize, ksize, x, y, data, d);

    // convolution
    conv2d_filter(out_maps, wsize_align, d, weight, bias, &result[idx]);

    x   += (stride * in_maps);
    idx += out_maps;
  } // for i

  delete[] d;
}

export void ispc_conv2d(
    uniform int in_x_nodes,
    uniform int in_y_nodes,
    uniform int in_maps,
    uniform int out_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    uniform int out_x_nodes,
    uniform int out_y_nodes,
    const uniform float data[],
    const uniform float weight[],
    const uniform float bias[],
    uniform float result[]) {
  uniform int in_x_max    = in_x_nodes * in_maps;

  launch[out_y_nodes] conv_x(in_x_max, in_y_nodes, out_x_nodes, in_maps, out_maps, ksize, stride, padding,
                  data, weight, bias, result);
  sync;
}
