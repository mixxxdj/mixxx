#include "fmatrix.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fmatrix {
	size_t cols, rows;
	fval_t *data;
};

struct fmatrix *fmat_alloc(const size_t rows, const size_t cols)
{
	if (!rows || !cols) {
		errno = EINVAL;
		perror(__func__);
		return NULL;
	}

	/* Allocate the struct */
	struct fmatrix *m = malloc(sizeof(struct fmatrix));
	if (!m) {
		perror(__func__);
		goto error;
	}

	m->cols = cols;
	m->rows = rows;
	m->data = NULL;

	/* Allocate the rows */
	m->data = calloc(rows * cols, sizeof(fval_t));
	if (!m->data) {
		perror(__func__);
		goto error_data;
	}

	return m;

error_data:
	free(m);
error:
	return NULL;
}

void fmat_free(struct fmatrix *m)
{
	if (!m)
		return;

	if (m->data)
		free(m->data);

	free(m);
}

void fmat_set(struct fmatrix *m, size_t row, size_t col, fval_t val)
{
	if (!m || row >= m->rows || col >= m->cols) {
		errno = EINVAL;
		perror(__func__);
		return;
	}

	m->data[m->cols * row + col] = val;
}

inline void fmat_set_unsafe(struct fmatrix *m, size_t row, size_t col, fval_t val)
{
	m->data[m->cols * row + col] = val;
}

fval_t fmat_get(const struct fmatrix *m, size_t row, size_t col)
{
	if (!m || row >= m->rows || col >= m->cols) {
		errno = EINVAL;
		perror(__func__);
		return 0;
	}

	return m->data[m->cols * row + col];
}

inline fval_t fmat_get_unsafe(const struct fmatrix *m, size_t row, size_t col)
{
	return m->data[m->cols * row + col];
}

struct fmatrix *fmat_mul(struct fmatrix *dest, const struct fmatrix *a, const struct fmatrix *b)
{
	if (!a || !b || a->cols != b->rows) {
		errno = EINVAL;
		perror(__func__);
		return NULL;
	}

	if (dest) {
		if (dest->rows != a->rows || dest->cols != b->cols) {
			errno = EINVAL;
			perror(__func__);
			return NULL;
		}
	} else {
		dest = fmat_alloc(a->rows, b->cols);
		if (!dest)
			return NULL;
	}

	for (size_t i = 0; i < a->rows; i++) {
		for (size_t j = 0; j < b->cols; j++) {
			fval_t sum = 0;

			for (size_t k = 0; k < a->cols; k++)
				sum += FMAT_GET(a, i, k) * FMAT_GET(b, k, j);

			FMAT_SET(dest, i, j, sum);
		}
	}

	return dest;
}

struct fmatrix *fmat_trans(struct fmatrix *dest, const struct fmatrix *src)
{
	if (!src) {
		errno = EINVAL;
		perror(__func__);
		return NULL;
	}

	if (dest) {
		if (dest->rows != src->cols || dest->cols != src->rows) {
			errno = EINVAL;
			perror(__func__);
			return NULL;
		}
	} else {
		dest = fmat_alloc(src->cols, src->rows);
		if (!dest)
			return NULL;
	}

	for (size_t r = 0; r < src->rows; r++)
		for (size_t c = 0; c < src->cols; c++)
			FMAT_SET(dest, c, r, FMAT_GET(src, r, c));

	return dest;
}

static int fmat_lup_decompose(struct fmatrix *a, size_t *perm, double *sign)
{
	const size_t n = a->rows;
	const double eps = 1e-12;

	for (size_t i = 0; i < n; ++i)
		perm[i] = i;

	*sign = 1.0;

	for (size_t k = 0; k < n; ++k) {
		size_t pivot_row = k;
		double max_abs = fabs(a->data[k * n + k]);

		for (size_t i = k + 1; i < n; ++i) {
			double v = fabs(a->data[i * n + k]);
			if (v > max_abs) {
				max_abs = v;
				pivot_row = i;
			}
		}

		if (max_abs < eps)
			return -1;

		if (pivot_row != k) {
			for (size_t j = 0; j < n; ++j) {
				double t = a->data[k * n + j];
				a->data[k * n + j] = a->data[pivot_row * n + j];
				a->data[pivot_row * n + j] = t;
			}
			size_t tp = perm[k];
			perm[k] = perm[pivot_row];
			perm[pivot_row] = tp;
			*sign = -*sign;
		}

		double pivot = a->data[k * n + k];

		for (size_t i = k + 1; i < n; ++i) {
			double *row_i = &a->data[i * n];
			double *row_k = &a->data[k * n];
			row_i[k] /= pivot;
			double lik = row_i[k];
			for (size_t j = k + 1; j < n; ++j)
				row_i[j] -= lik * row_k[j];
		}
	}

	return 0;
}

