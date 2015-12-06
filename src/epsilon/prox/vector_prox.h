#ifndef EPSILON_PROX_VECTOR_H
#define EPSILON_PROX_VECTOR_H

#include "epsilon/prox/prox.h"

class VectorProxInput {
 public:
  double lambda() const;
  const Eigen::VectorXd& lambda_vec() const;

  double value(int i) const;
  const Eigen::VectorXd& value_vec(int i) const;

 private:
  friend class VectorProx;

  bool elementwise_;
  Eigen::VectorXd lambda_vec_;
  double lambda_;
  BlockVector v_;
};

class VectorProxOutput {
 public:
  void set_value(int i, double x);
  void set_value(int i, const Eigen::VectorXd& x);

 private:
  friend class VectorProx;

  BlockVector x_;
};

class VectorProx : public ProxOperator {
 public:
  void Init(const ProxOperatorArg& arg) override;
  BlockVector Apply(const BlockVector& v) override;

  // To be overriden by subclasses
  virtual void ApplyVector(
      const VectorProxInput& input,
      VectorProxOutput* output) = 0;

 private:
  void InitArgs(const AffineOperator& f);
  void InitConstraints(const AffineOperator& f);
  void InitInput();

  void PreProcessInput(const BlockVector& v);
  BlockVector PostProcessOutput();

  BlockMatrix MT_, H_inv_;
  BlockVector g_;

  // Used in ApplyVector()
  // NOTE(mwytock): Storing per-Apply() state like this makes VectorProx not
  // threadsafe which may be a consideration in future
  VectorProxInput input_;
  VectorProxOutput output_;
};

#endif  // EPSILON_PROX_VECTOR_H