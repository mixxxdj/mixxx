#ifndef FMATRIX_H
#define FMATRIX_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Scalar type for matrix elements */
typedef double fval_t;

#ifdef DEBUG
#define FMAT_SET(m, row, col, val) fmat_set(m, row, col, val)
#define FMAT_GET(m, row, col) fmat_get(m, row, col)
#else
#define FMAT_SET(m, row, col, val) fmat_set_unsafe(m, row, col, val)
#define FMAT_GET(m, row, col) fmat_get_unsafe(m, row, col)
#endif

/* Allocate an empty floating-point matrix */
struct fmatrix *fmat_alloc(const size_t rows, const size_t cols);
/* Delete a floating-point matrix */
void fmat_free(struct fmatrix *m);

/* Set a single field of a floating-point matrix (slower safe version for testing) */
void fmat_set(struct fmatrix *m, size_t row, size_t col, fval_t val);
/* Set a single field of a floating-point matrix (unsafe inlined version) */
void fmat_set_unsafe(struct fmatrix *m, size_t row, size_t col, fval_t val);
/* Get a single field of a floating-point matrix (slower safe version for testing) */
fval_t fmat_get(const struct fmatrix *m, size_t row, size_t col);
/* Get a single field of a floating-point matrix (unsafe inlined version) */
fval_t fmat_get_unsafe(const struct fmatrix *m, size_t row, size_t col);

/* Multiply two floating-point matrices */
struct fmatrix *fmat_mul(struct fmatrix *dest, const struct fmatrix *a, const struct fmatrix *b);
/* Transpose a floating-point matrix */
struct fmatrix *fmat_trans(struct fmatrix *dest, const struct fmatrix *src);
/* Compute the inverse of a floating-point matrix */
struct fmatrix *fmat_inv(struct fmatrix *dest, const struct fmatrix *src);

#ifdef __cplusplus
}
#endif

#endif /* FMATRIX_H */