static void fmat_lu_solve(const struct fmatrix *lu, const size_t *perm, const double *b, double *x)
{
	const size_t n = lu->rows;

	for (size_t i = 0; i < n; ++i) {
		double sum = b[perm[i]];
		for (size_t j = 0; j < i; ++j)
			sum -= lu->data[i * n + j] * x[j];
		x[i] = sum;
	}

	for (size_t i = n; i-- > 0;) {
		double sum = x[i];
		for (size_t j = i + 1; j < n; ++j)
			sum -= lu->data[i * n + j] * x[j];
		x[i] = sum / lu->data[i * n + i];
	}
}

struct fmatrix *fmat_inv_lup(struct fmatrix *dest, const struct fmatrix *src)
{
	struct fmatrix *lu = NULL;
	int dest_allocated = 0;
	size_t *perm = NULL;
	double *rhs = NULL;
	double *sol = NULL;
	int error = -1;
	double sign;

	if (!src || src->rows != src->cols) {
		errno = EINVAL;
		goto out;
	}

	size_t n = src->rows;

	if (dest) {
		if (dest->rows != n || dest->cols != n) {
			errno = EINVAL;
			goto out;
		}
	} else {
		dest = fmat_alloc(n, n);
		if (!dest)
			goto out;
		dest_allocated = 1;
	}

	lu = fmat_alloc(n, n);
	perm = malloc(n * sizeof(*perm));
	rhs = malloc(n * sizeof(*rhs));
	sol = malloc(n * sizeof(*sol));
	if (!lu || !perm || !rhs || !sol)
		goto out;

	memcpy(lu->data, src->data, n * n * sizeof(double));

	if (fmat_lup_decompose(lu, perm, &sign) != 0) {
		errno = EINVAL;
		goto out;
	}

	for (size_t col = 0; col < n; ++col) {
		memset(rhs, 0, n * sizeof(*rhs));
		rhs[col] = 1.0;
		fmat_lu_solve(lu, perm, rhs, sol);

		for (size_t row = 0; row < n; ++row)
			dest->data[row * n + col] = sol[row];
	}

	error = 0;

out:
	free(sol);
	free(rhs);
	free(perm);
	fmat_free(lu);

	if (error) {
		if (dest_allocated)
			fmat_free(dest);
		perror(__func__);
		return NULL;
	}

	return dest;
}

static int fmat_inv_2x2(const struct fmatrix *src, struct fmatrix *dest)
{
	const double a = src->data[0];
	const double b = src->data[1];
	const double c = src->data[2];
	const double d = src->data[3];

	const double det = a * d - b * c;
	if (fabs(det) < 1e-12)
		return -1;

	const double inv_det = 1.0 / det;

	dest->data[0] = d * inv_det;
	dest->data[1] = -b * inv_det;
	dest->data[2] = -c * inv_det;
	dest->data[3] = a * inv_det;

	return 0;
}

static int fmat_inv_3x3(const struct fmatrix *src, struct fmatrix *dest)
{
	const double *m = src->data;

	const double a = m[0], b = m[1], c = m[2];
	const double d = m[3], e = m[4], f = m[5];
	const double g = m[6], h = m[7], i = m[8];

	const double A = e * i - f * h;
	const double B = -(d * i - f * g);
	const double C = d * h - e * g;
	const double D = -(b * i - c * h);
	const double E = a * i - c * g;
	const double F = -(a * h - b * g);
	const double G = b * f - c * e;
	const double H = -(a * f - c * d);
	const double I = a * e - b * d;

	const double det = a * A + b * B + c * C;
	if (fabs(det) < 1e-12)
		return -1;

	const double inv_det = 1.0 / det;

	dest->data[0] = A * inv_det;
	dest->data[1] = D * inv_det;
	dest->data[2] = G * inv_det;
	dest->data[3] = B * inv_det;
	dest->data[4] = E * inv_det;
	dest->data[5] = H * inv_det;
	dest->data[6] = C * inv_det;
	dest->data[7] = F * inv_det;
	dest->data[8] = I * inv_det;

	return 0;
}

struct fmatrix *fmat_inv(struct fmatrix *dest, const struct fmatrix *src)
{
	if (!src || src->rows != src->cols) {
		errno = EINVAL;
		perror(__func__);
		return NULL;
	}

	const size_t n = src->rows;

	if (dest) {
		if (dest->rows != n || dest->cols != n) {
			errno = EINVAL;
			perror(__func__);

			return NULL;
		}
	} else {
		dest = fmat_alloc(n, n);

		if (!dest)
			return NULL;
	}

	if (n == 1) {
		if (fabs(src->data[0]) < 1e-12) {
			fprintf(stderr, "%s: matrix is singular\n", __func__);
			goto error;
		}

		dest->data[0] = 1.0 / src->data[0];

		return dest;
	}

	if (n == 2) {
		if (fmat_inv_2x2(src, dest) != 0) {
			fprintf(stderr, "%s: matrix is singular\n", __func__);
			goto error;
		}

		return dest;
	}

	if (n == 3) {
		if (fmat_inv_3x3(src, dest) != 0) {
			fprintf(stderr, "%s: matrix is singular\n", __func__);
			goto error;
		}

		return dest;
	}

	return fmat_inv_lup(dest, src);

error:
	fmat_free(dest);
	return NULL;
}
