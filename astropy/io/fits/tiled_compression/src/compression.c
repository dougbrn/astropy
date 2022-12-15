#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <fits_hcompress.h>
#include <fits_hdecompress.h>
#include <imcompress.h>
#include <pliocomp.h>
#include <quantize.h>
#include <ricecomp.h>

// TODO: use better estimates for compressed buffer sizes, as done in
//       imcomp_calc_max_elem in cfitsio. For now we assume the
//       compressed data won't be more than four times the size of the
//       uncompressed data, which is safe but too generous.

// Some of the cfitsio compression files use ffpmsg
// so we provide a dummy function to replace this.
void ffpmsg(const char *err_message) {}

// Compatibility code because we pick up fitsio2.h from cextern. Can
// remove once we remove cextern
#ifdef _REENTRANT
pthread_mutex_t Fitsio_Lock;
int Fitsio_Pthread_Status = 0;
#endif

/* Define docstrings */
static char module_docstring[] = "Core compression/decompression functions";
static char compress_plio_1_c_docstring[] = "Compress data using PLIO_1";
static char decompress_plio_1_c_docstring[] = "Decompress data using PLIO_1";
static char compress_rice_1_c_docstring[] = "Compress data using RICE_1";
static char decompress_rice_1_c_docstring[] = "Decompress data using RICE_1";
static char compress_hcompress_1_c_docstring[] =
    "Compress data using HCOMPRESS_1";
static char decompress_hcompress_1_c_docstring[] =
    "Decompress data using HCOMPRESS_1";
static char quantize_float_c_docstring[] = "Quantize float data";
static char quantize_double_c_docstring[] = "Quantize float data";
static char unquantize_float_c_docstring[] = "Unquantize data to float";
static char unquantize_double_c_docstring[] = "Unquantize data to double";

/* Declare the C functions here. */
static PyObject *compress_plio_1_c(PyObject *self, PyObject *args);
static PyObject *decompress_plio_1_c(PyObject *self, PyObject *args);
static PyObject *compress_rice_1_c(PyObject *self, PyObject *args);
static PyObject *decompress_rice_1_c(PyObject *self, PyObject *args);
static PyObject *compress_hcompress_1_c(PyObject *self, PyObject *args);
static PyObject *decompress_hcompress_1_c(PyObject *self, PyObject *args);
static PyObject *quantize_float_c(PyObject *self, PyObject *args);
static PyObject *quantize_double_c(PyObject *self, PyObject *args);
static PyObject *unquantize_float_c(PyObject *self, PyObject *args);
static PyObject *unquantize_double_c(PyObject *self, PyObject *args);

/* Define the methods that will be available on the module. */
static PyMethodDef module_methods[] = {
    {"compress_plio_1_c", compress_plio_1_c, METH_VARARGS, compress_plio_1_c_docstring},
    {"decompress_plio_1_c", decompress_plio_1_c, METH_VARARGS, decompress_plio_1_c_docstring},
    {"compress_rice_1_c", compress_rice_1_c, METH_VARARGS, compress_rice_1_c_docstring},
    {"decompress_rice_1_c", decompress_rice_1_c, METH_VARARGS, decompress_rice_1_c_docstring},
    {"compress_hcompress_1_c", compress_hcompress_1_c, METH_VARARGS, compress_hcompress_1_c_docstring},
    {"decompress_hcompress_1_c", decompress_hcompress_1_c, METH_VARARGS, decompress_hcompress_1_c_docstring},
    {"quantize_float_c", quantize_float_c, METH_VARARGS, quantize_float_c_docstring},
    {"quantize_double_c", quantize_double_c, METH_VARARGS, quantize_double_c_docstring},
    {"unquantize_float_c", unquantize_float_c, METH_VARARGS, unquantize_float_c_docstring},
    {"unquantize_double_c", unquantize_double_c, METH_VARARGS, unquantize_double_c_docstring},
    {NULL, NULL, 0, NULL}
};

/* This is the function that is called on import. */

#define MOD_ERROR_VAL NULL
#define MOD_SUCCESS_VAL(val) val
#define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
#define MOD_DEF(ob, name, doc, methods)                                                                                                                                                                \
  static struct PyModuleDef moduledef = {                                                                                                                                                              \
      PyModuleDef_HEAD_INIT, name, doc, -1, methods,                                                                                                                                                   \
  };                                                                                                                                                                                                   \
  ob = PyModule_Create(&moduledef);

