
from nose.tools import assert_items_equal, assert_equal

from epsilon import cvxpy_expr
from epsilon.compiler import compiler
from epsilon.compiler import validate
from epsilon.problems import basis_pursuit
from epsilon.problems import least_abs_dev
from epsilon.problems import tv_smooth
from epsilon.expression_pb2 import Expression

def prox_op(expr):
    if expr.expression_type == Expression.ADD:
        return prox_op(expr.arg[0])
    if expr.expression_type == Expression.MULTIPLY:
        return prox_op(expr.arg[1])
    return expr.proximal_operator.name

def prox_ops(problem):
    validate.check_sum_of_prox(problem)
    for f in problem.objective.arg:
        yield prox_op(f)

def test_basis_pursuit():
    problem = compiler.compile(cvxpy_expr.convert_problem(
        basis_pursuit.create(m=10, n=30))[0])
    assert_items_equal(prox_ops(problem), ["NormL1Prox", "LinearEqualityProx"])
    assert_equal(1, len(problem.constraint))

def test_least_abs_deviations():
    problem = compiler.compile(cvxpy_expr.convert_problem(
        least_abs_dev.create(m=10, n=5))[0])
    assert_items_equal(prox_ops(problem), ["NormL1Prox"])
    assert_equal(1, len(problem.constraint))

def test_tv_smooth():
    problem = compiler.compile(cvxpy_expr.convert_problem(
        tv_smooth.create(n=10, lam=1))[0])
    assert_items_equal(
        prox_ops(problem), 3*["LeastSquaresProx"] + ["NormL1L2Prox"])
    assert_equal(1, len(problem.constraint))