
export void ispc_identity(uniform int in_node, uniform float a[], uniform float result[]) {
  foreach (i = 0 ... in_node) {
    result[i] = a[i];
  }
}

export void ispc_identity_df(uniform int in_node, uniform float a[], uniform float result[]) {
  foreach (i = 0 ... in_node) {
    result[i] = 1.0f;
  }
}
