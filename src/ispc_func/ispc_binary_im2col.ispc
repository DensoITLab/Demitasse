
inline void load_clamped_data(
    uniform int in_x_max,
    uniform int in_y_nodes,
    uniform int in_maps,
    uniform int ksize,
    uniform int vectorize,
    uniform int x,
    uniform int y,
    const uniform unsigned int8 data[],
    uniform unsigned int8 result[]) {

  uniform int yy      = y;
  uniform int src_idx = y * in_x_max + x;
  uniform int dst_idx = 0;
  uniform unsigned int8 * uniform out = result;

  for (uniform int j = 0; j < ksize; ++j) {
    // check clip rect range for Y
    if (yy >= 0 && yy < in_y_nodes) {
      // check clip rect range for X
      if (x >= 0 && x + vectorize < in_x_max) {
        foreach (i = 0 ... vectorize) {
          result[dst_idx + i] = data[src_idx + i];
        }
      } else {
        foreach (i = 0 ... vectorize) {
          int xx = x + i;
          if (xx >= 0 && xx < in_x_max) {
            result[dst_idx + i] = data[src_idx + i];
          } else {
            result[dst_idx + i] = 0;
          }
        }
      }
    } else {
      foreach (i = 0 ... vectorize) {
        result[dst_idx + i] = 0;
      }
    }
    src_idx += in_x_max; // next line
    dst_idx += vectorize;
    ++yy;
  }
}

task void im2col_x(
    uniform int in_x_max,   // in_x_nodes * in_maps
    uniform int in_y_nodes,
    uniform int num_basis,
    uniform int out_x_nodes,
    uniform int in_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    const uniform unsigned int8 data[],
    uniform unsigned int8 col[]) {
  uniform int vectorize   = ksize * in_maps; // binary
  uniform int wsize       = vectorize * ksize;
  uniform int patch_size  = select(wsize % 16 > 0, ((wsize / 16) + 1) * 16, wsize);

  uniform int x     = -(padding * in_maps);
  uniform int y     = (taskIndex * stride) - padding;
  uniform int idx   = patch_size * out_x_nodes * num_basis * taskIndex;
  uniform unsigned int8* uniform out   = &col[idx];

  for (uniform int i = 0; i < out_x_nodes; ++i) {
    for (uniform int n = 0; n < num_basis; ++n) {
      const uniform unsigned int8* uniform in_data = &data[(in_x_max * in_y_nodes * n)];
      // copy data to allocated array to align memory layout
      load_clamped_data(in_x_max, in_y_nodes, in_maps, ksize, vectorize, x, y, in_data, out);
      out += patch_size;
    }
    x += stride * in_maps;
  }
}

export void ispc_binary_im2col(
    uniform int in_x_nodes,
    uniform int in_y_nodes,
    uniform int in_maps,      // binary packed 1/8
    uniform int num_basis,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    uniform int out_x_nodes,
    uniform int out_y_nodes,
    const uniform unsigned int8 data[],
    uniform unsigned int8 col[]) {
  uniform int in_x_max    = in_x_nodes * in_maps;

  launch[out_y_nodes] im2col_x(in_x_max, in_y_nodes, num_basis, out_x_nodes, in_maps, ksize, stride, padding,
                  data, col);
  sync;
}
