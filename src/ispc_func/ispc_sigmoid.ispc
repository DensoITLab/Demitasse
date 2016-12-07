
export void ispc_sigmoid(uniform int in_node, uniform float a[], uniform float result[]) {
  foreach (i = 0 ... in_node) {
    result[i] = 1.0f / (1.0f + exp(a[i]));
  }
}

export void ispc_sigmoid_df(uniform int in_node, uniform float a[], uniform float result[]) {
  ispc_sigmoid(in_node, a, result);
  foreach (i = 0 ... in_node) {
    float f = result[i];
    result[i] = (1.0f - f) * f;
  }
}
