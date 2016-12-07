
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
    uniform float out[],
    uniform int num_words,
    uniform int num_basis,
    const uniform unsigned int64 * uniform input,
    uniform int N,
    const uniform unsigned int64 * uniform basis_star,
    const uniform unsigned int64 * uniform basis_mask,
    const uniform unsigned int32  basis_nnz[],
    const uniform float basis_weight[]) {
  uniform float * uniform out_ptr = (uniform float * uniform)out;
  const uniform unsigned int64 * uniform in_ptr = input;

  // calc offset for each task
  out_ptr += (N         * taskIndex);
  in_ptr  += (num_words * num_basis * taskIndex);

  // ternary basis
  uniform int bits = 0;
  // const uniform unsigned int64 * uniform a_ptr;
  const uniform unsigned int64 * uniform b_star_ptr = basis_star;
  const uniform unsigned int64 * uniform b_mask_ptr = basis_mask;

  uniform int sum = 0;

  for (uniform int n = 0; n < N; ++n) {
    // a_ptr = in_ptr;
    sum = 0;
    for (uniform int i = 0; i < num_basis; ++i) {
      bits = scalar_product_3(&in_ptr[num_words * i], b_star_ptr, b_mask_ptr, basis_nnz[n], num_words);
      sum += basis_weight[i] * bits;
      // a_ptr += num_words;
    }
    out_ptr[n] = sum;
    b_star_ptr += num_words;
    b_mask_ptr += num_words;
    // out_ptr++;
  }
}

task void binary_multi(
    uniform float out[],
    uniform int num_words,
    uniform int num_basis,
    const uniform unsigned int64 * uniform input,
    uniform int N,
    const uniform unsigned int64 * uniform basis_star,
    const uniform float basis_weight[]) {
  uniform float * uniform out_ptr = (uniform float * uniform)out;
  const uniform unsigned int64 * uniform in_ptr = input;

  // calc offset for each task
  out_ptr += (N * taskIndex);
  in_ptr  += (num_words * num_basis * taskIndex);

  // binary basis
  uniform int bits = 0;
  // const uniform unsigned int64 * uniform a_ptr;
  const uniform unsigned int64 * uniform b_star_ptr = basis_star;

  uniform int sum = 0;

  for (uniform int n = 0; n < N; ++n) {
    // a_ptr = in_ptr;
    sum = 0;
    for (uniform int i = 0; i < num_basis; ++i) {
      bits = scalar_product_2(&in_ptr[(num_words * i)] /* a_ptr */, b_star_ptr, num_words);
      sum += basis_weight[i] * bits;
      // a_ptr += num_words;
    }
    out_ptr[n] = sum;
    b_star_ptr += num_words;
    // out_ptr++;
  }
}

/*
    C[M N] = A[M K] x B[K N]
 */
export void ispc_binary_mul(
    uniform float out[],
    uniform int input_size,                     // K
    uniform int num_basis,
    uniform int M,                              // M
    const uniform unsigned int8 input[],        // A
    uniform int num_basis_total,                // N
    uniform int basis_type,
    const uniform unsigned int8 basis_star[],   // B binary weight
    const uniform unsigned int8 basis_mask[],
    const uniform unsigned int32 basis_nnz[],
    const uniform float basis_weight[]) {

    uniform int K = (input_size / 64);

    // clear y value
    foreach (n = 0 ... num_basis_total * M) {
      out[n] = 0;
    }

    if (basis_type == 2) {
      launch[M] ternary_multi(
        out,
        K,
        num_basis,
        (const uniform unsigned int64 *)input,
        num_basis_total,
        (const uniform unsigned int64 *)basis_star,
        (const uniform unsigned int64 *)basis_mask,
        basis_nnz,
        basis_weight);
    } else {
      launch[M] binary_multi(
        out,
        K,
        num_basis,
        (const uniform unsigned int64 *)input,
        num_basis_total,
        (const uniform unsigned int64 *)basis_star,
        basis_weight);
    }
    sync;
}
