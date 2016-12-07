
inline void load_clamped_data(
    uniform int in_x_max,
    uniform int in_y_max,
    uniform int in_maps,
    uniform int ksize,
    uniform int vectorize,
    uniform int x,
    uniform int y,
    const uniform float data[],
    uniform float result[]) {
/*
  foreach (j = 0 ... ksize, i = 0 ... vectorize) {
    int xx        = x + i;
    int yy        = y + j;

    bool in_range = and(and(yy >= 0, yy < in_y_max),
                        and(xx >= 0, xx < in_x_max));

    int dst_idx = (j * vectorize) + i;
    int src_idx = (j * in_x_max)  + i;

    if (in_range) {
      result[dst_idx] = data[src_idx];
    } else {
      result[dst_idx] = 0.0f;
    }
  }
*/

  // uniform int yy      = y;
  const uniform float * uniform d = data;
  uniform float * uniform out     = result;

  uniform bool in_x_range = (x >= 0 && x + vectorize < in_x_max);

  for (uniform int j = 0; j < ksize; ++j) {
    uniform int yy = y + j;
    // check clip rect range for Y
    if (yy >= 0 && yy < in_y_max) {
      // check clip rect range for X
      if (in_x_range) {
        foreach (i = 0 ... vectorize) {
          out[i] = d[i];
        }
      } else {
        foreach (i = 0 ... vectorize) {
          int xx = x + i;
          if (xx >= 0 && xx < in_x_max) {
            out[i] = d[i];
          } else {
            out[i] = 0.0f;
          }
        }
      }
    } else {
      foreach (i = 0 ... vectorize) {
        out[i] = 0.0f;
      }
    }
    // next line
    d   += in_x_max;
    out += vectorize;
  }
}


inline void im2col_core(
    uniform int in_x_max,   // in_x_nodes * in_maps
    uniform int in_y_max,
    uniform int x_nums,
    uniform int y_nums,
    uniform int index_y,
    uniform int in_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    uniform int vectorize,
    uniform int patch_size,
    const uniform float data[],
    uniform float col[]) {

  uniform int y     = (index_y * stride) - padding;

  uniform int idx   = patch_size * x_nums * index_y;
  uniform float* uniform out   = &col[idx];

  uniform int x_offset = stride * in_maps;

  for (uniform int j = 0; j < y_nums; ++j) {
    uniform int x = -(padding * in_maps);
    const uniform float * uniform d = &data[(y * in_x_max) + x];

    for (uniform int i = 0; i < x_nums; ++i) {
      load_clamped_data(in_x_max, in_y_max, in_maps, ksize, vectorize, x, y, d, out);
      x   += x_offset;
      d   += x_offset;
      out += patch_size;
    } // for i

    y   += stride;
  } // for j
}

task void im2col_task(
    uniform int in_x_max,   // in_x_nodes * in_maps
    uniform int in_y_max,
    uniform int x_nums,
    uniform int y_nums,
    uniform int in_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    uniform int vectorize,
    uniform int patch_size,
    const uniform float data[],
    uniform float col[]) {
  im2col_core(in_x_max, in_y_max, x_nums, y_nums, (taskIndex * y_nums), in_maps, ksize, stride, padding, vectorize, patch_size, data, col);
}


export void ispc_im2col(
    uniform int in_x_nodes,
    uniform int in_y_nodes,
    uniform int in_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    uniform int out_x_nodes,
    uniform int out_y_nodes,
    const uniform float data[],
    uniform float col[]) {

  uniform int in_x_max    = in_x_nodes * in_maps;
  uniform int vectorize   = ksize * in_maps;
  uniform int wsize       = vectorize * ksize;
  uniform int patch_size  = select(wsize % 16 > 0, ((wsize / 16) + 1) * 16, wsize);

  uniform int num_threads = min((out_y_nodes / 1), 8); // 1: min task items, 8: max threads
  if (num_threads > 1) {
    uniform int y_nums      = out_y_nodes / num_threads;
    uniform int y_mods      = out_y_nodes % num_threads;

    launch[num_threads] im2col_task(in_x_max, in_y_nodes, out_x_nodes, y_nums, in_maps, ksize, stride, padding, vectorize, patch_size, data, col);
    sync;

    if (y_mods >  0) {
      uniform int index_y = out_y_nodes - y_mods;

      im2col_core(in_x_max, in_y_nodes, out_x_nodes, y_mods, index_y, in_maps, ksize, stride, padding, vectorize, patch_size, data, col);
    }
  } else {
    im2col_core(in_x_max, in_y_nodes, out_x_nodes, out_y_nodes, 0, in_maps, ksize, stride, padding, vectorize, patch_size, data, col);
  }
}
