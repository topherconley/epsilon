
#include <gtest/gtest.h>
#include <glog/logging.h>

#include "epsilon/vector/matrix_variant.h"
#include "epsilon/vector/vector_testutil.h"

namespace {

typedef Eigen::MatrixXd DenseMatrix;
typedef Eigen::SparseMatrix<double> SparseMatrix;

class MatrixVariantTest : public testing::Test {
 protected:
  MatrixVariantTest() : A_(2,2), B_(2,2) {
    A_ << 1, 2, 3, 4;
    B_ << -1, 2, -3, 4;

    C_ = SparseMatrix(2, 2);
    C_.coeffRef(0,1) = 1;
    C_.coeffRef(1,0) = -1;

    D_ = SparseMatrix(2, 2);
    D_.coeffRef(0,0) = 1;
    D_.coeffRef(1,1) = 2;

    C2_ = C_;
    D2_ = D_;
  }

  DenseMatrix A_, B_, C2_, D2_;
  SparseMatrix C_, D_;
};

TEST_F(MatrixVariantTest, Assignment) {
  MatrixVariant X = MatrixVariant(A_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_));
  X = MatrixVariant(B_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), B_));
  X = MatrixVariant(C_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), C_));
  X = MatrixVariant(D_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), D_));
  X = MatrixVariant(A_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_));
  MatrixVariant Y = X;
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), A_));
}

TEST_F(MatrixVariantTest, Add) {
  MatrixVariant X = MatrixVariant(A_);
  X += MatrixVariant(B_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_+B_));
  X += MatrixVariant(C_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_+B_+C2_));
  X += MatrixVariant(D_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_+B_+C2_+D2_));

  MatrixVariant Y = MatrixVariant(C_);
  Y += MatrixVariant(D_);
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), C2_+D2_));
  Y += MatrixVariant(A_);
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), C2_+D2_+A_));
  Y += MatrixVariant(B_);
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), C2_+D2_+A_+B_));

  X += Y + Y;
  EXPECT_TRUE(MatrixEquals(X.AsDense(), 3*(A_+B_+C2_+D2_)));
}

TEST_F(MatrixVariantTest, Multiply) {
  MatrixVariant X = MatrixVariant(A_);
  X *= MatrixVariant(B_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_ * B_));
  X *= MatrixVariant(C_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_ * B_ * C_));
  X *= MatrixVariant(D_);
  EXPECT_TRUE(MatrixEquals(X.AsDense(), A_ * B_ * C_ * D_));

  MatrixVariant Y = MatrixVariant(C_);
  Y *= MatrixVariant(D_);
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), (C_ * D_).eval()));
  Y *= MatrixVariant(A_);
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), C_ * D_ * A_));

  Y = Y * X;
  EXPECT_TRUE(MatrixEquals(Y.AsDense(), C_*D_*A_*A_*B_*C_*D_));
}

}  // namespace