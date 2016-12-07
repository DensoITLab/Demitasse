
export uniform float mul_func(
    uniform int in_nodes,
    const uniform float weight[],
    const uniform float data[]) {
  float result = 0.0f;

  foreach (i = 0...in_nodes) {
      result += weight[i] * data[i];
  }

  return reduce_add(result);
}

/*
  muladd with task parallelism
 */
task void muladd(uniform int in_nodes,
    const uniform float weight[],
    const uniform float bias[],
    const uniform float data[],
    uniform float result[]) {
  float s = 0.0f;

  const uniform float *w = &weight[(taskIndex * in_nodes)];

  foreach (i = 0 ... in_nodes) {
      s += w[i] * data[i];
  }

  result[taskIndex] = reduce_add(s) + bias[taskIndex];
}

export void muladd_func(
    uniform int in_nodes,
    uniform int out_nodes,
    const uniform float weight[],
    const uniform float bias[],
    const uniform float data[],
    uniform float result[]) {
  launch[out_nodes] muladd(in_nodes, weight, bias, data, result);
  sync;
}
