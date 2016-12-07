
uniform float max_func(uniform int in_node, const uniform float data[]) {
  float m = 0.0f;
  foreach (i = 0...in_node) {
    m = max(m, data[i]);
  }
  return reduce_max(m);
}

uniform float all_func(uniform int in_node, uniform float m, const uniform float data[]) {
  float all = 0.0f;
  foreach (i = 0...in_node) {
    all += exp(data[i] - m);
  }
  return reduce_add(all);
}

export void ispc_softmax(uniform int in_node, uniform float a[], uniform float result[]) {
  uniform float m   = max_func(in_node, a);
  uniform float all = all_func(in_node, m, a);
  foreach (i = 0 ... in_node) {
    result[i] = exp(a[i] - m) / all;
  }
}

export void ispc_softmax_df(uniform int in_node, uniform float a[], uniform float result[]) {
  foreach (i = 0 ... in_node) {
    result[i] = a[i] * (1.0f - a[i]);
  }
}