MOD_INIT(_compression) {
  PyObject *m;
  MOD_DEF(m, "_compression", module_docstring, module_methods);
  if (m == NULL)
    return MOD_ERROR_VAL;
  return MOD_SUCCESS_VAL(m);
}

/* PLIO/IRAF compression */

static PyObject *compress_plio_1_c(PyObject *self, PyObject *args) {

  const char *str;
  char *buf;
  Py_ssize_t count;
  PyObject *result;

  int maxelem;
  int tilesize;
  short *compressed_values;
  int compressed_length;
  int *decompressed_values;

  if (!PyArg_ParseTuple(args, "y#i", &str, &count, &tilesize)) {
    return NULL;
  }

  // For PLIO imcomp_calc_max_elem in cfitsio does this to calculate max memory:
  maxelem = tilesize * sizeof(int);
  compressed_values = (short *)malloc(maxelem);

  decompressed_values = (int *)str;

  // Zero the compressed values array
  for (int i = 0; i < maxelem / 2; i++) {
    compressed_values[i] = 0;
  }

  compressed_length = pl_p2li(decompressed_values, 1, compressed_values, tilesize);

  buf = (char *)compressed_values;

  result = Py_BuildValue("y#", buf, compressed_length * 2);
  free(buf);
  return result;
}

static PyObject *decompress_plio_1_c(PyObject *self, PyObject *args) {

  const char *str;
  char *buf;
  Py_ssize_t count;
  PyObject *result;

  int tilesize;

  short *compressed_values;
  int *decompressed_values;

  if (!PyArg_ParseTuple(args, "y#i", &str, &count, &tilesize)) {
    return NULL;
  }

  compressed_values = (short *)str;

  decompressed_values = (int *)malloc(sizeof(int) * tilesize);

  pl_l2pi(compressed_values, 1, decompressed_values, tilesize);

  buf = (char *)decompressed_values;

  result = Py_BuildValue("y#", buf, tilesize * 4);
  free(buf);
  return result;
}

/* RICE compression */

static PyObject *compress_rice_1_c(PyObject *self, PyObject *args) {

  const char *str;
  Py_ssize_t count;
  PyObject *result;

  int blocksize, bytepix;

  int maxelem;
  unsigned char *compressed_values;
  int compressed_length;
  signed char *decompressed_values_byte;
  short *decompressed_values_short;
  int *decompressed_values_int;

  if (!PyArg_ParseTuple(args, "y#ii", &str, &count, &blocksize, &bytepix)) {
    return NULL;
  }

  // maxelem adapted from cfitsio's imcomp_calc_max_elem function
  maxelem = count + count / bytepix / blocksize + 2 + 4;

  compressed_values = (unsigned char *)malloc(maxelem);

  if (bytepix == 1) {
    decompressed_values_byte = (signed char *)str;
    compressed_length = fits_rcomp_byte(decompressed_values_byte, (int)count, compressed_values, count * 16, blocksize);
  } else if (bytepix == 2) {
    decompressed_values_short = (short *)str;
    compressed_length = fits_rcomp_short(decompressed_values_short, (int)count / 2, compressed_values, count * 16, blocksize);
  } else {
    decompressed_values_int = (int *)str;
    compressed_length = fits_rcomp(decompressed_values_int, (int)count / 4, compressed_values, count * 16, blocksize);
  }

  result = Py_BuildValue("y#", compressed_values, compressed_length);
  free(compressed_values);
  return result;
}

static PyObject *decompress_rice_1_c(PyObject *self, PyObject *args) {

  const char *str;
  char *dbytes;
  Py_ssize_t count;
  PyObject *result;

  int blocksize, bytepix, tilesize;

  unsigned char *compressed_values;
  unsigned char *decompressed_values_byte;
  unsigned short *decompressed_values_short;
  unsigned int *decompressed_values_int;

  if (!PyArg_ParseTuple(args, "y#iii", &str, &count, &blocksize, &bytepix, &tilesize)) {
    return NULL;
  }

  compressed_values = (unsigned char *)str;

  if (bytepix == 1) {
    decompressed_values_byte = (unsigned char *)malloc(tilesize);
    fits_rdecomp_byte(compressed_values, (int)count, decompressed_values_byte, tilesize, blocksize);
    dbytes = (char *)decompressed_values_byte;
  } else if (bytepix == 2) {
    decompressed_values_short = (unsigned short *)malloc(tilesize * 2);
    fits_rdecomp_short(compressed_values, (int)count, decompressed_values_short, tilesize, blocksize);
    dbytes = (char *)decompressed_values_short;
  } else {
    decompressed_values_int = (unsigned int *)malloc(tilesize * 4);
    fits_rdecomp(compressed_values, (int)count, decompressed_values_int, tilesize, blocksize);
    dbytes = (char *)decompressed_values_int;
  }

  result = Py_BuildValue("y#", dbytes, tilesize * bytepix);
  free(dbytes);
  return result;
}

