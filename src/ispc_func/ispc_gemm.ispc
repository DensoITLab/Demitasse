/*
    C[M N] = A[M K] x B[K N] + C'[M N]
 */

#define M_BLOCK    16
#define N_BLOCK    16
#define BLOCK_SIZE 32

inline uniform float mm_block(const uniform float A[], const uniform float B[], uniform int K) {
  float sum = 0;

  foreach (k = 0 ... K) {
    sum += A[k] * B[k];
  }

  return reduce_add(sum);
}

inline void gemm_mod(
    uniform int m,
    uniform int mrows,
    uniform int K,
    uniform int N,
    const uniform float A[],
    const uniform float B[],
    uniform float C[]) {

  for (uniform int n = 0; n < N; n += N_BLOCK) {
    for (uniform int k = 0; k < K; k += BLOCK_SIZE) {
      for (uniform int mi = 0; mi < mrows; ++mi) {
        for (uniform int ni = 0; ni < N_BLOCK; ++ni) {
          C[(N * (m + mi)) + n + ni] += mm_block(&A[(K * (m + mi)) + k], &B[(K * (n + ni)) + k], BLOCK_SIZE);
        }
      }
    }
  }
}

task void gemm_core(
    uniform int mrows,
    // uniform int msize,
    uniform int K,
    uniform int N,
    const uniform float A[],
    const uniform float B[],
    uniform float C[]) {

  uniform int moffset = taskIndex * mrows;
  uniform float * uniform tmp = uniform new float[M_BLOCK * N];

  const uniform float * uniform Ap = A + (K * moffset);

  // for (uniform int m = moffset; m < moffset + mrows; m += M_BLOCK) {
  for (uniform int m = 0; m < mrows; m += M_BLOCK) {

    // clear temp
    foreach (mi = 0 ... M_BLOCK, n = 0 ... N) {
      tmp[(N * mi) + n] = 0.0f;
    }

    for (uniform int n = 0; n < N; n += N_BLOCK) {
      for (uniform int k = 0; k < K; k += BLOCK_SIZE) {

        // inner blocks
        for (uniform int mi = 0; mi < M_BLOCK; ++mi) {
          for (uniform int ni = 0; ni < N_BLOCK; ++ni) {
            tmp[(N * mi) + n + ni] += mm_block(&Ap[(K * mi) + k], &B[(K * (n + ni)) + k], BLOCK_SIZE);
          } // ni
        } // mi

      } // K
    } // N


    // copy result
    foreach (mi = 0 ... M_BLOCK, n = 0 ... N) {
      C[(N * (m + mi)) + n] += tmp[(N * mi) + n];
    }

    Ap += (K * M_BLOCK);

  } // M
}

export void ispc_gemm(
    uniform int M,
    uniform int N,
    uniform int K,
    const uniform float A[],
    const uniform float B[],
    const uniform float Cp[],
    uniform float C[]) {

  // copy bias to result
  foreach (m = 0 ... M, n = 0 ... N) {
    C[(N * m) + n] = Cp[n];
  }

  uniform int num_threads = 4;

  uniform int mblocks = M / M_BLOCK;
  uniform int mrows   = (mblocks / num_threads) * M_BLOCK;
  uniform int mod     = M - (mrows * num_threads);

  if (mrows > 0) {
    launch [num_threads] gemm_core(mrows, K, N, A, B, C);
    sync;
  }

  if (mod > 0) {
    gemm_mod(M - mod, mod, K, N, A, B, C);
  }
}
