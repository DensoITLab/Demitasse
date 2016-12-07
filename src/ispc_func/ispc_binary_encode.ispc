
inline int value_to_index(
    float v,
    uniform float coff1,
    uniform float coff2,
    uniform int table_size) {
  int idx = floor(coff1 * v + coff2 + 0.5);

  // return min(max(idx, 0), table_size - 1);
  return clamp(idx, 0, table_size - 1);
}

export void ispc_binary_encode(
    uniform unsigned int8       out[],
    const uniform float         input[],
    uniform int                 data_size,
    uniform float               coff1,
    uniform float               coff2,
    uniform int                 num_basis,
    uniform int                 table_size,
    const uniform unsigned int32 bits_table[]) {
  uniform unsigned int8 * uniform p = out;
  const uniform float   * uniform d = input;

  uniform int idx[8];

  // FIXME: data_size % 8 == 0, otherwise, it causes error
  uniform int out_size = data_size / 8;

  const uniform unsigned int32 * uniform bit = bits_table;
  uniform int z = 0;

  for (uniform int i = 0; i < out_size; i++) {
    // bit table index from 8 value
    foreach (n = 0 ... 8) {
      idx[n] = value_to_index(d[n], coff1, coff2, table_size);
    }

    // set bit lookup table for each num_basis
    bit = bits_table;

    for (uniform int k = 0; k < num_basis; ++k) {
      // pack 8 bits to binary
      z  = (bit[idx[ 0]] <<  0);
      z += (bit[idx[ 1]] <<  1);
      z += (bit[idx[ 2]] <<  2);
      z += (bit[idx[ 3]] <<  3);
      z += (bit[idx[ 4]] <<  4);
      z += (bit[idx[ 5]] <<  5);
      z += (bit[idx[ 6]] <<  6);
      z += (bit[idx[ 7]] <<  7);

      p[(out_size * k) + i] = z;

      bit += table_size;
    }

    d += 8;
  }
}
