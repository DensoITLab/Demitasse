
export void ispc_max2d(
    uniform int in_x_nodes,
    uniform int in_y_nodes,
    uniform int in_maps,
    uniform int ksize,
    uniform int stride,
    uniform int padding,
    uniform int out_x_nodes,
    uniform int out_y_nodes,
    const uniform float data[],
    uniform float result[]) {
  uniform int idx      = 0;
  uniform int stride_y = (in_x_nodes * in_maps);

  for (uniform int j = 0; j < out_y_nodes; ++j) {
    for (uniform int i = 0; i < out_x_nodes; ++i) {
      uniform float xx = (i * stride) - padding;
      uniform float yy = (j * stride) - padding;

      uniform int ii   = ((yy * in_x_nodes) + xx) * in_maps;

      // prarallel by k-maps
      foreach (k = 0 ... in_maps) {
        // N x N pooling (optimize by unroll)
        float m = 0.0f;

        for (uniform int y = 0; y < ksize; ++y) {
          if (yy + y >= 0 && yy + y < in_y_nodes) {
            float mm = 0.0f;
            for (uniform int x = 0; x < ksize; ++x) {
              if (xx + x >= 0 && xx + x < in_x_nodes) {
                float v = data[ii + (y * stride_y) + (x * in_maps) + k];
                mm = max(mm, v);
              }
            }
            m = max(m, mm);
          }
        }
        result[idx + k] = m;
      } // for k
      idx += in_maps;
    } // for i
  } // for j
}
