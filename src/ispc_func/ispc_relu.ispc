
task void relu_task(uniform int in_node, const uniform float a[], uniform float result[]) {
  uniform int offset = taskIndex * in_node;
  result += offset;
  a      += offset;
  foreach (i = 0 ... in_node) {
    result[i] = max(0.0f, a[i]);
  }
}

export void ispc_relu(uniform int in_node, const uniform float a[], uniform float result[]) {

 uniform int num_threads = min((in_node / 512), 8); // 1: min task items, 8: max threads
 if (num_threads > 1) {
   uniform int nums      = in_node / num_threads;
   uniform int mods      = in_node % num_threads;

    launch[num_threads] relu_task(nums, a, result);
    sync;

    if (mods > 0) {
      uniform int offset = nums * num_threads;
      result += offset;
      a      += offset;
      foreach (i = 0 ... mods) {
        result[i] = max(0.0f, a[i]);
      }
    }
  } else {
    foreach (i = 0 ... in_node) {
      result[i] = max(0.0f, a[i]);
    }
  }
}

export void ispc_relu_df(uniform int in_node, uniform float a[], uniform float result[]) {
  foreach (i = 0 ... in_node) {
    result[i] = select(a[i] > 0.0f, 1.0f, 0.0f);
  }
}
