#include "epsilon/prox/newton.h"

#include "epsilon/affine/affine.h"
#include "epsilon/expression/expression_util.h"
#include "epsilon/prox/prox.h"
#include "epsilon/vector/dynamic_matrix.h"
#include "epsilon/vector/vector_util.h"

Eigen::VectorXd NewtonProx::residual(
    const Eigen::VectorXd &x, const Eigen::VectorXd &v, double lam) {
  return x-v+lam*f_->gradf(x);
}

Eigen::VectorXd NewtonProx::Apply(const Eigen::VectorXd &v) {
  int n = v.rows();
  double eps = std::max(1e-12, 1e-10/n);

  // init
  Eigen::VectorXd x = f_->proj_feasible(v);

  int iter = 0;
  int MAX_ITER = 100;
  for(; iter < MAX_ITER; iter++) {
    Eigen::VectorXd hx = Eigen::VectorXd::Constant(n, 1.) + lambda_ * f_->hessf(x);
    Eigen::VectorXd gx = residual(x, v, lambda_);
    Eigen::VectorXd dx = (gx.array() / hx.array()).matrix();
    VLOG(2) << "Iter " << iter << " gx: " << VectorDebugString(gx);

    // line search
    double beta = 0.001;
    double gamma = 0.5;
    double theta = 1;
    double x_res = gx.norm();
    while(theta > eps) {
      Eigen::VectorXd nx = x - theta * dx;
      double nx_res = residual(nx, v, lambda_).norm();
      if(nx_res <= (1-beta*theta)*x_res) {
        x = nx;
        x_res = nx_res;
        break;
      }
      theta *= gamma;
    }

    if(x_res < eps) {
      VLOG(1) << "Using " << iter+1 << " Newton iteration.\n";
      break;
    } else if(iter == MAX_ITER-1) {
      VLOG(1) << "Newton Method won't converge for epigraph.\n";
    }
  }
  return x;
}

Eigen::VectorXd NewtonEpigraph::residual
(const Eigen::VectorXd &x, double t, double lam, const Eigen::VectorXd &v, double s) {
  int n = x.rows();
  VectorXd r(x.rows()+2);
  r.head(n) = x-v+lam*f_->gradf(x);
  r(n) = t-s-lam;
  r(n+1) = f_->eval(x)-t;
  return r;
}

Eigen::VectorXd NewtonEpigraph::Apply(const Eigen::VectorXd &sv) {
  int n = sv.rows()-1;
  double eps = std::max(1e-12, 1e-10/n);

  double s = sv(0);
  Eigen::VectorXd v = sv.tail(n);

  // easy case
  if (f_->eval(v) <= s)
    return sv;

  // init
  Eigen::VectorXd x = f_->proj_feasible(v);
  double t = s;
  double lam = 1;

  int iter = 0;
  int MAX_ITER = 100;
  for(; iter < MAX_ITER; iter++) {
    Eigen::VectorXd hx = Eigen::VectorXd::Constant(n, 1.) + lam * f_->hessf(x);
    Eigen::VectorXd g = residual(x, t, lam, v, s);
    VLOG(2) << "Iter " << iter << "\n"
            << " x: " << VectorDebugString(x) << "\n"
            << " g: " << VectorDebugString(g) << "\n"
            << " hx: " << VectorDebugString(hx);

    // construct arrowhead hessian matrix
    Eigen::VectorXd d(n+1);
    d.head(n) = Eigen::VectorXd::Constant(n, 1.) + lam * f_->hessf(x);
    d(n) = 1;
    Eigen::VectorXd z(n+1);
    z.head(n) = g.head(n);
    z(n) = -1.;
    double alpha = 0;
    Eigen::VectorXd step = SolveArrowheadSystem(d, z, alpha, g);

    // line search
    double beta = 0.001;
    double gamma = 0.5;
    double theta = 1;
    double x_res = g.norm();
    while(theta > eps) {
      Eigen::VectorXd nx = x - theta*step.head(n);
      double nt = t - theta*step(n);
      double nlam = lam - theta*step(n+1);
      if(nlam < 0)
              nlam = 0;
      double nx_res = residual(nx, nt, nlam, v, s).norm();
      if(nx_res <= (1-beta*theta)*x_res) {
        x = nx;
        t = nt;
        lam = nlam;
        x_res = nx_res;
        break;
      }
      theta *= gamma;
      if(theta < eps)
              VLOG(1) << "Line search reach max iter, x_res=" << x_res << "\n";
    }

    if(x_res < eps) {
      VLOG(1) << "Using " << iter+1 << " Newton iteration.\n";
      break;
    } else if(iter == MAX_ITER-1) {
      VLOG(1) << "Newton Method won't converge for epigraph.\n"
        << "sv = " << sv << '\n';
    }
  }
  Eigen::VectorXd tx(n+1);
  tx(0) = t;
  tx.tail(n) = x;
  return tx;
}

// solve Ax=b, where A = [diag(d), z; z', alpha], and d_i>0 forall i
Eigen::VectorXd NewtonEpigraph::SolveArrowheadSystem
  (const Eigen::VectorXd &d, const Eigen::VectorXd &z, double alpha,
    const Eigen::VectorXd &b) {
    int n = z.rows();
    Eigen::VectorXd u = z.array() / d.array();
    double rho = alpha - u.dot(z);
    Eigen::VectorXd y(n+1);
    y.head(n) = u;
    y(n) = -1;
    Eigen::VectorXd dinv_b(n+1);
    dinv_b.head(n) = b.head(n).array() / d.array();
    dinv_b(n) = 0;

    return dinv_b + 1/rho * y.dot(b) * y;
}
