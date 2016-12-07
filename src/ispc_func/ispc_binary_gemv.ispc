
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
    uniform int num_words,
    uniform int num_basis,
    const uniform unsigned int64 * uniform input,
    uniform int rows,
    uniform int row_offset,
    const uniform unsigned int64 * uniform basis_star,
    const uniform unsigned int64 * uniform basis_mask,
    const uniform unsigned int32  basis_nnz[],
    const uniform float basis_weight[],
    uniform float out[]) {
  uniform float * uniform out_ptr = (uniform float * uniform)out;
  const uniform unsigned int64 * uniform in_ptr = input;


  // calc offset for each task
  // out_ptr += (rows      * taskIndex);
  // in_ptr  += (num_words * num_basis * taskIndex);
  uniform int n = (rows * taskIndex) + row_offset;

  // ternary basis
  uniform int bits = 0;
  uniform int sum  = 0;

  // const uniform unsigned int64 * uniform a_ptr;

  for (uniform int ni = 0; ni < rows; ++ni) {
    sum = 0;
    for (uniform int i = 0; i < num_basis; ++i) {
      bits = scalar_product_3(&in_ptr[num_words * i], &basis_star[(num_words * (n + ni))], &basis_mask[(num_words * (n + ni))], basis_nnz[n + ni], num_words);
      sum += basis_weight[i] * bits;
    }
    out_ptr[n + ni] = sum;
  }
}

task void binary_multi(
    uniform int num_words,
    uniform int num_basis,
    const uniform unsigned int64 * uniform input,
    uniform int rows,
    uniform int row_offset,
    const uniform unsigned int64 * uniform basis_star,
    const uniform float basis_weight[],
    uniform float out[]) {
  uniform float * uniform out_ptr = (uniform float * uniform)out;
  const uniform unsigned int64 * uniform in_ptr = input;

  // calc offset for each task
  // out_ptr += (N * taskIndex);
  // in_ptr  += (num_words * num_basis * taskIndex);
  uniform int n = (rows * taskIndex) + row_offset;

  // binary basis
  uniform int bits = 0;
  uniform int sum  = 0;
  // const uniform unsigned int64 * uniform b_star_ptr = basis_star;

  for (uniform int ni = 0; ni < rows; ++ni) {
    sum = 0;
    for (uniform int i = 0; i < num_basis; ++i) {
      bits = scalar_product_2(&in_ptr[(num_words * i)], &basis_star[(num_words * (n + ni))], num_words);
      sum += basis_weight[i] * bits;
    }
    out_ptr[n + ni] = sum;
  }
}

/*
    C[M] = A[M N] x X[N] + B[M]
 */
export void ispc_binary_gemv(
    uniform int num_basis_total,                // M
    uniform int input_size,                     // N
    uniform int num_basis,
    uniform int basis_type,
    const uniform unsigned int8 basis_star[],   // A binary weight
    const uniform unsigned int8 basis_mask[],
    const uniform unsigned int32 basis_nnz[],
    const uniform float basis_weight[],
    const uniform unsigned int8 X[],            // X
    uniform float C[]) {

    // cache control
    prefetch_l2((void * uniform)X);

    uniform int num_words = (input_size / 64);

    // clear y value
    foreach (n = 0 ... num_basis_total) {
      C[n] = 0;
    }

    // pararell
    uniform int threads = 4;
    uniform int rows = num_basis_total / threads;
    uniform int mod  = num_basis_total % threads;

    if (basis_type == 2) {
      launch [threads] ternary_multi(
        num_words,
        num_basis,
        (const uniform unsigned int64 *)X,
        rows,
        0,
        (const uniform unsigned int64 *)basis_star,
        (const uniform unsigned int64 *)basis_mask,
        basis_nnz,
        basis_weight,
        C);
      sync;

      if (mod > 0) {
        launch ternary_multi(
          num_words,
          num_basis,
          (const uniform unsigned int64 *)X,
          mod,
          num_basis_total - mod,
          (const uniform unsigned int64 *)basis_star,
          (const uniform unsigned int64 *)basis_mask,
          basis_nnz,
          basis_weight,
          C);
          sync;
      }
    } else {
      launch [threads] binary_multi(
        num_words,
        num_basis,
        (const uniform unsigned int64 *)X,
        rows,
        0,
        (const uniform unsigned int64 *)basis_star,
        basis_weight,
        C);

      sync;
      if (mod > 0) {
        launch binary_multi(
          num_words,
          num_basis,
          (const uniform unsigned int64 *)X,
          mod,
          num_basis_total - mod,
          (const uniform unsigned int64 *)basis_star,
          basis_weight,
          C);
          sync;
      }
    }
}
