/*
    C[N] = A[M N] x B[N] + C'[N]
 */

task void gemv(
    uniform int rows,
    uniform int row_offset,
    uniform int cols, // N
    const uniform float weight[],
    const uniform float bias[],
    const uniform float data[],
    uniform float result[]) {

    uniform int m = (rows * taskIndex) + row_offset;

    for (uniform int mi = 0; mi < rows; ++mi) {
      float z = 0.0f;
      foreach (i = 0 ... cols) {
        z += data[i] * weight[(cols * (m + mi)) + i];
      }
      result[m + mi] = reduce_add(z) + bias[m + mi];
    }
}

/*
    C[M] = A[M N] x X[N] + B[M]
 */
export void ispc_gemv(
    uniform int M,
    uniform int cols, // N
    const uniform float weight[], // A
    const uniform float bias[],
    const uniform float X[],      // X
    uniform float result[]) {

    // cache control
    prefetch_l2((void * uniform)X);

    uniform int threads = 4;
    uniform int rows    = M / threads;
    uniform int mod     = M % threads;

    launch [threads] gemv(rows, 0, cols, weight, bias, X, result);
    sync;
    if (mod > 0) {
      launch gemv(mod, M - mod, cols, weight, bias, X, result);
      sync;
    }
}
