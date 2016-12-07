
#define M_BLOCK    16
#define N_BLOCK    16

inline uniform int scalar_product_2(
    const uniform unsigned int64 x1_binary[],
    const uniform unsigned int64 x2_binary[],
    uniform int num_words) {
  int bit_count = 0;
  unsigned int64 bits = 0;

  foreach (i = 0 ... num_words) {
    bits       = (x1_binary[i] ^ x2_binary[i]);
    bit_count += popcnt(bits);
  }

	return (num_words * 64) - (reduce_add(bit_count) * 2);
}

inline uniform int scalar_product_3(
    const uniform unsigned int64 x1_binary[],
    const uniform unsigned int64 x2_ternary_star[],
    const uniform unsigned int64 x2_ternary_mask[],
    uniform unsigned int x2_ternary_nnz,
    uniform int num_words) {
  int bit_count = 0;

  unsigned int64 bits = 0;

  foreach (i = 0 ... num_words) {
    bits       = ((x1_binary[i] ^ x2_ternary_star[i]) & x2_ternary_mask[i]);
    bit_count += popcnt(bits);
  }

  return x2_ternary_nnz - (reduce_add(bit_count) * 2);
}

//
// A: Input binary matrix
// B: basis matrix
task void ternary_multi(
    uniform float C[],
    uniform int num_words,
    uniform int num_basis,
    uniform int rows,
    uniform int row_offset,
    const uniform unsigned int64 * uniform A, // input,
    uniform int num_basis_total,
    const uniform unsigned int64 * uniform basis_star,  // B
    const uniform unsigned int64 * uniform basis_mask,  // B
    const uniform unsigned int32  basis_nnz[],          // B
    const uniform float basis_weight[]) {

    // cache control
    prefetch_l1((void * uniform)basis_star);
    prefetch_l1((void * uniform)basis_mask);
    prefetch_l1((void * uniform)basis_nnz);

  // calc offset for each task
  uniform int m_offset = (rows * taskIndex) + row_offset;

  // ternary basis
  uniform int bits = 0;

  A += num_words * num_basis * m_offset;
  C += num_basis_total * m_offset;

  for (uniform int m = m_offset; m < m_offset + rows; ++m) {
    for (uniform int n = 0; n < num_basis_total; ++n) {
      // inner block
      // access range: basis_star (num_words * n)
      C[n] = 0;
      for (uniform int i = 0; i < num_basis; ++i) {
        bits = scalar_product_3(&A[((num_basis * m) + i) * num_words],
                                &basis_star[(num_words * n)],
                                &basis_mask[(num_words * n)],
                                basis_nnz[n],
                                num_words);
        C[n] += basis_weight[i] * (float)bits;
      }
    }
    C += num_basis_total;
  }
}

task void binary_multi(
    uniform float C[],
    uniform int num_words,
    uniform int num_basis,
    uniform int rows,
    uniform int row_offset,
    const uniform unsigned int64 * uniform A,           // input,
    uniform int num_basis_total,
    const uniform unsigned int64 * uniform basis_star,  // B
    const uniform float basis_weight[]) {

  // cache control
  prefetch_l1((void * uniform)basis_star);

  // calc offset for each task
  uniform int m_offset = (rows * taskIndex) + row_offset;

  // binary basis
  uniform int bits = 0;

  C += num_basis_total * m_offset;

  for (uniform int m = m_offset; m < m_offset + rows; ++m) {
    for (uniform int n = 0; n < num_basis_total; ++n) {
      C[n] = 0;
      for (uniform int i = 0; i < num_basis; ++i) {
        bits = scalar_product_2(&A[num_words * ((num_basis * m) + i)],
                                &basis_star[(num_words * n)],
                                num_words);
        C[n] += basis_weight[i] * (float)bits;
      }
    }
    C += num_basis_total;
  }
}

/*
    C[M N] = A[M K] x B[K N]
 */
export void ispc_binary_gemm(
    uniform int input_rows,                     // M
    uniform int num_basis_total,                // N
    uniform int input_size,                     // K
    uniform int num_basis,
    const uniform unsigned int8 A[],            // A
    uniform int basis_type,
    const uniform unsigned int8 basis_star[],   // B binary weight
    const uniform unsigned int8 basis_mask[],
    const uniform unsigned int32 basis_nnz[],
    const uniform float basis_weight[],
    uniform float C[]) {

    uniform int num_words = (input_size / 64);

    // clear y value
    foreach (n = 0 ... num_basis_total) {
      C[n] = 0;
    }

    // pararell
    uniform int threads = 4;
    uniform int rows = input_rows / threads;
    uniform int mod  = input_rows % threads;

    if (basis_type == 2) {
      launch [threads] ternary_multi(
        C,
        num_words,
        num_basis,
        rows,
        0,
        (const uniform unsigned int64 *)A,
        num_basis_total,
        (const uniform unsigned int64 *)basis_star,
        (const uniform unsigned int64 *)basis_mask,
        basis_nnz,
        basis_weight);
      sync;

      if (mod > 0) {
        launch ternary_multi(
          C,
          num_words,
          num_basis,
          mod,
          num_basis_total - mod,
          (const uniform unsigned int64 *)A,
          num_basis_total,
          (const uniform unsigned int64 *)basis_star,
          (const uniform unsigned int64 *)basis_mask,
          basis_nnz,
          basis_weight);
          sync;
      }
    } else {
      launch [threads] binary_multi(
        C,
        num_words,
        num_basis,
        rows,
        0,
        (const uniform unsigned int64 *)A,
        num_basis_total,
        (const uniform unsigned int64 *)basis_star,
        basis_weight);

      sync;
      if (mod > 0) {
        launch binary_multi(
          C,
          num_words,
          num_basis,
          mod,
          num_basis_total - mod,
          (const uniform unsigned int64 *)A,
          num_basis_total,
          (const uniform unsigned int64 *)basis_star,
          basis_weight);
          sync;
      }
    }
}
