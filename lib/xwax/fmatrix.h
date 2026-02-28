#ifndef FMATRIX_H
#define FMATRIX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Scalar type for matrix elements */
typedef double fval_t;

struct fmatrix {
	const size_t cols, rows;
	fval_t **data;
};

/* Allocate an empty floating-point matrix */
struct fmatrix *fmat_alloc(const size_t rows, const size_t cols);
/* Delete a floating-point matrix */
void fmat_free(struct fmatrix *m);

/* Set a single field of a floating-point matrix */
void fmat_set(struct fmatrix *m, size_t row, size_t col, fval_t val);
/* Get a single field of a floating-point matrix */
fval_t fmat_get(struct fmatrix *m, size_t row, size_t col);

/* Multiply two floating-point matrices */
struct fmatrix *fmat_mul(struct fmatrix *dest, const struct fmatrix *a, const struct fmatrix *b);;
/* Transpose a floating-point matrix */
struct fmatrix *fmat_trans(struct fmatrix *dest, const struct fmatrix *src);
/* Compute the inverse of a floating-point matrix */
struct fmatrix *fmat_inv(struct fmatrix *dest, const struct fmatrix *src);

#endif /* FMATRIX_H */