/* HCompress compression */

static PyObject *compress_hcompress_1_c(PyObject *self, PyObject *args) {

  const char *str;
  Py_ssize_t count;
  PyObject *result;

  int bytepix, nx, ny, scale;
  int status=0;  // Important to initialize this to zero otherwise will fail silently

  int maxelem;
  char *compressed_values;
  int *decompressed_values_int;
  long long *decompressed_values_longlong;

  if (!PyArg_ParseTuple(args, "y#iiii", &str, &count, &nx, &ny, &scale, &bytepix)) {
    return NULL;
  }

  // maxelem adapted from cfitsio's imcomp_calc_max_elem function
  maxelem = count / 4 * 2.2 + 26;

  compressed_values = (char *)malloc(maxelem);

  if (bytepix == 4) {
    decompressed_values_int = (int *)str;
    fits_hcompress(decompressed_values_int, ny, nx, scale, compressed_values, &count, &status);
  } else {
    decompressed_values_longlong = (long long *)str;
    fits_hcompress64(decompressed_values_longlong, ny, nx, scale, compressed_values, &count, &status);
  }

  result = Py_BuildValue("y#", compressed_values, count);
  free(compressed_values);
  return result;
}

static PyObject *decompress_hcompress_1_c(PyObject *self, PyObject *args) {

  const unsigned char *str;
  char *dbytes;
  Py_ssize_t count;
  PyObject *result;

  int bytepix, nx, ny, scale, smooth;
  int status=0;  // Important to initialize this to zero otherwise will fail silently

  unsigned char *compressed_values;
  int *decompressed_values_int;
  long long *decompressed_values_longlong;

  if (!PyArg_ParseTuple(args, "y#iiiii", &str, &count, &nx, &ny, &scale, &smooth, &bytepix)) {
    return NULL;
  }

  compressed_values = (unsigned char *)str;

  // TODO: raise an error if bytepix is not 4 or 8

  dbytes = malloc(nx * ny * bytepix);

  if (bytepix == 4) {
    decompressed_values_int = (int *)dbytes;
    fits_hdecompress(compressed_values, smooth, decompressed_values_int, &ny, &nx, &scale, &status);
  } else {
    decompressed_values_longlong = (long long *)dbytes;
    fits_hdecompress64(compressed_values, smooth, decompressed_values_longlong, &ny, &nx, &scale, &status);
  }

  result = Py_BuildValue("y#", dbytes, nx * ny * bytepix);
  free(dbytes);
  return result;
}

static PyObject *quantize_float_c(PyObject *self, PyObject *args) {

  const char *input_bytes;
  Py_ssize_t nbytes;
  PyObject *result;

  float *input_data;

  long row, nx, ny;
  int nullcheck;
  float in_null_value;
  float qlevel;
  int dither_method;

  int *quantized_data;
  char *quantized_bytes;
  double bscale, bzero;
  int iminval, imaxval;

  int status;

  if (!PyArg_ParseTuple(args, "y#lllidfi", &input_bytes, &nbytes, &row, &nx,
                        &ny, &nullcheck, &in_null_value, &qlevel,
                        &dither_method)) {
    return NULL;
  }

  input_data = (float *)input_bytes;
  quantized_data = (int *)malloc(nx * ny * sizeof(int));

  status = fits_quantize_float(row, input_data, nx, ny, nullcheck, in_null_value, qlevel,
                               dither_method, quantized_data, &bscale, &bzero, &iminval,
                               &imaxval);

  quantized_bytes = (char *)quantized_data;

  result = Py_BuildValue("y#iddii", quantized_bytes, nx * ny * sizeof(int), status,
                                   bscale, bzero, iminval, imaxval);
  free(quantized_bytes);
  return result;
}

