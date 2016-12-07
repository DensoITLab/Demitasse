inline void lrn_core(
    uniform int in_x_nodes,
    uniform int y_nums,
    uniform int in_maps,
    uniform int local_size,
    uniform float k,
    uniform float alpha,
    uniform float beta,
    const uniform float data[],
    uniform float result[]) {
  uniform int pad      = (local_size >> 1); // local_size / 2;
  uniform int map_size = in_maps + (pad * 2);

  uniform float alpha_over_size = alpha / (float)local_size;

  // padded map buffer
  uniform float * uniform squared = uniform new uniform float[map_size];
  uniform float * uniform norm    = uniform new uniform float[in_maps];

  const uniform float * uniform d = data;
  uniform float * uniform r       = result;

  // initialize data
  foreach (n = 0 ... map_size) {
    squared[n] = 0;
  }

  for (uniform int j = 0; j < y_nums; ++j) {

    for (uniform int i = 0; i < in_x_nodes; ++i) {
      // squared data
      foreach(n = 0 ... in_maps) {
        float v = d[n];
        squared[pad + n] = v * v;
      }

      // calc normalized scale
      for (uniform int m = 0; m < in_maps; ++m) {
        float sum = 0;
        foreach (n = 0 ... local_size) {
          sum += squared[m + n];
        }
        norm[m] = reduce_add(sum);
      }

      foreach (m = 0 ... in_maps) {
        float scale = (norm[m] * alpha_over_size) + k;
        scale = pow(scale, - beta);
        r[m] = d[m] * scale;
      } // for m

      d += in_maps;
      r += in_maps;
    } // for i
  } // for j

  delete[] squared;
  delete[] norm;
}

task void lrn_task(
    uniform int in_x_nodes,
    uniform int y_nums,
    uniform int in_maps,
    uniform int local_size,
    uniform float k,
    uniform float alpha,
    uniform float beta,
    const uniform float data[],
    uniform float result[]) {

    uniform int j = taskIndex * y_nums;
    const uniform float * uniform d = &data[(j * in_x_nodes * in_maps)];
    float * uniform r = &result[(j * in_x_nodes * in_maps)];

    lrn_core(in_x_nodes, y_nums, in_maps, local_size, k, alpha, beta, d, r);
}

export void ispc_lrn(
    uniform int in_x_nodes,
    uniform int in_y_nodes,
    uniform int in_maps,
    uniform int local_size,
    uniform float k,
    uniform float alpha,
    uniform float beta,
    const uniform float data[],
    uniform float result[]) {

  uniform int num_threads = min((in_y_nodes / 1), 8); // 1: min task items, 8: max threads
  if (num_threads > 1) {
    uniform int y_nums      = in_y_nodes / num_threads;
    uniform int y_mods      = in_y_nodes % num_threads;

    launch[num_threads] lrn_task(in_x_nodes, y_nums, in_maps, local_size, k, alpha, beta, data, result);
    sync;
    if (y_mods >  0) {
      uniform int j = in_y_nodes - y_mods;
      const uniform float * uniform d = &data[(j * in_x_nodes * in_maps)];
      float * uniform r = &result[(j * in_x_nodes * in_maps)];

      lrn_core(in_x_nodes, y_mods, in_maps, local_size, k, alpha, beta, d, r);
    }
  } else {
    lrn_core(in_x_nodes, in_y_nodes, in_maps, local_size, k, alpha, beta, data, result);
  }

}
