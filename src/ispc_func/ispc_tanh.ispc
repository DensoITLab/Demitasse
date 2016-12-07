
export void ispc_tanh(uniform int in_node, uniform float a[], uniform float result[]) {
  foreach (i = 0 ... in_node) {
    float e1 = exp(a[i]);
    float e2 = exp(-a[i]);
    result[i] = (e1 - e2) / (e1 + e2);
  }
}

export void ispc_tanh_df(uniform int in_node, uniform float a[], uniform float result[]) {
  ispc_tanh(in_node, a, result);
  foreach (i = 0 ... in_node) {
    float f = result[i];
    result[i] = 1.0f - (f * f);
  }
}