static PyObject *quantize_double_c(PyObject *self, PyObject *args) {

  const char *input_bytes;
  Py_ssize_t nbytes;
  PyObject *result;

  double *input_data;

  long row, nx, ny;
  int nullcheck;
  double in_null_value;
  float qlevel;
  int dither_method;

  int *quantized_data;
  char *quantized_bytes;
  double bscale, bzero;
  int iminval, imaxval;

  int status;

  if (!PyArg_ParseTuple(args, "y#lllidfi", &input_bytes, &nbytes, &row, &nx,
                        &ny, &nullcheck, &in_null_value, &qlevel,
                        &dither_method)) {
    return NULL;
  }

  input_data = (double *)input_bytes;
  quantized_data = (int *)malloc(nx * ny * sizeof(int));

  status = fits_quantize_double(row, input_data, nx, ny, nullcheck, in_null_value,
                                qlevel, dither_method, quantized_data, &bscale, &bzero,
                                &iminval, &imaxval);

  quantized_bytes = (char *)quantized_data;

  result = Py_BuildValue("y#iddii", quantized_bytes, nx * ny * sizeof(int), status,
                                    bscale, bzero, iminval, imaxval);
  free(quantized_bytes);
  return result;
}

static PyObject *unquantize_float_c(PyObject *self, PyObject *args) {

  const char *input_bytes;
  Py_ssize_t nbytes;
  PyObject *result;

  long row, npix;
  int nullcheck;
  int tnull;
  float nullval;
  int dither_method;

  double bscale, bzero;
  int bytepix; // int size
  int status = 0;

  int *anynull;
  float *output_data;
  char *output_bytes;

  if (!PyArg_ParseTuple(args, "y#llddiiifi", &input_bytes, &nbytes, &row, &npix,
                        &bscale, &bzero, &dither_method, &nullcheck, &tnull,
                        &nullval, &bytepix)) {
    return NULL;
  }

// TODO: add support, if needed, for nullcheck=1

anynull = (int *)malloc(npix * sizeof(int));
output_data = (float *)malloc(npix * sizeof(float));

if (bytepix == 1) {
    unquantize_i1r4(row, (unsigned char *)input_bytes, npix, bscale, bzero,
                    dither_method, nullcheck, (unsigned char)tnull, nullval,
                    NULL, anynull, output_data, &status);
} else if (bytepix == 2) {
    unquantize_i2r4(row, (short *)input_bytes, npix, bscale, bzero,
                    dither_method, nullcheck, (short)tnull, nullval, NULL,
                    anynull, output_data, &status);
} else if (bytepix == 4) {
    unquantize_i4r4(row, (int *)input_bytes, npix, bscale, bzero, dither_method,
                    nullcheck, (int)tnull, nullval, NULL, anynull, output_data,
                    &status);
}

output_bytes = (char *)output_data;

  result = Py_BuildValue("y#", output_bytes, npix * sizeof(float));
  free(output_bytes);
  return result;
}

static PyObject *unquantize_double_c(PyObject *self, PyObject *args) {

  const char *input_bytes;
  Py_ssize_t nbytes;
  PyObject *result;

  long row, npix;
  int nullcheck;
  int tnull;
  double nullval;
  int dither_method;

  double bscale, bzero;
  int bytepix; // int size
  int status = 0;

  int *anynull;
  double *output_data;
  char *output_bytes;

  if (!PyArg_ParseTuple(args, "y#llddiiidi", &input_bytes, &nbytes, &row, &npix,
                        &bscale, &bzero, &dither_method, &nullcheck, &tnull,
                        &nullval, &bytepix)) {
    return NULL;
  }

// TODO: add support, if needed, for nullcheck=1

anynull = (int *)malloc(npix * sizeof(int));
output_data = (double *)malloc(npix * sizeof(double));

if (bytepix == 1) {
    unquantize_i1r8(row, (unsigned char *)input_bytes, npix, bscale, bzero,
                    dither_method, nullcheck, (unsigned char)tnull, nullval,
                    NULL, anynull, output_data, &status);
} else if (bytepix == 2) {
    unquantize_i2r8(row, (short *)input_bytes, npix, bscale, bzero,
                    dither_method, nullcheck, (short)tnull, nullval, NULL,
                    anynull, output_data, &status);
} else if (bytepix == 4) {
    unquantize_i4r8(row, (int *)input_bytes, npix, bscale, bzero, dither_method,
                    nullcheck, (int)tnull, nullval, NULL, anynull, output_data,
                    &status);
}

output_bytes = (char *)output_data;

  result = Py_BuildValue("y#", output_bytes, npix * sizeof(double));
  free(output_bytes);
  return result;
}