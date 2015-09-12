"""Split problems so that objective terms are separable."""

# Mutators

def rename_var(old_id, new_id, expr):
    if (expr.expression_type == Expression.VARIABLE and
        expr.variable.variable_id == old_id):
        expr.variable.variable_id = new_id

    for arg in expr.arg:
        rename_var(old_id, new_id, arg)

def rename_function_var(old_var_id, new_var_id, function):
    for arg in function.arg:
        rename_var(old_var_id, new_var_id, arg)
    if function.HasField("affine"):
        rename_var(old_var_id, new_var_id, function.affine)
    if function.HasField("regularization"):
        rename_var(old_var_id, new_var_id, function.regularization)

def transform(problem):
    """If two prox functions refer to the same variable, add a copy."""

    fs = state.prox_prob.prox_function
    var_function_map = defaultdict(list)
    orig_vars = {}

    for f in state.prox_prob.prox_function:
        f_vars = function_vars(f)
        orig_vars.update(f_vars)
        for var_id in f_vars:
            var_function_map[var_id].append(f)

    for var_id, fs in var_function_map.iteritems():
        # Enumerate the functions backwards so that the last prox function keeps
        # the original variable id.
        for i, f in enumerate(fs[-2::-1]):
            new_var_id = "%s:%d" % (var_id, i)
            old_var = orig_vars[var_id]
            new_var = Expression()
            new_var.CopyFrom(old_var)
            new_var.variable.variable_id = new_var_id

            state.add_equality_constraint(add(old_var, negate(new_var)))
            rename_function_var(var_id, new_var_id, f)

    return problem
