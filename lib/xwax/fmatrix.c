#include "fmatrix.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>

struct fmatrix *fmat_alloc(const size_t rows, const size_t cols)
{
	if (!rows || !cols) {
		errno = EINVAL;
		perror(__func__);
		return NULL;
	}

	size_t row;

	/* Allocate the struct */
	struct fmatrix *m = malloc(sizeof(struct fmatrix));
	if (!m) {
		perror(__func__);
		goto error;
	}

	/* Copy over a temporary matrix that holds the const rows and columns */
	struct fmatrix m_temp = { .cols = cols, .rows = rows, .data = NULL };
	memcpy(m, &m_temp, sizeof(struct fmatrix));

	/* Allocate the rows */
	m->data = calloc(rows, sizeof(fval_t *));
	if (!m->data) {
		perror(__func__);
		goto error_rows;
	}

	/* Allocate the columns */
	for (row = 0; row < rows; row++) {
		m->data[row] = calloc(cols, sizeof(fval_t));
		if (!m->data[row]) {
			perror(__func__);
			goto error_columns;
		}
	}

	return m;

error_columns:
	while (row > 0) {
		row--;
		free(m->data[row]);
	}

	free(m->data);
error_rows:
	free(m);
error:
	return NULL;
}

void fmat_free(struct fmatrix *m)
{
	if (!m)
		return;

	for (size_t row = 0; row < m->rows; row++)
		if (m->data[row])
			free(m->data[row]);

	if (m->data)
		free(m->data);

	free(m);
}

void fmat_set(struct fmatrix *m, size_t row, size_t col, fval_t val)
{
	if (!m || row > m->rows || col > m->cols) {
		errno = EINVAL;
		perror(__func__);
		return;
	}

	m->data[row][col] = val;
}

fval_t fmat_get(struct fmatrix *m, size_t row, size_t col)
{
	if (!m || row > m->rows || col > m->cols) {
		errno = EINVAL;
		perror(__func__);
		return 0.0;
	}

	return m->data[row][col];
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
				sum += a->data[i][k] * b->data[k][j];
			dest->data[i][j] = sum;
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
			dest->data[c][r] = src->data[r][c];

	return dest;
}

struct fmatrix *fmat_inv(struct fmatrix *dest, const struct fmatrix *src)
{
	if (!src || src->rows != src->cols) {
		errno = EINVAL;
		perror(__func__);
		return NULL;
	}

	if (dest) {
		if (dest->rows != src->rows || dest->cols != src->cols) {
			errno = EINVAL;
			perror(__func__);
			return NULL;
		}
	} else {
		dest = fmat_alloc(src->rows, src->cols);
		if (!dest)
			return NULL;
	}

	struct fmatrix *tmp_mat = fmat_alloc(src->rows, src->cols);
	if (!tmp_mat) {
		perror(__func__);
		fmat_free(dest);
		return NULL;
	}

	double *inv_row = NULL;
	double *a_row = NULL;
	double *inv_i = NULL;
	double factor = 0.0;
	double pivot = 0.0;
	double *a_i = NULL;
	size_t max_row = 0;
	double tmp = 0.0;

	/* Copy src to a, initialize inv as identity */

	for (size_t r = 0; r < src->rows; r++)
		for (size_t c = 0; c < src->cols; c++) {
			tmp_mat->data[r][c] = (double)src->data[r][c];
			dest->data[r][c] = (r == c) ? 1.0 : 0.0;
		}

	for (size_t r = 0; r < src->rows; r++) {
		/* Pivot search */

		max_row = r;

		for (size_t i = r; i < src->rows; i++)
			if (fabs(tmp_mat->data[i][r]) > fabs(tmp_mat->data[max_row][r]))
				max_row = i;

		if (fabs(tmp_mat->data[max_row][r]) < 1e-12) {
			fprintf(stderr, "%s: matrix is singular\n", __func__);
			fmat_free(tmp_mat);
			fmat_free(dest);
			return NULL;
		}

		/* Swap rows in a and inv */

		if (max_row != r)
			for (size_t c = 0; c < src->cols; c++) {
				tmp = tmp_mat->data[r][c];
				tmp_mat->data[r][c] = tmp_mat->data[max_row][c];
				tmp_mat->data[max_row][c] = tmp;

				tmp = dest->data[r][c];
				dest->data[r][c] = dest->data[max_row][c];
				dest->data[max_row][c] = tmp;
			}

		/* Cache pivot row */

		a_row = tmp_mat->data[r];
		inv_row = dest->data[r];
		pivot = a_row[r];

		/* Eliminate other rows */

		for (size_t i = 0; i < src->rows; i++) {
			if (i == r)
				continue;

			factor = tmp_mat->data[i][r] / pivot;
			a_i = tmp_mat->data[i];
			inv_i = dest->data[i];

			for (size_t c = 0; c < src->cols; c++) {
				a_i[c] -= factor * a_row[c];
				inv_i[c] -= factor * inv_row[c];
			}
		}

		/* Normalize pivot row */

		for (size_t c = 0; c < src->cols; c++) {
			a_row[c] /= pivot;
			inv_row[c] /= pivot;
		}
	}

	fmat_free(tmp_mat);

	return dest;
}
