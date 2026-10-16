#include <gtest/gtest.h>

#ifdef _MSC_VER
#include "fmatrix.h"
#else
extern "C" {
#include "fmatrix.h"
}
#endif

namespace {

class MatrixApiTest : public ::testing::Test {
  protected:
    static void SetUpTestSuite() {
    }

    static void TearDownTestSuite() {
    }
};

TEST_F(MatrixApiTest, AllocAndSetGet) {
    struct fmatrix* A = fmat_alloc(2, 3);
    ASSERT_NE(A, nullptr);

    fmat_set(A, 0, 1, 5.0);
    fmat_set(A, 1, 2, -3.0);

    EXPECT_DOUBLE_EQ(fmat_get(A, 0, 1), 5.0);
    EXPECT_DOUBLE_EQ(fmat_get(A, 1, 2), -3.0);

    fmat_free(A);
}

TEST_F(MatrixApiTest, HeapCreation) {
    struct fmatrix* A = fmat_alloc(3, 3);
    ASSERT_NE(A, nullptr);

    fmat_set(A, 0, 0, 1);
    fmat_set(A, 0, 1, 1);
    fmat_set(A, 0, 2, 1);

    EXPECT_NEAR(fmat_get(A, 0, 1), 1, 1e-12);
    EXPECT_NEAR(fmat_get(A, 0, 0), 1, 1e-12);
    EXPECT_NEAR(fmat_get(A, 0, 2), 1, 1e-12);
    EXPECT_NEAR(fmat_get(A, 1, 0), 0, 1e-12);
    EXPECT_NEAR(fmat_get(A, 1, 1), 0, 1e-12);
    EXPECT_NEAR(fmat_get(A, 1, 2), 0, 1e-12);
    EXPECT_NEAR(fmat_get(A, 2, 0), 0, 1e-12);
    EXPECT_NEAR(fmat_get(A, 2, 1), 0, 1e-12);
    EXPECT_NEAR(fmat_get(A, 2, 2), 0, 1e-12);

    fmat_free(A);
}

TEST_F(MatrixApiTest, Multiplication) {
    struct fmatrix* A = fmat_alloc(3, 3);
    struct fmatrix* B = fmat_alloc(3, 3);
    ASSERT_NE(A, nullptr);
    ASSERT_NE(B, nullptr);

    // A = [1 1 1; 0 0 0; 0 0 0]
    fmat_set(A, 0, 0, 1);
    fmat_set(A, 0, 1, 1);
    fmat_set(A, 0, 2, 1);

    // B = column of ones
    fmat_set(B, 0, 0, 1);
    fmat_set(B, 1, 0, 1);
    fmat_set(B, 2, 0, 1);

    struct fmatrix* C = fmat_mul(nullptr, A, B);
    ASSERT_NE(C, nullptr);

    EXPECT_NEAR(fmat_get(C, 0, 0), 3.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 0, 1), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 0, 2), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 1, 0), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 1, 1), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 1, 2), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 2, 0), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 2, 1), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(C, 2, 2), 0.0, 1e-12);

    fmat_free(A);
    fmat_free(B);
    fmat_free(C);
}

TEST_F(MatrixApiTest, Transposition) {
    struct fmatrix* A = fmat_alloc(2, 3);
    ASSERT_NE(A, nullptr);

    fmat_set(A, 0, 1, 1);
    fmat_set(A, 1, 2, 1);

    struct fmatrix* T = fmat_trans(nullptr, A);
    ASSERT_NE(T, nullptr);

    EXPECT_NEAR(fmat_get(T, 0, 0), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(T, 0, 1), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(T, 1, 0), 1.0, 1e-12);
    EXPECT_NEAR(fmat_get(T, 1, 1), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(T, 2, 0), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(T, 2, 1), 1.0, 1e-12);

    fmat_free(A);
    fmat_free(T);
}

TEST_F(MatrixApiTest, Inverse) {
    struct fmatrix* A = fmat_alloc(3, 3);
    ASSERT_NE(A, nullptr);

    fmat_set(A, 0, 0, 2);
    fmat_set(A, 1, 0, 1);
    fmat_set(A, 2, 0, 0);

    fmat_set(A, 0, 1, -1);
    fmat_set(A, 1, 1, 2);
    fmat_set(A, 2, 1, -1);

    fmat_set(A, 0, 2, 0);
    fmat_set(A, 1, 2, -2);
    fmat_set(A, 2, 2, 1);

    struct fmatrix* Ainv = fmat_inv(nullptr, A);
    ASSERT_NE(Ainv, nullptr);

    EXPECT_NEAR(fmat_get(Ainv, 0, 0), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(Ainv, 0, 1), 1.0, 1e-12);
    EXPECT_NEAR(fmat_get(Ainv, 0, 2), 2.0, 1e-12);

    EXPECT_NEAR(fmat_get(Ainv, 1, 0), -1.0, 1e-12);
    EXPECT_NEAR(fmat_get(Ainv, 1, 1), 2.0, 1e-12);
    EXPECT_NEAR(fmat_get(Ainv, 1, 2), 4.0, 1e-12);

    EXPECT_NEAR(fmat_get(Ainv, 2, 0), -1.0, 1e-12);
    EXPECT_NEAR(fmat_get(Ainv, 2, 1), 2.0, 1e-12);
    EXPECT_NEAR(fmat_get(Ainv, 2, 2), 5.0, 1e-12);

    fmat_free(A);
    fmat_free(Ainv);
}

TEST_F(MatrixApiTest, InverseProperty) {
    struct fmatrix* A = fmat_alloc(2, 2);
    ASSERT_NE(A, nullptr);

    fmat_set(A, 0, 0, 4);
    fmat_set(A, 0, 1, 7);
    fmat_set(A, 1, 0, 2);
    fmat_set(A, 1, 1, 6);

    struct fmatrix* Ainv = fmat_inv(nullptr, A);
    ASSERT_NE(Ainv, nullptr);

    struct fmatrix* I = fmat_mul(nullptr, A, Ainv);
    ASSERT_NE(I, nullptr);

    EXPECT_NEAR(fmat_get(I, 0, 0), 1.0, 1e-12);
    EXPECT_NEAR(fmat_get(I, 0, 1), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(I, 1, 0), 0.0, 1e-12);
    EXPECT_NEAR(fmat_get(I, 1, 1), 1.0, 1e-12);

    fmat_free(A);
    fmat_free(Ainv);
    fmat_free(I);
}

} // namespace
