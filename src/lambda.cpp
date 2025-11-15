#include "../include/lambda.hpp"

#define VERBOSE_LOGS 1
#if VERBOSE_LOGS

#include <iostream>

#define LOG_EXPR(a_expr)                                                       \
    a_expr->print(std::cout);                                                  \
    std::cout << std::endl;

#else

// no-op
#define LOG_EXPR(a_expr)

#endif

namespace lambda
{

// EQUALS METHODS

bool var::equals(const std::unique_ptr<expr>& a_other) const
{
    const var* l_casted = dynamic_cast<const var*>(a_other.get());

    if(!l_casted)
        return false;

    return m_index == l_casted->m_index;
}

bool func::equals(const std::unique_ptr<expr>& a_other) const
{
    const func* l_casted = dynamic_cast<const func*>(a_other.get());

    if(!l_casted)
        return false;

    return m_body->equals(l_casted->m_body);
}

bool app::equals(const std::unique_ptr<expr>& a_other) const
{
    const app* l_casted = dynamic_cast<const app*>(a_other.get());

    if(!l_casted)
        return false;

    return lhs()->equals(l_casted->lhs()) && rhs()->equals(l_casted->rhs());
}

// PRINT METHODS

void var::print(std::ostream& a_ostream) const
{
    a_ostream << m_index;
}

void func::print(std::ostream& a_ostream) const
{
    a_ostream << "λ.(";
    m_body->print(a_ostream);
    a_ostream << ")";
}

void app::print(std::ostream& a_ostream) const
{
    a_ostream << "(";
    m_lhs->print(a_ostream);
    a_ostream << " ";
    m_rhs->print(a_ostream);
    a_ostream << ")";
}

// LIFT METHODS

std::unique_ptr<expr> var::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    if(m_index < a_cutoff)
        return v(m_index);

    return v(m_index + a_lift_amount);
}

std::unique_ptr<expr> func::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    // we don't increment here, since the goal is to lift the WHOLE function
    // (all locals inside) by the same amount (provided they are >= cutoff).
    return f(m_body->lift(a_lift_amount, a_cutoff));
}

std::unique_ptr<expr> app::lift(size_t a_lift_amount, size_t a_cutoff) const
{
    return a(m_lhs->lift(a_lift_amount, a_cutoff),
             m_rhs->lift(a_lift_amount, a_cutoff));
}

// SUBSTITUTE METHODS

std::unique_ptr<expr> var::substitute(size_t a_lift_amount, size_t a_var_index,
                                      const std::unique_ptr<expr>& a_arg) const
{
    if(m_index > a_var_index)
        // this var is defined inside the redex, so it is
        //     now 1 level shallower.
        return v(m_index - 1);

    if(m_index < a_var_index)
        // leave the var alone, it was declared outside the redex
        return clone();

    // this var is the one we are substituting, so we must substitute it
    return a_arg->lift(a_lift_amount, a_var_index);
}

std::unique_ptr<expr> func::substitute(size_t a_lift_amount, size_t a_var_index,
                                       const std::unique_ptr<expr>& a_arg) const
{
    // increment the binder depth
    return f(m_body->substitute(a_lift_amount + 1, a_var_index, a_arg));
}

std::unique_ptr<expr> app::substitute(size_t a_lift_amount, size_t a_var_index,
                                      const std::unique_ptr<expr>& a_arg) const
{
    // just substitute the function and argument
    return a(m_lhs->substitute(a_lift_amount, a_var_index, a_arg),
             m_rhs->substitute(a_lift_amount, a_var_index, a_arg));
}

// REDUCE METHODS

std::unique_ptr<expr> var::reduce_one_step(size_t a_depth) const
{
    // variables cannot reduce
    return nullptr;
}

std::unique_ptr<expr> func::reduce_one_step(size_t a_depth) const
{
    // check if body can reduce
    auto l_reduced_body = m_body->reduce_one_step(a_depth + 1);

    if(!l_reduced_body)
        // body cannot reduce, so this function cannot reduce
        return nullptr;

    // return the new function with the reduced body
    return f(std::move(l_reduced_body));
}

std::unique_ptr<expr> app::reduce_one_step(size_t a_depth) const
{
    // see if this app is a beta-redex
    const func* l_lhs_func = dynamic_cast<const func*>(lhs().get());

    // if the lhs is a function, beta-contract the body
    if(l_lhs_func)
        return l_lhs_func->body()->substitute(0, a_depth, rhs());

    // try to reduce lhs
    auto l_reduced_lhs = lhs()->reduce_one_step(a_depth);

    // if lhs can reduce, leave rhs alone and return
    if(l_reduced_lhs)
        return a(std::move(l_reduced_lhs), rhs()->clone());

    // try to reduce rhs
    auto l_reduced_rhs = rhs()->reduce_one_step(a_depth);

    // if rhs can reduce, leave lhs alone and return
    if(l_reduced_rhs)
        return a(lhs()->clone(), std::move(l_reduced_rhs));

    // otherwise, return nullptr
    return nullptr;
}

// EXPR CLONE METHOD
std::unique_ptr<expr> expr::clone() const
{
    return lift(0, 0);
}

// EXPR NORMALIZE METHOD
expr::normalize_result expr::normalize(size_t a_step_limit,
                                       size_t a_size_limit) const
{
    // initialize the result
    normalize_result l_result{
        .m_step_excess = false,
        .m_size_excess = false,
        .m_step_count = 0,
        .m_size_peak = std::numeric_limits<size_t>::min(),
        .m_expr = clone(),
    };

    // log the original expression
    LOG_EXPR(l_result.m_expr);

    // as long as we are within the limits, keep reducing
    while(auto l_reduced = l_result.m_expr->reduce_one_step(0))
    {
        // check if the step limit has been exceeded
        if(l_result.m_step_count == a_step_limit)
        {
            l_result.m_step_excess = true;
            break;
        }

        // check if the size limit has been exceeded
        if(l_reduced->size() > a_size_limit)
        {
            l_result.m_size_excess = true;
            break;
        }

        // update the step count
        ++l_result.m_step_count;

        // update the peak size
        l_result.m_size_peak =
            std::max(l_result.m_size_peak, l_reduced->size());

        // update the result to the reduced expression
        l_result.m_expr = std::move(l_reduced);

        // log the reduction
        LOG_EXPR(l_result.m_expr);
    }

    return l_result;
}

// GETTER METHODS
size_t expr::size() const
{
    return m_size;
}

size_t var::index() const
{
    return m_index;
}

const std::unique_ptr<expr>& func::body() const
{
    return m_body;
}

const std::unique_ptr<expr>& app::lhs() const
{
    return m_lhs;
}

const std::unique_ptr<expr>& app::rhs() const
{
    return m_rhs;
}

// CONSTRUCTORS
expr::expr(size_t a_size) : m_size(a_size)
{
}

var::var(size_t a_index) : expr(1), m_index(a_index)
{
}

func::func(std::unique_ptr<expr>&& a_body)
    : expr(1 + a_body->size()), m_body(std::move(a_body))
{
}

app::app(std::unique_ptr<expr>&& a_lhs, std::unique_ptr<expr>&& a_rhs)
    : expr(1 + a_lhs->size() + a_rhs->size()), m_lhs(std::move(a_lhs)),
      m_rhs(std::move(a_rhs))
{
}

// FACTORY FUNCTIONS

std::unique_ptr<expr> v(size_t a_index)
{
    return std::unique_ptr<expr>(new var(a_index));
}

std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body)
{
    return std::unique_ptr<expr>(new func(std::move(a_body)));
}

std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_lhs,
                        std::unique_ptr<expr>&& a_rhs)
{
    return std::unique_ptr<expr>(new app(std::move(a_lhs), std::move(a_rhs)));
}

} // namespace lambda

#ifdef UNIT_TEST

#include "../testing/test_utils.hpp"
#include <iostream>
#include <list>

using namespace lambda;

void test_var_constructor()
{
    // index 0
    {
        auto l_var = v(0);
        const var* l_var_casted = dynamic_cast<var*>(l_var.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->index() == 0);
    }

    // index 1
    {
        auto l_var = v(1);
        const var* l_var_casted = dynamic_cast<var*>(l_var.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->index() == 1);
    }
}

void test_func_constructor()
{
    // local body
    {
        auto l_func = f(v(0));
        // get body
        const func* l_func_casted = dynamic_cast<func*>(l_func.get());
        assert(l_func_casted != nullptr);
        const auto& l_body = l_func_casted->body();
        // check if the body is a local
        const var* l_var = dynamic_cast<var*>(l_body.get());
        assert(l_var != nullptr);
        // check if the index is correct
        assert(l_var->index() == 0);
    }
}

void test_app_constructor()
{
    // local application
    {
        auto l_app = a(v(0), v(1));
        // get the lhs
        const app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);
        const auto& l_lhs = l_app_casted->lhs();
        // get the rhs
        const auto& l_rhs = l_app_casted->rhs();

        // make sure they both are locals
        const var* l_lhs_var = dynamic_cast<var*>(l_lhs.get());
        const var* l_rhs_var = dynamic_cast<var*>(l_rhs.get());
        assert(l_lhs_var != nullptr);
        assert(l_rhs_var != nullptr);

        // make sure the indices are correct
        assert(l_lhs_var->index() == 0);
        assert(l_rhs_var->index() == 1);
    }
}

void test_var_equals()
{
    // index 0, equals index 0
    {
        auto l_var = v(0);
        auto l_local_other = v(0);
        assert(l_var->equals(l_local_other->clone()));
    }

    // index 0, equals index 1
    {
        auto l_var = v(0);
        auto l_local_other = v(1);
        assert(!l_var->equals(l_local_other->clone()));
    }

    // index 1, equals index 1
    {
        auto l_var = v(1);
        auto l_local_other = v(1);
        assert(l_var->equals(l_local_other->clone()));
    }

    // var equals global
    // {
    //     auto l_var = l(0);
    //     auto l_global = g(0);
    //     assert(!l_var->equals(l_global->clone()));
    // }

    // local equals func
    {
        auto l_var = v(0);
        auto l_func = f(v(0));
        assert(!l_var->equals(l_func->clone()));
    }

    // local equals app
    {
        auto l_var = v(0);
        auto l_app = a(v(0), v(0));
        assert(!l_var->equals(l_app->clone()));
    }
}

void test_func_equals()
{
    // local body, equals local body
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_func_other = f(l_var->clone());
        assert(l_func->equals(l_func_other->clone()));
    }

    // func equals local
    {
        auto l_func = f(v(0));
        auto l_var = v(0);
        assert(!l_func->equals(l_var->clone()));
    }

    // func with different bodies
    {
        auto l_var = v(0);
        auto l_local_other = v(1);
        auto l_func = f(l_var->clone());
        auto l_func_other = f(l_local_other->clone());
        assert(!l_func->equals(l_func_other->clone()));
    }
}

void test_app_equals()
{
    // local lhs, local rhs, equals local lhs, local rhs
    {
        auto l_lhs_var = v(0);
        auto l_rhs_var = v(0);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_app_other = a(l_lhs_var->clone(), l_rhs_var->clone());
        assert(l_app->equals(l_app_other->clone()));
    }

    // different applications
    {
        auto l_lhs_lhs = v(1);
        auto l_lhs_rhs = v(0);
        auto l_rhs_lhs = v(0);
        auto l_rhs_rhs = v(0);
        auto l_lhs = a(l_lhs_lhs->clone(), l_lhs_rhs->clone());
        auto l_rhs = a(l_rhs_lhs->clone(), l_rhs_rhs->clone());
        assert(!l_lhs->equals(l_rhs));
    }
}

void test_var_lift()
{
    // index 0, lift 1 level
    {
        auto l_var = v(0);
        auto l_lifted = l_var->lift(1, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 1);
    }

    // index 1, lift 1 level
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(1, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 2);
    }

    // index 1, lift 0 levels
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(0, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 1);
    }

    // index 0, lift 1 level, cutoff 1
    {
        auto l_var = v(0);
        auto l_lifted = l_var->lift(1, 1);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 0);
    }

    // index 1, lift 2 levels, cutoff 1
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(2, 1);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 3);
    }

    // index 1, lift 2 levels, cutoff 2
    {
        auto l_var = v(1);
        auto l_lifted = l_var->lift(2, 2);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 1);
    }

    // Edge case: index equals cutoff, should be lifted (>= cutoff)
    // index 3, lift 5 levels, cutoff 3
    {
        auto l_var = v(3);
        auto l_lifted = l_var->lift(5, 3);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 8); // 3 + 5
    }

    // Edge case: index just below cutoff
    // index 4, lift 3 levels, cutoff 5
    {
        auto l_var = v(4);
        auto l_lifted = l_var->lift(3, 5);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 4); // not lifted
    }

    // Higher cutoff value test
    // index 7, lift 10 levels, cutoff 3
    {
        auto l_var = v(7);
        auto l_lifted = l_var->lift(10, 3);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 17); // 7 + 10
    }

    // Multiple different cutoffs - lower index below cutoff
    // index 2, lift 4 levels, cutoff 10
    {
        auto l_var = v(2);
        auto l_lifted = l_var->lift(4, 10);
        const var* l_lifted_var = dynamic_cast<var*>(l_lifted.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 2); // not lifted
    }
}

void test_func_lift()
{
    // local body, lift 1 level
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(1, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->body().get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 1);
    }

    // local body, lift 2 levels
    {
        auto l_var = v(1);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(2, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->body().get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 3);
    }

    // local body, lift 1 level, cutoff 1
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(1, 1);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->body().get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 0);
    }

    // local body, lift 2 levels, cutoff 2
    {
        auto l_var = v(2);
        auto l_func = f(l_var->clone());
        auto l_lifted = l_func->lift(2, 2);
        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->body().get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->index() == 4);
    }

    // func with app body, mixed locals with various cutoffs
    // Body: (2 5 8), lift 3, cutoff 5
    {
        auto l_body = a(a(v(2)->clone(), v(5)->clone()), v(8)->clone());
        auto l_func = f(l_body->clone());
        auto l_lifted = l_func->lift(3, 5);

        const func* l_lifted_func = dynamic_cast<func*>(l_lifted.get());
        assert(l_lifted_func != nullptr);

        // Expected: (2 8 11)
        // l(2) < 5, not lifted
        // l(5) >= 5, lifted to 8
        // l(8) >= 5, lifted to 11
        auto l_expected = f(a(a(v(2)->clone(), v(8)->clone()), v(11)->clone()));
        assert(l_lifted->equals(l_expected));
    }

    // func with nested func, testing cutoff propagation
    // f(f((1 3 6))), lift 2, cutoff 3
    {
        auto l_inner_body = a(a(v(1)->clone(), v(3)->clone()), v(6)->clone());
        auto l_body = f(l_inner_body->clone());
        auto l_func = f(l_body->clone());
        auto l_lifted = l_func->lift(2, 3);

        // Expected: f(f((1 5 8)))
        // l(1) < 3, not lifted
        // l(3) >= 3, lifted to 5
        // l(6) >= 3, lifted to 8
        auto l_expected =
            f(f(a(a(v(1)->clone(), v(5)->clone()), v(8)->clone())));
        assert(l_lifted->equals(l_expected));
    }

    // func with higher cutoff than any local
    // f(l(2)), lift 5, cutoff 10
    {
        auto l_func = f(v(2)->clone());
        auto l_lifted = l_func->lift(5, 10);

        // l(2) < 10, not lifted
        auto l_expected = f(v(2)->clone());
        assert(l_lifted->equals(l_expected));
    }
}

void test_app_lift()
{
    // local lhs, local rhs, lift 1 level
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(1, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->lhs();
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->index() == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->rhs();
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->index() == 3);
    }

    // local lhs, local rhs, lift 2 levels
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(2, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->lhs();
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->index() == 3);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->rhs();
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->index() == 4);
    }

    // local lhs, local rhs, lift 1 level, cutoff 1
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(1, 1);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->lhs();
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->index() == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->rhs();
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->index() == 3);
    }

    // local lhs, local rhs, lift 2 levels, cutoff 2
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        auto l_lifted = l_app->lift(2, 2);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_lifted.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->lhs();
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->index() == 1);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->rhs();
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->index() == 4);
    }

    // app with mixed locals, lift 4, cutoff 3
    // (1 2 3 4 5) - mix below, at, and above cutoff
    {
        auto l_app = a(
            a(a(a(v(1)->clone(), v(2)->clone()), v(3)->clone()), v(4)->clone()),
            v(5)->clone());
        auto l_lifted = l_app->lift(4, 3);

        // Expected: (1 2 7 8 9)
        // l(1), l(2) < 3, not lifted
        // l(3), l(4), l(5) >= 3, lifted by 4
        auto l_expected = a(
            a(a(a(v(1)->clone(), v(2)->clone()), v(7)->clone()), v(8)->clone()),
            v(9)->clone());
        assert(l_lifted->equals(l_expected));
    }

    // app with nested funcs, cutoff applies throughout
    // (f(l(2)) f(l(4))), lift 3, cutoff 3
    {
        auto l_lhs = f(v(2)->clone());
        auto l_rhs = f(v(4)->clone());
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_lifted = l_app->lift(3, 3);

        // Expected: (f(l(2)) f(l(7)))
        // l(2) < 3, not lifted
        // l(4) >= 3, lifted to 7
        auto l_expected = a(f(v(2)->clone()), f(v(7)->clone()));
        assert(l_lifted->equals(l_expected));
    }

    // app with complex nested structure, various cutoffs
    // ((1 6) (f(3) f(8))), lift 2, cutoff 5
    {
        auto l_func_left = a(v(1)->clone(), v(6)->clone());
        auto l_func_right = a(f(v(3)->clone()), f(v(8)->clone()));
        auto l_app = a(l_func_left->clone(), l_func_right->clone());
        auto l_lifted = l_app->lift(2, 5);

        // Expected: ((1 8) (f(3) f(10)))
        // l(1) < 5, not lifted
        // l(6) >= 5, lifted to 8
        // l(3) < 5, not lifted
        // l(8) >= 5, lifted to 10
        auto l_expected = a(a(v(1)->clone(), v(8)->clone()),
                            a(f(v(3)->clone()), f(v(10)->clone())));
        assert(l_lifted->equals(l_expected));
    }

    // app with high cutoff, nothing gets lifted
    // (3 4 5), lift 10, cutoff 20
    {
        auto l_app = a(a(v(3)->clone(), v(4)->clone()), v(5)->clone());
        auto l_lifted = l_app->lift(10, 20);

        // All < 20, none lifted
        auto l_expected = a(a(v(3)->clone(), v(4)->clone()), v(5)->clone());
        assert(l_lifted->equals(l_expected));
    }

    // app with cutoff 0, everything gets lifted
    // (0 1 2), lift 5, cutoff 0
    {
        auto l_app = a(a(v(0)->clone(), v(1)->clone()), v(2)->clone());
        auto l_lifted = l_app->lift(5, 0);

        // All >= 0, all lifted by 5
        auto l_expected = a(a(v(5)->clone(), v(6)->clone()), v(7)->clone());
        assert(l_lifted->equals(l_expected));
    }
}

void test_var_substitute()
{
    // index 0, occurrance depth 0, substitute with a local
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(0, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        assert(l_substituted_var->index() == 1);
    }

    // index 0, occurrance depth 10, substitute with a local
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(10, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        // it would be 1 if occurrance depth == 0, but since 10,
        //     l_substitute had to be lifted.
        assert(l_substituted_var->index() == 11);
    }

    // index 2, occurrance depth 0, substitute with a local
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->index() == 1);
    }

    // index 1, occurrance depth 0, substitute with a local
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->index() == 0);
    }

    // index 2, occurrance depth 10, substitute with a local
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->index() == 1);
    }

    // index 1, occurrance depth 10, substitute with a local
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 0, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->index() == 0);
    }

    // index 0, occurrance depth 0, substitute with a local, a_var_index 1
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(0, 1, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        assert(l_substituted_var->index() == 0);
    }

    // index 0, occurrance depth 10, substitute with a local, a_var_index 1
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        auto l_substituted = l_var->substitute(10, 1, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);
        // it would be 10 if a_var_index == 0, but since 1,
        //     not only were there no occurrances, but the
        //     var(0) was bound before cutoff (a_var_index == 1)
        //     so no lifting occurred.
        assert(l_substituted_var->index() == 0);
    }

    // index 2, occurrance depth 0, substitute with a local, a_var_index 2
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution trivially finds var(2) and substitutes with var(3),
        // without lifting as the redex body has no binders.
        assert(l_substituted_var->index() == 3);
    }

    // index 1, occurrance depth 0, substitute with a local, a_var_index 2
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(0, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution does not find var(2),
        // but var(1) is left alone since it was bound before cutoff
        // (a_var_index == 2)
        assert(l_substituted_var->index() == 1);
    }

    // index 2, occurrance depth 10, substitute with a local, a_var_index 2
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // this substitution finds var(2) and substitutes with var(3),
        // and lifts by 10 levels since the redex body has 10 binders.
        assert(l_substituted_var->index() == 13);
    }

    // index 1, occurrance depth 10, substitute with a local, a_var_index 2
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        auto l_substituted = l_var->substitute(10, 2, l_sub->clone());

        const var* l_substituted_var = dynamic_cast<var*>(l_substituted.get());
        assert(l_substituted_var != nullptr);

        // no var(2) was found, so var(1) is left alone since it was bound
        // before cutoff (a_var_index == 2)
        assert(l_substituted_var->index() == 1);
    }
}

void test_func_substitute()
{
    // single composition lambda, outer depth 0, occurrance found
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(v(0)->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 0, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->body().get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the index of the substitute is lifted by 1 (one binder)
        assert(l_subbed_var->index() == 12);
    }
    // doublle composition lambda, outer depth 0, occurrance found
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(f(v(0)->clone())->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 0, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->body().get());

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func_2->body().get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the index of the substitute is lifted by 2 (two binders)
        assert(l_subbed_var->index() == 13);
    }

    // single composition lambda, outer depth 0, occurrance NOT found,
    // a_var_index 1
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(v(0)->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 1, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->body().get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the local is unchanged as it was not replaced and it was bound before
        // cutoff (a_var_index == 1) so no lifting occurred.
        assert(l_subbed_var->index() == 0);
    }

    // doublle composition lambda, outer depth 0, occurrance not found,
    // a_var_index 1
    {
        // it should be noted:
        // when saying here that l_func has a body local of index 0,
        // that the local does NOT reference the binder introduced by
        // l_func. This is because, when subbing, it is implied that
        // there USED to be an even more outer binder (which 0 would
        // have been bound to, hence the substitution DOES take place)

        auto l_func = f(f(v(0)->clone())->clone());
        auto l_var = v(11);

        const auto l_subbed = l_func->substitute(0, 1, l_var->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->body().get());

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func_2->body().get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the local is unchanged as it was not replaced and it was bound before
        // cutoff (a_var_index == 1) so no lifting occurred.
        assert(l_subbed_var->index() == 0);
    }

    // func with occurrence in its body, a_lift_amount > 0
    {
        // Create a function whose body contains a local with index 2 (is the
        // occurrence)
        auto l_func = f(v(2)->clone());
        auto l_sub = v(7);

        // substitute at depth = 6 (5 + 1), var_index=2, so a_lift_amount=6
        const auto l_subbed = l_func->substitute(5, 2, l_sub->clone());

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());
        assert(l_subbed_func != nullptr);

        // get body, should be a local with index = 7 + 6 = 13
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->body().get());
        assert(l_subbed_var != nullptr);
        assert(l_subbed_var->index() == 13);
    }

    // func with app body containing mixed locals, var_index=3
    // Tests that locals < var_index are left alone
    {
        // Body: (0 1 2 3 4) - mix of locals below, at, and above var_index=3
        auto l_body = a(
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone()),
            v(4)->clone());
        auto l_func = f(l_body->clone());
        auto l_sub = v(99);

        // substitute var_index=3 with l(99) at depth 0
        const auto l_subbed = l_func->substitute(0, 3, l_sub->clone());

        // Expected: (0 1 2 100 3)
        // - l(0), l(1), l(2) stay unchanged (< 3)
        // - l(3) gets replaced with l(99) lifted by 1 (func binder) = l(100)
        // - l(4) decrements to l(3) (> 3)
        auto l_expected =
            f(a(a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()),
                  v(100)->clone()),
                v(3)->clone()));

        assert(l_subbed->equals(l_expected));
    }

    // func with nested func, testing locals < var_index preservation
    {
        // Body: λ.(0 2 3) with outer var_index=2
        // Inner l(0) is bound by inner lambda
        // Outer l(2) and l(3) are from outer scope
        auto l_inner_body = a(a(v(0)->clone(), v(2)->clone()), v(3)->clone());
        auto l_body = f(l_inner_body->clone());
        auto l_func = f(l_body->clone());
        auto l_sub = v(88);

        // substitute var_index=2 at depth=0
        const auto l_subbed = l_func->substitute(0, 2, l_sub->clone());

        // Expected: λ.λ.(0 89 2)
        // - Inner l(0) unchanged (bound by inner lambda)
        // - l(2) at depth 2 (due to 2 binders) should match var 2
        //   and be replaced with l(88) lifted by 2 = l(90)
        // - l(3) decrements to l(2)
        auto l_expected =
            f(f(a(a(v(0)->clone(), v(90)->clone()), v(2)->clone())));

        assert(l_subbed->equals(l_expected));
    }
}

void test_app_substitute()
{
    // app of locals, both are occurrances
    {
        auto l_lhs = v(0);
        auto l_rhs = v(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->lhs().get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->rhs().get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->index() == 11);
        assert(l_subbed_rhs->index() == 11);
    }

    // app of locals, lhs is an occurrance
    {
        auto l_lhs = v(0);
        auto l_rhs = v(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->lhs().get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->rhs().get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->index() == 11);
        assert(l_subbed_rhs->index() == 0);
    }

    // app of locals, rhs is an occurrance
    {
        auto l_lhs = v(1);
        auto l_rhs = v(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->lhs().get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->rhs().get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->index() == 0);
        assert(l_subbed_rhs->index() == 11);
    }

    // app of locals, neither are occurrances
    {
        auto l_lhs = v(1);
        auto l_rhs = v(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->lhs().get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->rhs().get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->index() == 0);
        assert(l_subbed_rhs->index() == 0);
    }

    // app of funcs, both with occurrances
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = f(v(0)->clone());
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const func* l_subbed_lhs =
            dynamic_cast<func*>(l_subbed_app->lhs().get());

        // make sure lhs is a func
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const func* l_subbed_rhs =
            dynamic_cast<func*>(l_subbed_app->rhs().get());

        // make sure rhs is a func
        assert(l_subbed_rhs != nullptr);

        const var* l_lhs_var = dynamic_cast<var*>(l_subbed_lhs->body().get());

        // make sure body of lhs is a local
        assert(l_lhs_var != nullptr);

        const var* l_rhs_var = dynamic_cast<var*>(l_subbed_rhs->body().get());

        // make sure body of rhs is a local
        assert(l_rhs_var != nullptr);

        // make sure they have correct indices (lifted by 1 due to binders)
        assert(l_lhs_var->index() == 12);
        assert(l_rhs_var->index() == 12);
    }

    ////////////////////////////////////
    // Testing substitute with various binder depths
    ////////////////////////////////////

    // Test 1: substitute at depth 0, var_index 0 - basic substitution
    // (0 0) with var 0 -> l(5) should give (5 5)
    {
        auto l_app = a(v(0)->clone(), v(0)->clone());
        auto l_sub = v(5);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(v(5)->clone(), v(5)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 2: substitute at depth 0, var_index 1 - substituting higher var
    // (1 2) with var 1 -> l(7) should give (7 1)
    {
        auto l_app = a(v(1)->clone(), v(2)->clone());
        auto l_sub = v(7);
        const auto l_subbed = l_app->substitute(0, 1, l_sub->clone());
        auto l_expected = a(v(7)->clone(), v(1)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 3: substitute at depth 1, var_index 0 - inside a binder context
    // (λ.0 λ.1) with var 0 at depth 1 -> l(3) should give (λ.5 λ.0)
    {
        auto l_app = a(f(v(0)->clone()), f(v(1)->clone()));
        auto l_sub = v(3);
        const auto l_subbed = l_app->substitute(1, 0, l_sub->clone());
        auto l_expected = a(f(v(5)->clone()), f(v(0)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 5: substitute with complex expression (app as substitution)
    // (0 1) with var 0 -> (2 3) should give ((2 3) 0)
    {
        auto l_app = a(v(0)->clone(), v(1)->clone());
        auto l_sub = a(v(2)->clone(), v(3)->clone());
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(a(v(2)->clone(), v(3)->clone()), v(0)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 6: substitute with func as substitution
    // (0 0) with var 0 -> λ.5 should give (λ.5 λ.5)
    {
        auto l_app = a(v(0)->clone(), v(0)->clone());
        auto l_sub = f(v(5)->clone());
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(f(v(5)->clone()), f(v(5)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 7: substitute at depth 2, var_index 1 - deeply nested
    // (λ.λ.1 λ.λ.2) with var 1 at depth 2 -> l(10) should give (λ.λ.14 λ.λ.1)
    {
        auto l_app = a(f(f(v(1)->clone())), f(f(v(2)->clone())));
        auto l_sub = v(10);
        const auto l_subbed = l_app->substitute(2, 1, l_sub->clone());
        auto l_expected = a(f(f(v(14)->clone())), f(f(v(1)->clone())));
        assert(l_subbed->equals(l_expected));
    }

    // Test 8: no matching variable - all vars higher than target
    // (2 3) with var 0 -> l(99) should give (1 2)
    {
        auto l_app = a(v(2)->clone(), v(3)->clone());
        auto l_sub = v(99);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected = a(v(1)->clone(), v(2)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 9: no matching variable - all vars lower than target
    // (0 1) with var 5 -> l(99) should give (0 1)
    {
        auto l_app = a(v(0)->clone(), v(1)->clone());
        auto l_sub = v(99);
        const auto l_subbed = l_app->substitute(0, 5, l_sub->clone());
        auto l_expected = a(v(0)->clone(), v(1)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 10: nested app with mixed locals
    // ((0 1) (2 0)) with var 0 -> l(8) should give ((8 0) (1 8))
    {
        auto l_inner1 = a(v(0)->clone(), v(1)->clone());
        auto l_inner2 = a(v(2)->clone(), v(0)->clone());
        auto l_app = a(l_inner1->clone(), l_inner2->clone());
        auto l_sub = v(8);
        const auto l_subbed = l_app->substitute(0, 0, l_sub->clone());
        auto l_expected =
            a(a(v(8)->clone(), v(0)->clone()), a(v(1)->clone(), v(8)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 11: substitute higher var with lower vars present
    // (0 2) with var 2 -> l(9) should give (0 9)
    // l(0) stays unchanged (< 2), l(2) gets replaced
    {
        auto l_app = a(v(0)->clone(), v(2)->clone());
        auto l_sub = v(9);
        const auto l_subbed = l_app->substitute(0, 2, l_sub->clone());
        auto l_expected = a(v(0)->clone(), v(9)->clone());
        assert(l_subbed->equals(l_expected));
    }

    // Test 12: substitute with var_index=4, multiple locals below and above
    // ((0 1 2 3) (4 5 6)) with var 4 -> l(77) should give ((0 1 2 3) (77 4 5))
    {
        auto l_func_app =
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone());
        auto l_arg_app = a(a(v(4)->clone(), v(5)->clone()), v(6)->clone());
        auto l_app = a(l_func_app->clone(), l_arg_app->clone());
        auto l_sub = v(77);
        const auto l_subbed = l_app->substitute(0, 4, l_sub->clone());

        // Expected: ((0 1 2 3) (77 4 5))
        // l(0), l(1), l(2), l(3) unchanged (< 4)
        // l(4) replaced with l(77)
        // l(5), l(6) decremented to l(4), l(5)
        auto l_expected = a(
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone()),
            a(a(v(77)->clone(), v(4)->clone()), v(5)->clone()));
        assert(l_subbed->equals(l_expected));
    }

    // Test 13: nested app with funcs, var_index=2
    // (λ.(0 1 2) λ.(1 2 3)) with var 2 -> l(55)
    {
        auto l_lhs_body = a(a(v(0)->clone(), v(1)->clone()), v(2)->clone());
        auto l_rhs_body = a(a(v(1)->clone(), v(2)->clone()), v(3)->clone());
        auto l_app = a(f(l_lhs_body->clone()), f(l_rhs_body->clone()));
        auto l_sub = v(55);
        const auto l_subbed = l_app->substitute(0, 2, l_sub->clone());

        // In lhs func: depth becomes 1, so var 2 at that depth
        // l(0), l(1) unchanged (< 2)
        // l(2) matches var 2, replaced with l(55) lifted by 1 = l(56)
        // In rhs func: same logic
        // l(1) unchanged (< 2)
        // l(2) matches var 2, replaced with l(55) lifted by 1 = l(56)
        // l(3) decrements to l(2)
        auto l_expected =
            a(f(a(a(v(0)->clone(), v(1)->clone()), v(56)->clone())),
              f(a(a(v(1)->clone(), v(56)->clone()), v(2)->clone())));
        assert(l_subbed->equals(l_expected));
    }
}

void test_var_normalize()
{
    // local with var 0
    {
        auto l_expr = v(0);
        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());

        // cast the pointer
        const var* l_var = dynamic_cast<var*>(l_result.m_expr.get());
        assert(l_var != nullptr);

        // make sure it has the same index
        assert(l_var->index() == 0);
    }

    // local with var 1
    {
        auto l_expr = v(1);
        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());

        // cast the pointer
        const var* l_var = dynamic_cast<var*>(l_result.m_expr.get());
        assert(l_var != nullptr);

        // make sure it has the same index
        assert(l_var->index() == 1);
    }

    // Test reduction count with no reductions (var cannot reduce)
    {
        auto l_expr = v(5);
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test reduction limit on var (no effect since vars don't reduce)
    {
        auto l_expr = v(7);
        auto l_result = l_expr->normalize(0);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(7)));
    }

    // Test size peak tracking on var (size should stay at min since no
    // reductions)
    {
        auto l_expr = v(10);
        auto l_result = l_expr->normalize(std::numeric_limits<size_t>::max(),
                                          std::numeric_limits<size_t>::max());
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(10)));
    }

    // Test size peak with default parameters (should work without tracking)
    {
        auto l_expr = v(5);
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test size limit on var (no effect since size stays at 1 and never
    // reduces)
    {
        auto l_expr = v(3);
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 1);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(3)));
    }

    // Test size limit smaller than var size (no reductions happen so no excess)
    {
        auto l_expr = v(5);
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 0);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test both step count and size peak together
    {
        auto l_expr = v(8);
        auto l_result = l_expr->normalize(std::numeric_limits<size_t>::max(),
                                          std::numeric_limits<size_t>::max());
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(8)));
    }

    // Test all parameters together
    {
        auto l_expr = v(12);
        auto l_result = l_expr->normalize(100, 10);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(v(12)));
    }
}

void test_func_normalize()
{
    // func with body of a local
    {
        auto l_expr = f(v(0)->clone());

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());

        // make sure still a func
        const auto* l_func = dynamic_cast<func*>(l_result.m_expr.get());
        assert(l_func != nullptr);

        // get body
        const auto* l_body = dynamic_cast<var*>(l_func->body().get());
        assert(l_body != nullptr);

        // make sure body is still same thing
        assert(l_body->index() == 0);
    }

    // Test reduction count on func with no redex
    {
        auto l_expr = f(v(3));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(f(v(3))));
    }

    // Test reduction limit on func with no redex
    {
        auto l_expr = f(a(v(2), v(5)));
        auto l_result = l_expr->normalize(0);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(f(a(v(2), v(5)))));
    }

    // Test reduction count with redex inside function body
    // λ.((λ.0) 5) -> λ.0 (1 reduction inside body)
    // Note: v(0) at depth 1 refers to outer lambda, so it's not replaced
    {
        auto l_expr = f(a(f(v(0)), v(5)));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2); // λ.0 has size 2
        auto l_expected = f(v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test reduction limit stopping reduction inside function body
    // λ.((λ.1) 2) with limit 0 -> step limit reached after 0 steps (first
    // reduction blocked)
    {
        auto l_expr = f(a(f(v(1)), v(2)));
        auto l_result = l_expr->normalize(0);
        assert(l_result.m_step_excess == true); // Can reduce but hit limit
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test nested functions with multiple reductions
    // λ.λ.((λ.2) ((λ.3) 5)) -> λ.λ.(2 4) (2 reductions)
    {
        auto l_expr = f(f(a(f(v(2)), a(f(v(3)), v(5)))));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // First reduction: size 9 -> 6
        // Second reduction: size 6 -> 3
        // Peak is the max of intermediate result sizes = 6
        assert(l_result.m_size_peak == 6);
        auto l_expected = f(f(v(2)));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test size peak tracking on func with no reductions
    // λ.5 has size 2 and stays that way
    {
        auto l_expr = f(v(5));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(f(v(5))));
    }

    // Test size peak tracking on func with reduction where size decreases
    // λ.((λ.0) 5) has size 5, reduces to λ.0 with size 2
    // Peak should be 2 (size after reduction)
    {
        auto l_expr = f(a(f(v(0)), v(5)));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2);
        auto l_expected = f(v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test size peak with step count on func with reduction
    {
        auto l_expr = f(a(f(v(1)), v(2)));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2); // λ.2 has size 2
        assert(l_result.m_expr->equals(f(v(2))));
    }

    // Test size limit preventing reduction
    // λ.((λ.3) 4) has size 5, result would be λ.2 with size 2
    // If we set size limit to 1, the reduction's result (size 2) exceeds it
    {
        auto l_expr = f(a(f(v(3)), v(4)));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 1);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == true); // Result would exceed limit
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test size limit allowing reduction (limit >= result size)
    // λ.((λ.3) 4) with size limit 2 -> can reduce to λ.2 (size 2)
    {
        auto l_expr = f(a(f(v(3)), v(4)));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 2);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2);
        assert(l_result.m_expr->equals(f(v(2))));
    }

    // Test all parameters together on func
    {
        auto l_expr = f(a(f(v(1)), v(2)));
        auto l_result = l_expr->normalize(10, 100);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2);
        assert(l_result.m_expr->equals(f(v(2))));
    }

    // Test N-2 scenario for func: needs 2 reductions, limit 0 blocks all
    // λ.((λ.1) ((λ.2) 3)) needs 2 reductions in body
    {
        auto l_expr = f(a(f(v(1)), a(f(v(2)), v(3))));
        auto l_result = l_expr->normalize(0, 100);
        assert(l_result.m_step_excess == true); // Can reduce but hit limit
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        // No reductions performed, so still the original
        assert(l_result.m_expr->equals(l_expr));
    }
}

void test_app_normalize()
{
    // app with lhs and rhs both locals
    {
        auto l_lhs = v(0);
        auto l_rhs = v(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());

        const app* l_app = dynamic_cast<app*>(l_result.m_expr.get());
        assert(l_app != nullptr);

        // lhs should be local
        const var* l_reduced_lhs = dynamic_cast<var*>(l_app->lhs().get());
        assert(l_reduced_lhs != nullptr);

        // rhs should be local
        const var* l_reduced_rhs = dynamic_cast<var*>(l_app->rhs().get());
        assert(l_reduced_rhs != nullptr);

        // same on both (no reduction occurred)
        assert(l_reduced_lhs->index() == 0);
        assert(l_reduced_rhs->index() == 1);
    }

    // app with lhs func and rhs local
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = v(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1); // Result is v(1) with size 1

        // make sure nothing changed
        // (beta reduction did not occur since rhs is local)
        assert(l_result.m_expr->equals(v(1)));
    }

    // app with lhs func and rhs func
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = f(v(1)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2); // Result is f(v(1)) with size 2

        // make sure beta-reduction occurred, with no lifting of indices
        assert(l_result.m_expr->equals(l_rhs->clone()));
    }

    // app with lhs func (without occurrences of var 0) and rhs func
    {
        auto l_lhs = f(v(3)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1); // Result is v(2) with size 1

        // make sure beta-reduction occurred, but no replacements.
        // other vars decremented by 1.
        assert(l_result.m_expr->equals(v(2)->clone()));
    }

    // app with lhs (nested func with occurrences of var 0) and rhs func
    {
        auto l_lhs = f(f(v(0)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 3); // Result is f(f(v(6))) with size 3

        // make sure beta-reduction occurred, with replacement,
        // and a lifting of 1 level
        assert(l_result.m_expr->equals(f(f(v(6)->clone()))->clone()));
    }

    // app with lhs (nested func without occurrences of var 0) and rhs func
    {
        auto l_lhs = f(f(v(3)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2); // Result is f(v(2)) with size 2

        // make sure beta-reduction occurred, no replacements.
        // other vars decremented by 1.
        assert(l_result.m_expr->equals(f(v(2)->clone())));
    }

    // app with lhs (app that doesnt reduce to func) and rhs func
    {
        auto l_lhs = a(v(3)->clone(), v(4)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());

        // make sure nothing changed
        // (both lhs and rhs were fully reduced already)
        assert(l_result.m_expr->equals(l_expr->clone()));
    }

    // app with lhs (app with lhs (func without occurrances), rhs local)
    // and rhs func
    {
        auto l_lhs = a(f(v(3)->clone())->clone(), v(4)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 4); // Result is (2 f(v(5))) with size 4

        // lhs should have beta-reduced, but cannot consume rhs of app
        assert(l_result.m_expr->equals(a(v(2), f(v(5)))));
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are too many arguments supplied
    {
        auto l_lhs = a(f(v(3)->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 4); // Result is (2 f(v(5))) with size 4

        // lhs of app should beta-reduce, but lhs is not capable of consuming 2
        // args. Thus NF is an application with LHS beta-reduced once.
        assert(l_result.m_expr->equals(
            a(v(2)->clone(), f(v(5)->clone()))->clone()));
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs =
            a(f(f(v(3)->clone())->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: (λ.2) λ.5 has size 5 (2+2+1)
        // After 2nd reduction: v(1) has size 1
        // Peak is max of intermediate results = 5
        assert(l_result.m_size_peak == 5);

        // should beta-reduce twice, consuming all args. No replacements, only
        // decrementing twice.
        assert(l_result.m_expr->equals(v(1)->clone()));
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs =
            a(f(f(v(0)->clone())->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: (λ.λ.5) λ.5 has size 6 (3+2+1)
        // After 2nd reduction: λ.4 has size 2
        // Peak is max of intermediate results = 6
        assert(l_result.m_size_peak == 6);

        // should beta-reduce twice, consuming all args.
        // becomes first arg, without lifting. (lifting occurred but was undone
        // by second arg)
        assert(l_result.m_expr->equals(f(v(4)->clone())));
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs =
            a(f(f(v(1)->clone())->clone())->clone(), f(v(4)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        const auto l_result = l_expr->normalize();

        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: (λ.0) λ.5 has size 5 (2+2+1)
        // After 2nd reduction: λ.5 has size 2
        // Peak is max of intermediate results = 5
        assert(l_result.m_size_peak == 5);

        // should beta-reduce twice, consuming all args.
        // becomes second arg, without lifting. (lifting occurred but was undone
        // by second arg)
        assert(l_result.m_expr->equals(l_rhs->clone()));
    }

    // Test reduction count with single reduction
    // (λ.0) 5 -> 5 (1 reduction)
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test reduction count with multiple reductions
    // ((λ.0) 5) ((λ.1) 6) -> (5 ((λ.1) 6)) -> (5 0) (2 reductions)
    // Note: v(1) is a free variable, so it decrements to v(0)
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: (5 ((λ.1) 6)) has size 6 (1+1+(1+2+1)=6)
        // After 2nd reduction: (5 0) has size 3
        // Peak is max of intermediate results = 6
        assert(l_result.m_size_peak == 6);
        auto l_expected = a(v(5), v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test reduction count with church numeral reduction
    // Church numeral 2 applied: many reductions
    {
        auto l_two = f(f(a(v(0), a(v(0), v(1)))));
        auto l_f = f(v(10));
        auto l_x = v(5);
        auto l_expr = a(a(l_two->clone(), l_f->clone()), l_x->clone());
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        // Should have multiple reductions
        assert(l_result.m_step_count > 0);
        assert(l_result.m_size_peak > 0);
    }

    // Test limit of 0 - blocks first reduction
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->normalize(0);
        // Cannot do any reductions with limit 0
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test limit of 1 - allows 1 reduction, blocks second
    // ((λ.0) 5) ((λ.1) 6) needs 2 reductions total
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->normalize(1);
        // Does 1 reduction, then hits limit
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        // After 1 reduction: (5 ((λ.1) 6)) - partially normalized
        auto l_expected = a(v(5), a(f(v(1)), v(6)));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test limit exactly at needed reductions (reaches normal form)
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->normalize(2);
        // Needs 2 reductions, limit is 2, so reaches normal form
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // Peak is 6 (size after 1st reduction)
        assert(l_result.m_size_peak == 6);
        auto l_expected = a(v(5), v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test limit exceeds needed reductions
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->normalize(100);
        // Only needs 1 reduction, limit is 100
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test N-2 scenario: expression needs 2 reductions, limit is 0
    // Should do 0 reductions (blocked)
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->normalize(0);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test count with limit that allows full normalization
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->normalize(100);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test complex reduction with limit
    // ((λ.λ.0) 5) 6 -> (λ.6) 6 -> 5 (2 reductions total)
    // Note: v(6) in body is a free variable, so it decrements to v(5)
    {
        auto l_expr = a(a(f(f(v(0))), v(5)), v(6));

        // Limit to 0: blocks first reduction
        auto l_result0 = l_expr->normalize(0);
        assert(l_result0.m_step_excess == true);
        assert(l_result0.m_size_excess == false);
        assert(l_result0.m_step_count == 0);
        assert(l_result0.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result0.m_expr->equals(l_expr));

        // Limit to 1: allows 1 reduction
        auto l_result1 = l_expr->normalize(1);
        assert(l_result1.m_step_excess == true);
        assert(l_result1.m_size_excess == false);
        assert(l_result1.m_step_count == 1);

        // Limit to 2: allows 2 reductions (normal form)
        auto l_result2 = l_expr->normalize(2);
        assert(l_result2.m_step_excess == false);
        assert(l_result2.m_size_excess == false);
        assert(l_result2.m_step_count == 2);
        // After 1st reduction: (λ.6) 6 has size 4, after 2nd: v(5) has size 1
        // Peak is 4
        assert(l_result2.m_size_peak == 4);
        assert(l_result2.m_expr->equals(v(5)));

        // Full normalization
        auto l_result_full = l_expr->normalize();
        assert(l_result_full.m_step_excess == false);
        assert(l_result_full.m_size_excess == false);
        assert(l_result_full.m_step_count == 2);
        // Peak is 4
        assert(l_result_full.m_size_peak == 4);
        assert(l_result_full.m_expr->equals(v(5)));
    }

    // Test identity combinator: (λ.0) applied to expression
    // (λ.0) (λ.5) -> λ.5 (1 reduction)
    {
        auto l_identity = f(v(0));
        auto l_arg = f(v(5));
        auto l_expr = a(l_identity->clone(), l_arg->clone());
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2);
        assert(l_result.m_expr->equals(l_arg));
    }

    // Test normal order with limit - should reduce leftmost-outermost first
    // ((λ.2) 3) ((λ.4) 5) needs 2 reductions, limit 0 blocks all
    // Left side reduces: v(2) > 0 decrements to v(1)
    {
        auto l_lhs = a(f(v(2)), v(3));
        auto l_rhs = a(f(v(4)), v(5));
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());
        auto l_result = l_expr->normalize(0);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test normal order continuation - reduce left, then right
    // ((λ.2) 3) ((λ.4) 5) with limit 1 allows 1 reduction
    {
        auto l_lhs = a(f(v(2)), v(3));
        auto l_rhs = a(f(v(4)), v(5));
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());
        auto l_result = l_expr->normalize(1);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        // Left reduces first (normal order)
        auto l_expected = a(v(1), a(f(v(4)), v(5)));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test self-application with limit to prevent infinite reduction
    // (λ.0 0) (λ.0 0) with limit 2 (omega combinator)
    // This tests that the limit actually stops potentially infinite reductions
    {
        auto l_omega = f(a(v(0), v(0)));
        auto l_expr = a(l_omega->clone(), l_omega->clone());
        auto l_result = l_expr->normalize(2);
        assert(l_result.m_step_excess == true); // Hits limit
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // Should stop after 2 reductions, not continue indefinitely
    }

    // Test reduction inside nested applications
    // (1 ((λ.0) 2)) -> (1 2) (1 reduction in rhs)
    {
        auto l_expr = a(v(1), a(f(v(0)), v(2)));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 3); // (1 2) has size 3
        auto l_expected = a(v(1), v(2));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test multiple reductions in sequence with precise count
    // (((λ.0) 1) ((λ.0) 2)) 3 -> (1 2) 3 -> no further reduction (2 reductions)
    {
        auto l_expr = a(a(a(f(v(0)), v(1)), a(f(v(0)), v(2))), v(3));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: ((1 ((λ.0) 2)) 3) has size 8
        // After 2nd reduction: ((1 2) 3) has size 5
        // Peak is 8
        assert(l_result.m_size_peak == 8);
        auto l_expected = a(a(v(1), v(2)), v(3));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test function returning function with counting
    // (λ.λ.1) 5 -> λ.0 (1 reduction, v(1) at depth 1 decrements)
    {
        auto l_expr = a(f(f(v(1))), v(5));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 2);
        auto l_expected = f(v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test limit preventing full normalization of nested redexes
    // (λ.0) (λ.((λ.1) 2)) needs 2 reductions, limit 0 blocks all
    // Nothing reduces
    {
        auto l_inner_redex = a(f(v(1)), v(2));
        auto l_arg = f(l_inner_redex->clone());
        auto l_expr = a(f(v(0)), l_arg->clone());
        auto l_result = l_expr->normalize(0);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test complete normalization with nested redexes
    // (λ.0) (λ.((λ.1) 2)) with no limit -> (λ.2)
    // Outer reduces, then inner reduces
    {
        auto l_inner_redex = a(f(v(1)), v(2));
        auto l_arg = f(l_inner_redex->clone());
        auto l_expr = a(f(v(0)), l_arg->clone());
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After outer reduction: λ.((λ.1) 2) has size 5
        // After inner reduction: λ.2 has size 2
        // Peak is 5
        assert(l_result.m_size_peak == 5);
        auto l_expected = f(v(2));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test K combinator: (λ.λ.0) applied to two arguments
    // ((λ.λ.0) 5) 6 -> (λ.6) 6 -> 5 (2 reductions)
    // Note: v(0) at depth 1 gets replaced by v(5) lifted to v(6)
    //       Then v(6) > var_index(0) decrements to v(5)
    {
        auto l_k = f(f(v(0)));
        auto l_expr = a(a(l_k->clone(), v(5)), v(6));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: (λ.6) 6 has size 4
        // After 2nd reduction: v(5) has size 1
        // Peak is 4
        assert(l_result.m_size_peak == 4);
        auto l_expected = v(5);
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test size peak tracking on simple beta-reduction
    // (λ.0) 5 has size 4, reduces to 5 with size 1
    // Peak should be 1 (size of result after reduction)
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test size peak tracking on app with no reductions
    // (3 4) has size 3 and stays that way
    {
        auto l_expr = a(v(3), v(4));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(a(v(3), v(4))));
    }

    // Test size peak with step count on multiple reductions
    // ((λ.0) 5) ((λ.1) 6) -> (5 ((λ.1) 6)) -> (5 0)
    // Sizes: 9 -> 6 -> 3, peak is max of results (6 then 3) = 6
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        assert(l_result.m_size_peak == 6); // Peak of intermediate results
        auto l_expected = a(v(5), v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test size limit preventing reduction on application
    // (λ.0) 5 has size 4, result would be size 1, set limit to 0 to prevent
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 0);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == true); // Size limit blocks reduction
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test size limit allowing reduction (limit >= result size)
    // (λ.0) 5 with size limit 1 -> can reduce to 5 (size 1)
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 1);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test size limit stopping in the middle of multi-step reduction
    // ((λ.0) 5) ((λ.1) 6) can do 2 reductions
    // After 1 reduction: size 6, after 2: size 3
    // Set limit to 5: first reduction produces size 6 > 5, so size_excess
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 5);
        // First reduction would produce size 6 > 5, so blocked by size limit
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == true);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test size limit that allows partial reduction
    // Set limit high enough to allow reductions (6 allows first, 3 allows
    // second)
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 9);
        // Can do all reductions since result sizes are <= 9
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        assert(l_result.m_size_peak == 6);
        auto l_expected = a(v(5), v(0));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test all parameters together on application
    {
        auto l_expr = a(f(v(0)), v(8));
        auto l_result = l_expr->normalize(10, 100);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(8)));
    }

    // Test size limit with omega combinator to prevent explosion
    // (λ.0 0) (λ.0 0) with step limit
    // Each reduction produces same size
    {
        auto l_omega = f(a(v(0), v(0)));
        auto l_expr = a(l_omega->clone(), l_omega->clone());
        // Set step limit to 5 to prevent infinite loop
        auto l_result = l_expr->normalize(5, 100);
        assert(l_result.m_step_excess == true); // Hits step limit
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 5);
        assert(l_result.m_size_peak == 9); // Omega combinator stays same size
        assert(l_result.m_expr->equals(l_expr)); // Returns to itself
    }

    // Test size peak tracking where size increases during reduction
    // (λ.λ.(0 1)) 3 -> λ.(3 1)
    // Original size: 6, result size: 4
    {
        auto l_expr = a(f(f(a(v(0), v(1)))), v(3));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        // Peak is size of result
        assert(l_result.m_size_peak == 4);
        auto l_expected = f(a(v(4), v(0)));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test size limit preventing explosion in nested reductions
    // (λ.0) (λ.((λ.5) 6)) could reduce multiple times
    // After first: λ.((λ.5) 6), after second: λ.4
    {
        auto l_inner_redex = a(f(v(5)), v(6));
        auto l_arg = f(l_inner_redex->clone());
        auto l_expr = a(f(v(0)), l_arg->clone());
        // Allow all reductions
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 100);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // After 1st reduction: λ.((λ.5) 6) has size 5
        // After 2nd reduction: λ.4 has size 2
        // Peak is 5
        assert(l_result.m_size_peak == 5);
        auto l_expected = f(v(4));
        assert(l_result.m_expr->equals(l_expected));
    }

    // Test size limit exactly at boundary
    // (λ.0) 5 result is v(5) with size 1, so limit 1 allows reduction
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 1);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test both step limit and size limit active
    // Step limit should be hit (limit 0 blocks all)
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->normalize(0, 100);
        assert(l_result.m_step_excess == true); // Step limit hit
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test size limit hit before step limit
    // Result would be size 1, limit is 0, so size limit blocks
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->normalize(100, 0);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == true); // Size limit hit
        assert(l_result.m_step_count == 0);
        assert(l_result.m_size_peak == std::numeric_limits<size_t>::min());
        assert(l_result.m_expr->equals(l_expr));
    }

    // Test size_limit allowing reduction
    {
        auto l_expr = a(f(v(0)), v(7));
        auto l_result =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 10);
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        assert(l_result.m_size_peak == 1);
        assert(l_result.m_expr->equals(v(7)));
    }

    // Test complex expression with size tracking
    // ((λ.λ.0) 5) 6 -> (λ.6) 6 -> 5
    // Sizes: 7 -> 4 -> 1
    {
        auto l_k = f(f(v(0)));
        auto l_expr = a(a(l_k->clone(), v(5)), v(6));
        auto l_result = l_expr->normalize();
        assert(l_result.m_step_excess == false);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        assert(l_result.m_size_peak == 4); // Peak of intermediate results
        assert(l_result.m_expr->equals(v(5)));
    }

    // Test size limit on expression that GROWS during reduction
    // (λ.0 0) (λ.0 0 0) - like omega but grows exponentially
    // Original: 1 + 4 + 6 = 11
    // Step 1: (λ.0 0 0) (λ.0 0 0) - size 1 + 6 + 6 = 13
    // Step 2: ((λ.0 0 0) (λ.0 0 0)) (λ.0 0 0) - size 1 + 13 + 6 = 20
    // Step 3: (((λ.0 0 0) (λ.0 0 0)) (λ.0 0 0)) (λ.0 0 0) - size ≈ 27+
    {
        auto l_dup2 = f(a(v(0), v(0)));                    // λ.(0 0), size 4
        auto l_dup3 = f(a(a(v(0), v(0)), v(0)));           // λ.(0 0 0), size 6
        auto l_expr = a(l_dup2->clone(), l_dup3->clone()); // size 11

        // Test with step limit to see growth pattern
        // Limit 2 allows 2 steps
        auto l_result_3steps =
            l_expr->normalize(2, std::numeric_limits<size_t>::max());
        assert(l_result_3steps.m_step_excess == true);
        assert(l_result_3steps.m_size_excess == false);
        assert(l_result_3steps.m_step_count == 2);
        assert(l_result_3steps.m_size_peak == 20); // After 2 steps

        // Test with size limit that allows some growth but prevents explosion
        // Limit to 15: allows reductions until size exceeds 15
        // 11->13 (ok), 13->20 (exceeds), so 1 reduction then blocked
        auto l_result_limited =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 15);
        assert(l_result_limited.m_step_excess == false);
        assert(l_result_limited.m_size_excess ==
               true); // Size limit blocks second reduction
        assert(l_result_limited.m_step_count == 1);
        assert(l_result_limited.m_size_peak == 13);
        // Result should be: (λ.0 0 0) (λ.0 0 0) with size 13
        auto l_expected_limited = a(l_dup3->clone(), l_dup3->clone());
        assert(l_result_limited.m_expr->equals(l_expected_limited));

        // Test with size limit preventing any reduction
        // First reduction produces size 13, limit of 10 prevents it
        auto l_result_blocked =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 10);
        assert(l_result_blocked.m_step_excess == false);
        assert(l_result_blocked.m_size_excess == true);
        assert(l_result_blocked.m_step_count == 0);
        assert(l_result_blocked.m_size_peak ==
               std::numeric_limits<size_t>::min());
        assert(l_result_blocked.m_expr->equals(l_expr));

        // Test with size limit at exact boundary
        // Limit exactly at 13: allows first reduction (result size 13 <= 13)
        auto l_result_boundary =
            l_expr->normalize(std::numeric_limits<size_t>::max(), 13);
        assert(l_result_boundary.m_step_excess == false);
        assert(l_result_boundary.m_size_excess ==
               true); // Second reduction would exceed
        assert(l_result_boundary.m_step_count == 1);
        assert(l_result_boundary.m_size_peak == 13);
        // Result should be: (λ.0 0 0) (λ.0 0 0) with size 13
        auto l_expected_boundary = a(l_dup3->clone(), l_dup3->clone());
        assert(l_result_boundary.m_expr->equals(l_expected_boundary));
    }

    // Test N-2 scenario: 3-step expression with limit 1
    // Expression: (λ.0) (((λ.2) 3) ((λ.4) 5))
    // Needs 3 reductions total, limit 1 allows only 1 step
    {
        auto l_inner =
            a(a(f(v(2)), v(3)), a(f(v(4)), v(5)));  // ((λ.2) 3) ((λ.4) 5)
        auto l_expr = a(f(v(0)), l_inner->clone()); // (λ.0) (...)
        auto l_result = l_expr->normalize(1);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 1);
        // Should reduce outer app only
        // NOT reach full normal form
    }

    // Test N-2 scenario: 4-step expression limited to 2 steps
    // Expression: ((λ.0) (λ.0)) (((λ.2) 3) ((λ.4) 5))
    // Needs 4 reductions, limit 2 allows 2
    {
        auto l_inner =
            a(a(f(v(2)), v(3)), a(f(v(4)), v(5))); // ((λ.2) 3) ((λ.4) 5)
        auto l_wrapped =
            a(a(f(v(0)), f(v(0))), l_inner->clone()); // ((λ.0) (λ.0)) (...)
        auto l_result = l_wrapped->normalize(2);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 2);
        // Should NOT reach full normal form
    }

    // Test N-2 scenario: Expression that needs 5 steps, limit 3
    // Expression: ((((λ.0) 1) ((λ.0) 2)) ((λ.0) 3)) ((λ.0) 4)) ((λ.0) 5)
    // Needs 5 reductions, limit 3 allows 3
    {
        auto l_r1 = a(f(v(0)), v(1)); // (λ.0) 1
        auto l_r2 = a(f(v(0)), v(2)); // (λ.0) 2
        auto l_r3 = a(f(v(0)), v(3)); // (λ.0) 3
        auto l_r4 = a(f(v(0)), v(4)); // (λ.0) 4
        auto l_r5 = a(f(v(0)), v(5)); // (λ.0) 5
        auto l_expr = a(
            a(a(a(l_r1->clone(), l_r2->clone()), l_r3->clone()), l_r4->clone()),
            l_r5->clone());
        auto l_result = l_expr->normalize(3);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 3);
        // Expression is partially reduced, NOT in normal form
        // Result: (((1 2) 3) ((λ.0) 4)) ((λ.0) 5) - still has redexes left
    }

    // Test exact N-1 scenario: 4-step expression with limit 3
    // Expression: ((λ.0) 5) ((λ.0) 6) ((λ.0) 7) ((λ.0) 8) Needs 4
    // reductions, limit 3 allows 3
    {
        auto l_expr =
            a(a(a(a(f(v(0)), v(5)), a(f(v(0)), v(6))), a(f(v(0)), v(7))),
              a(f(v(0)), v(8)));
        auto l_result = l_expr->normalize(3);
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 3);
        // Result: ((5 6) ((λ.0) 7)) ((λ.0) 8) - partially reduced
    }
}

void test_var_reduce_one_step()
{
    // var at index 0 - cannot reduce
    {
        auto l_var = v(0);
        auto l_reduced = l_var->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // var at index 1 - cannot reduce
    {
        auto l_var = v(1);
        auto l_reduced = l_var->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // var at index 5 - cannot reduce
    {
        auto l_var = v(5);
        auto l_reduced = l_var->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // var at depth 0 - cannot reduce
    {
        auto l_var = v(3);
        auto l_reduced = l_var->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // var at depth 5 - cannot reduce
    {
        auto l_var = v(3);
        auto l_reduced = l_var->reduce_one_step(5);
        assert(l_reduced == nullptr);
    }

    // var at depth 10 - cannot reduce
    {
        auto l_var = v(7);
        auto l_reduced = l_var->reduce_one_step(10);
        assert(l_reduced == nullptr);
    }
}

void test_func_reduce_one_step()
{
    // func with var body - cannot reduce
    {
        auto l_func = f(v(0));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // func with var body at depth 5 - cannot reduce
    {
        auto l_func = f(v(3));
        auto l_reduced = l_func->reduce_one_step(5);
        assert(l_reduced == nullptr);
    }

    // func with app body (both vars) - cannot reduce
    {
        auto l_func = f(a(v(0), v(1)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // func with beta-redex in body where inner func references outer var
    // λ.((λ.0) 1) -> λ.0
    // Note: v(0) inside inner lambda refers to OUTER lambda (De Bruijn levels)
    {
        auto l_func = f(a(f(v(0)), v(1)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(0));
        assert(l_reduced->equals(l_expected));
    }

    // func with beta-redex in body where inner func uses its own bound var
    // λ.((λ.1) 2) -> λ.2
    // Note: v(1) inside inner lambda at depth 1 refers to inner lambda's bound
    // var
    {
        auto l_func = f(a(f(v(1)), v(2)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(2));
        assert(l_reduced->equals(l_expected));
    }

    // func with beta-redex in body at depth 3 - inner func refs outer context
    // λ.((λ.3) 2) -> λ.3
    // Note: At depth 3, inner lambda is at depth 4, v(3) refs depth 3 (outer
    // lambda)
    {
        auto l_func = f(a(f(v(3)), v(2)));
        auto l_reduced = l_func->reduce_one_step(3);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(3));
        assert(l_reduced->equals(l_expected));
    }

    // func with beta-redex where inner func uses its bound var at depth 3
    // λ.((λ.4) 2) -> λ.2
    // Note: At depth 3, inner lambda is at depth 4, v(4) is the inner lambda's
    // bound var
    {
        auto l_func = f(a(f(v(4)), v(2)));
        auto l_reduced = l_func->reduce_one_step(3);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(2));
        assert(l_reduced->equals(l_expected));
    }

    // func with nested beta-redex where inner functions ref outer lambda
    // λ.((λ.(λ.0)) 5) -> λ.(λ.0)
    // Note: v(0) refs outermost lambda, not the one at depth 1 being reduced
    {
        auto l_func = f(a(f(f(v(0))), v(5)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(f(v(0)));
        assert(l_reduced->equals(l_expected));
    }

    // func with nested beta-redex where inner function uses the bound var being
    // reduced λ.((λ.(λ.1)) 5) -> λ.(λ.6) Note: v(1) refs the lambda at depth 1,
    // which gets substituted with v(5) and lifted
    {
        auto l_func = f(a(f(f(v(1))), v(5)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(f(v(6)));
        assert(l_reduced->equals(l_expected));
    }

    // func with app that can reduce lhs (normal order)
    // λ.(((λ.0) 1) 2) - inner app lhs reduces first (leftmost-outermost)
    {
        auto l_func = f(a(a(f(v(0)), v(1)), v(2)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // After reducing inner app: v(0) < depth 1, stays v(0), result: λ.(0 2)
        auto l_expected = f(a(v(0), v(2)));
        assert(l_reduced->equals(l_expected));
    }

    // func with app where inner function uses its bound var
    // λ.(((λ.1) 2) 3) - inner app reduces, v(1) gets replaced with v(2)
    {
        auto l_func = f(a(a(f(v(1)), v(2)), v(3)));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // After reducing inner app at depth 1: v(1) replaced with v(2), result:
        // λ.(2 3)
        auto l_expected = f(a(v(2), v(3)));
        assert(l_reduced->equals(l_expected));
    }

    // func with nested func containing redex where inner func refs outer
    // context λ.λ.((λ.0) 1) -> λ.λ.0 Note: At depth 2, v(0) refs depth 0, not
    // being substituted
    {
        auto l_func = f(f(a(f(v(0)), v(1))));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(f(v(0)));
        assert(l_reduced->equals(l_expected));
    }

    // func with nested func where inner function uses innermost bound var
    // λ.λ.((λ.2) 1) -> λ.λ.1
    // Note: At depth 2, v(2) refs the innermost lambda, gets replaced with v(1)
    {
        auto l_func = f(f(a(f(v(2)), v(1))));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(f(v(1)));
        assert(l_reduced->equals(l_expected));
    }

    // func with body that has reducible app in rhs position
    // λ.(0 ((λ.1) 2)) -> λ.(0 2)
    {
        auto l_func = f(a(v(0), a(f(v(1)), v(2))));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(a(v(0), v(2)));
        assert(l_reduced->equals(l_expected));
    }
}

void test_app_reduce_one_step()
{
    // app of two vars - cannot reduce
    {
        auto l_app = a(v(0), v(1));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced == nullptr);
    }

    // app of two vars at depth 5 - cannot reduce
    {
        auto l_app = a(v(3), v(4));
        auto l_reduced = l_app->reduce_one_step(5);
        assert(l_reduced == nullptr);
    }

    // Beta-reduction: (λ.0) 1 -> 1
    {
        auto l_app = a(f(v(0)), v(1));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = v(1);
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction at depth 0: (λ.0) (λ.1) -> λ.1
    {
        auto l_app = a(f(v(0)), f(v(1)));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(1));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction at depth 3: (λ.3) 2 -> 2
    {
        auto l_app = a(f(v(3)), v(2));
        auto l_reduced = l_app->reduce_one_step(3);
        assert(l_reduced != nullptr);
        auto l_expected = v(2);
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with substitution (no lifting at depth 0): (λ.0) (λ.5) ->
    // λ.5 Note: Applications don't contribute to depth, so lift_amount = 0
    {
        auto l_app = a(f(v(0)), f(v(5)));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(5));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with var that needs decrementing: (λ.2) 1 -> 1
    {
        auto l_app = a(f(v(2)), v(1));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = v(1);
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with nested function: (λ.λ.0) 5 -> λ.5 (lifted to λ.6)
    {
        auto l_app = a(f(f(v(0))), v(5));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(6));
        assert(l_reduced->equals(l_expected));
    }

    // Normal order: reduce lhs before rhs
    // ((λ.0) 1) 2 -> lhs reduces to 1, result is (1 2)
    {
        auto l_app = a(a(f(v(0)), v(1)), v(2));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(v(1), v(2));
        assert(l_reduced->equals(l_expected));
    }

    // Normal order: lhs is not a redex, but can reduce
    // ((λ.2) 3) 4 -> ((1) 4) = (1 4)
    // Note: v(2) > var_index(0), so it's a free var that gets decremented
    {
        auto l_app = a(a(f(v(2)), v(3)), v(4));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // lhs: v(2) decrements to v(1), so result is (1 4)
        auto l_expected = a(v(1), v(4));
        assert(l_reduced->equals(l_expected));
    }

    // Normal order: lhs cannot reduce, try rhs
    // (0 ((λ.1) 2)) -> (0 0)
    // Note: v(1) > var_index(0), so it decrements to v(0)
    {
        auto l_app = a(v(0), a(f(v(1)), v(2)));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(v(0), v(0));
        assert(l_reduced->equals(l_expected));
    }

    // Normal order: lhs is var (cannot reduce), rhs reduces
    // (5 ((λ.0) 3)) -> (5 3)
    {
        auto l_app = a(v(5), a(f(v(0)), v(3)));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(v(5), v(3));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction takes priority over reducing sub-expressions
    // (λ.((λ.3) 4)) 2 -> beta-reduces outer first, result is ((λ.2) 3)
    // Note: v(3) and v(4) are free vars that get decremented when outer lambda
    // is removed
    {
        auto l_app = a(f(a(f(v(3)), v(4))), v(2));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(f(v(2)), v(3));
        assert(l_reduced->equals(l_expected));
    }

    // Complex beta-reduction: (λ.(0 0)) (λ.5) -> (λ.5 λ.5) without lifting
    // Note: At depth 0, no lifting occurs (applications don't add depth)
    {
        auto l_app = a(f(a(v(0), v(0))), f(v(5)));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // v(0) gets replaced with f(v(5)) with no lifting at depth 0
        auto l_expected = a(f(v(5)), f(v(5)));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with multiple occurrences: (λ.(0 0 0)) 3 -> (3 3 3)
    {
        auto l_app = a(f(a(a(v(0), v(0)), v(0))), v(3));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(a(v(3), v(3)), v(3));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with higher De Bruijn level: (λ.5) 3 -> 4 (5 decrements to
    // 4)
    {
        auto l_app = a(f(v(5)), v(3));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = v(4);
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with app as argument: (λ.0) (1 2) -> (1 2)
    {
        auto l_app = a(f(v(0)), a(v(1), v(2)));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(v(1), v(2));
        assert(l_reduced->equals(l_expected));
    }

    // Normal order: nested apps, leftmost-outermost reduction
    // (((λ.0) 1) ((λ.2) 3)) -> ((1) ((λ.2) 3))
    {
        auto l_lhs = a(f(v(0)), v(1));
        auto l_rhs = a(f(v(2)), v(3));
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // lhs reduces to v(1), rhs stays same
        auto l_expected = a(v(1), a(f(v(2)), v(3)));
        assert(l_reduced->equals(l_expected));
    }

    // Deeply nested beta-reduction at depth 10
    // (λ.10) 8 at depth 10 -> 8
    {
        auto l_app = a(f(v(10)), v(8));
        auto l_reduced = l_app->reduce_one_step(10);
        assert(l_reduced != nullptr);
        auto l_expected = v(8);
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with free variables
    // (λ.(1 0)) 5 -> (0 5)
    // Note: v(0) replaced with v(5), v(1) > var_index(0) so it decrements to
    // v(0)
    {
        auto l_app = a(f(a(v(1), v(0))), v(5));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // v(1) decrements to v(0) (free var), v(0) replaced with v(5)
        auto l_expected = a(v(0), v(5));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with complex body
    // (λ.((0 1) (2 0))) 7 -> ((7 0) (1 7))
    {
        auto l_body = a(a(v(0), v(1)), a(v(2), v(0)));
        auto l_app = a(f(l_body->clone()), v(7));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // v(0) -> v(7), v(1) -> v(0), v(2) -> v(1)
        auto l_expected = a(a(v(7), v(0)), a(v(1), v(7)));
        assert(l_reduced->equals(l_expected));
    }

    // Self-application: (λ.(0 0)) 5 -> (5 5)
    {
        auto l_app = a(f(a(v(0), v(0))), v(5));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = a(v(5), v(5));
        assert(l_reduced->equals(l_expected));
    }

    // Triple nested lambdas with substitution at various depths
    // λ.λ.λ.((λ.3) 5) at depth 0 -> reduces innermost app
    // v(3) at depth 4 (inside innermost lambda) refers to depth 3 (middle
    // lambda)
    {
        auto l_func = f(f(f(a(f(v(3)), v(5)))));
        auto l_reduced = l_func->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // After reduction: λ.λ.λ.5 (v(3) gets replaced with v(5), v(5)
        // decrements to v(4))
        auto l_expected = f(f(f(v(5))));
        assert(l_reduced->equals(l_expected));
    }

    // Normal order with multiple redexes - leftmost outer reduces first
    // ((λ.3) 1) ((λ.4) 2) -> (2 ((λ.4) 2))
    {
        auto l_lhs = a(f(v(3)), v(1));
        auto l_rhs = a(f(v(4)), v(2));
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // LHS reduces first: v(3) > 0 decrements to v(2)
        auto l_expected = a(v(2), a(f(v(4)), v(2)));
        assert(l_reduced->equals(l_expected));
    }

    // Reduction inside nested application structure
    // (2 (3 ((λ.5) 4))) -> reduces innermost redex
    {
        auto l_inner = a(f(v(5)), v(4));
        auto l_middle = a(v(3), l_inner->clone());
        auto l_outer = a(v(2), l_middle->clone());
        auto l_reduced = l_outer->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // Innermost reduces: v(5) > 0 decrements to v(4)
        auto l_expected = a(v(2), a(v(3), v(4)));
        assert(l_reduced->equals(l_expected));
    }

    // Lambda that returns a lambda with substitution
    // (λ.(λ.0)) 8 -> λ.9
    // Note: v(0) refers to the outermost lambda being reduced, gets replaced
    // with 8 lifted by 1
    {
        auto l_app = a(f(f(v(0))), v(8));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(v(9));
        assert(l_reduced->equals(l_expected));
    }

    // Lambda that returns a lambda with free var
    // (λ.(λ.1)) 8 -> λ.0
    // Note: v(1) at depth 1 > var_index(0), so it's a free var that decrements
    {
        auto l_app = a(f(f(v(1))), v(8));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // v(1) > var_index(0), decrements to v(0)
        auto l_expected = f(v(0));
        assert(l_reduced->equals(l_expected));
    }

    // Reduction with argument that contains bound variables
    // (λ.0) (λ.(λ.10)) -> λ.(λ.10)
    {
        auto l_app = a(f(v(0)), f(f(v(10))));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        auto l_expected = f(f(v(10)));
        assert(l_reduced->equals(l_expected));
    }

    // Multiple substitutions with lifting
    // (λ.(0 (λ.1))) 7 -> (7 (λ.8))
    {
        auto l_app = a(f(a(v(0), f(v(1)))), v(7));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // v(0) -> v(7), v(1) inside lambda at depth 1 > var_index(0) so:
        // decrements to v(0)
        auto l_expected = a(v(7), f(v(0)));
        assert(l_reduced->equals(l_expected));
    }

    // Deep nesting with correct depth tracking at depth 5
    // At depth 5: (λ.5) 3 -> 3 (v(5) is the bound var at depth 5)
    {
        auto l_app = a(f(v(5)), v(3));
        auto l_reduced = l_app->reduce_one_step(5);
        assert(l_reduced != nullptr);
        auto l_expected = v(3);
        assert(l_reduced->equals(l_expected));
    }

    // Verify that variables below depth are preserved
    // At depth 5: (λ.3) 10 -> 3 (v(3) < depth 5, preserved as free var)
    {
        auto l_app = a(f(v(3)), v(10));
        auto l_reduced = l_app->reduce_one_step(5);
        assert(l_reduced != nullptr);
        auto l_expected = v(3);
        assert(l_reduced->equals(l_expected));
    }

    // Edge case: var index exactly at depth gets substituted
    // At depth 7: (λ.7) 12 -> 12
    {
        auto l_app = a(f(v(7)), v(12));
        auto l_reduced = l_app->reduce_one_step(7);
        assert(l_reduced != nullptr);
        auto l_expected = v(12);
        assert(l_reduced->equals(l_expected));
    }

    // Composition pattern: (λ.λ.(1 (0 2))) 5 -> λ.(0 (6 1))
    // Tests correct handling of multiple bound vars at different depths
    {
        auto l_body = f(a(v(1), a(v(0), v(2))));
        auto l_app = a(f(l_body->clone()), v(5));
        auto l_reduced = l_app->reduce_one_step(0);
        assert(l_reduced != nullptr);
        // v(1) > var_index(0): decrements to v(0)
        // v(0) == var_index(0): replaced with v(5) lifted by 1 = v(6)
        // v(2) > var_index(0): decrements to v(1)
        auto l_expected = f(a(v(0), a(v(6), v(1))));
        assert(l_reduced->equals(l_expected));
    }

    // Beta-reduction with bound variable that should NOT be lifted
    // At depth 2: (λ.2) 1 -> 1
    // Note: v(1) < depth(2), so it's a bound var from outer scope, no lifting
    // occurs
    {
        auto l_app = a(f(v(2)), v(1));
        auto l_reduced = l_app->reduce_one_step(2);
        assert(l_reduced != nullptr);
        // v(2) at depth 2 gets replaced with v(1), which is < cutoff(2) so NOT
        // lifted
        auto l_expected = v(1);
        assert(l_reduced->equals(l_expected));
    }
}

void test_var_size()
{
    // Test that all variables have size 1
    {
        auto l_var = v(0);
        assert(l_var->size() == 1);
    }

    {
        auto l_var = v(5);
        assert(l_var->size() == 1);
    }

    {
        auto l_var = v(100);
        assert(l_var->size() == 1);
    }

    {
        auto l_var = v(999);
        assert(l_var->size() == 1);
    }
}

void test_func_size()
{
    // Test that func has size 1 + body size

    // Simple function with var body: λ.0 has size 2
    {
        auto l_func = f(v(0));
        assert(l_func->size() == 2);
    }

    {
        auto l_func = f(v(5));
        assert(l_func->size() == 2);
    }

    // Nested functions: λ.λ.0 has size 3
    {
        auto l_func = f(f(v(0)));
        assert(l_func->size() == 3);
    }

    // Triple nested: λ.λ.λ.0 has size 4
    {
        auto l_func = f(f(f(v(0))));
        assert(l_func->size() == 4);
    }

    // Function with application body: λ.(0 1) has size 4
    // Body is app: 1 + 1 (var 0) + 1 (var 1) = 3
    // Total: 1 + 3 = 4
    {
        auto l_func = f(a(v(0), v(1)));
        assert(l_func->size() == 4);
    }

    // Function with nested application: λ.((0 1) 2) has size 6
    // Body is app: 1 + (1+1+1) + 1 = 5
    // Total: 1 + 5 = 6
    {
        auto l_func = f(a(a(v(0), v(1)), v(2)));
        assert(l_func->size() == 6);
    }

    // Function with function application: λ.((λ.0) 1) has size 5
    // Body is app: 1 + 2 (λ.0) + 1 (var 1) = 4
    // Total: 1 + 4 = 5
    {
        auto l_func = f(a(f(v(0)), v(1)));
        assert(l_func->size() == 5);
    }

    // Complex nested: λ.λ.(0 (1 2)) has size 7
    // Inner app (1 2): 1 + 1 + 1 = 3
    // Outer app: 1 + 1 + 3 = 5
    // Inner func: 1 + 5 = 6
    // Outer func: 1 + 6 = 7
    {
        auto l_func = f(f(a(v(0), a(v(1), v(2)))));
        assert(l_func->size() == 7);
    }
}

void test_app_size()
{
    // Test that app has size 1 + lhs size + rhs size

    // Simple application: (0 1) has size 3
    {
        auto l_app = a(v(0), v(1));
        assert(l_app->size() == 3);
    }

    {
        auto l_app = a(v(5), v(10));
        assert(l_app->size() == 3);
    }

    // Application with function on left: ((λ.0) 1) has size 4
    // lhs is func: 1 + 1 = 2
    // rhs is var: 1
    // Total: 1 + 2 + 1 = 4
    {
        auto l_app = a(f(v(0)), v(1));
        assert(l_app->size() == 4);
    }

    // Application with function on right: (0 (λ.1)) has size 4
    {
        auto l_app = a(v(0), f(v(1)));
        assert(l_app->size() == 4);
    }

    // Both sides are functions: ((λ.0) (λ.1)) has size 5
    {
        auto l_app = a(f(v(0)), f(v(1)));
        assert(l_app->size() == 5);
    }

    // Nested application on left: ((0 1) 2) has size 5
    // lhs is app: 1 + 1 + 1 = 3
    // rhs is var: 1
    // Total: 1 + 3 + 1 = 5
    {
        auto l_app = a(a(v(0), v(1)), v(2));
        assert(l_app->size() == 5);
    }

    // Nested application on right: (0 (1 2)) has size 5
    {
        auto l_app = a(v(0), a(v(1), v(2)));
        assert(l_app->size() == 5);
    }

    // Both sides nested applications: ((0 1) (2 3)) has size 7
    // lhs: 3, rhs: 3, total: 1 + 3 + 3 = 7
    {
        auto l_app = a(a(v(0), v(1)), a(v(2), v(3)));
        assert(l_app->size() == 7);
    }

    // Complex: ((λ.0) (λ.λ.1)) has size 6
    // lhs: 2, rhs: 3, total: 1 + 2 + 3 = 6
    {
        auto l_app = a(f(v(0)), f(f(v(1))));
        assert(l_app->size() == 6);
    }

    // Very complex: ((λ.(0 1)) (λ.(2 3))) has size 9
    // lhs func body: 1 + 1 + 1 = 3, lhs func: 1 + 3 = 4
    // rhs func body: 1 + 1 + 1 = 3, rhs func: 1 + 3 = 4
    // Total: 1 + 4 + 4 = 9
    {
        auto l_app = a(f(a(v(0), v(1))), f(a(v(2), v(3))));
        assert(l_app->size() == 9);
    }

    // Self-application: ((λ.0) (λ.0)) has size 5
    {
        auto l_func = f(v(0));
        auto l_app = a(l_func->clone(), l_func->clone());
        assert(l_app->size() == 5);
    }

    // Omega combinator: (λ.(0 0)) has size 4
    // body: 1 + 1 + 1 = 3
    // func: 1 + 3 = 4
    {
        auto l_omega_func = f(a(v(0), v(0)));
        assert(l_omega_func->size() == 4);
    }

    // Full omega: ((λ.(0 0)) (λ.(0 0))) has size 9
    {
        auto l_omega_func = f(a(v(0), v(0)));
        auto l_omega = a(l_omega_func->clone(), l_omega_func->clone());
        assert(l_omega->size() == 9);
    }

    // K combinator: (λ.λ.0) has size 3
    {
        auto l_k = f(f(v(0)));
        assert(l_k->size() == 3);
    }

    // S combinator: λ.λ.λ.((0 2) (1 2)) has size 10
    // (1 2): 3
    // (0 2): 3
    // ((0 2) (1 2)): 1 + 3 + 3 = 7
    // λ: 1 + 7 = 8
    // λ: 1 + 8 = 9
    // λ: 1 + 9 = 10
    {
        auto l_s = f(f(f(a(a(v(0), v(2)), a(v(1), v(2))))));
        assert(l_s->size() == 10);
    }

    // Church numeral ZERO: λ.λ.0 has size 3
    {
        auto l_zero = f(f(v(0)));
        assert(l_zero->size() == 3);
    }

    // Church numeral ONE: λ.λ.(1 0) has size 5
    // (1 0): 3
    // λ: 1 + 3 = 4
    // λ: 1 + 4 = 5
    {
        auto l_one = f(f(a(v(1), v(0))));
        assert(l_one->size() == 5);
    }

    // Church numeral TWO: λ.λ.(1 (1 0)) has size 7
    // (1 0): 3
    // (1 (1 0)): 1 + 1 + 3 = 5
    // λ: 1 + 5 = 6
    // λ: 1 + 6 = 7
    {
        auto l_two = f(f(a(v(1), a(v(1), v(0)))));
        assert(l_two->size() == 7);
    }

    // Test that size remains constant after cloning
    {
        auto l_expr = a(f(a(v(0), v(1))), v(2));
        size_t l_original_size = l_expr->size();
        auto l_cloned = l_expr->clone();
        assert(l_cloned->size() == l_original_size);
        assert(l_cloned->size() == 6);
    }
}

std::unique_ptr<expr> construct_program(
    std::list<std::unique_ptr<expr>>::const_iterator a_helpers_begin,
    std::list<std::unique_ptr<expr>>::const_iterator a_helpers_end,
    const std::unique_ptr<expr>& a_main_fn)
{
    // we will construct a tower of abstractions,
    // with the main function at the bottom.
    if(a_helpers_begin == a_helpers_end)
        return a_main_fn->clone();

    // construct an abstraction and recur
    return a(f(construct_program(std::next(a_helpers_begin), a_helpers_end,
                                 a_main_fn)),
             (*a_helpers_begin)->clone());
}

void generic_use_case_test()
{
    using namespace lambda;
    std::list<std::unique_ptr<expr>> l_helpers{};

    auto l = [&l_helpers](size_t a_local_index)
    { return v(l_helpers.size() + a_local_index); };

    auto g = [&l_helpers](size_t a_global_index) { return v(a_global_index); };

    // church booleans

    // true
    const auto TRUE = g(l_helpers.size());
    l_helpers.emplace_back(f(f(l(0))));
    // false
    const auto FALSE = g(l_helpers.size());
    l_helpers.emplace_back(f(f(l(1))));

    // test the church bools
    {
        // true case
        const auto l_true_case = f(l(10));

        // false case
        const auto l_false_case = f(l(11));

        // test the true case
        const auto l_true_case_main =
            a(a(TRUE->clone(), l_true_case->clone()), l_false_case->clone());

        // test the false case
        const auto l_false_case_main =
            a(a(FALSE->clone(), l_true_case->clone()), l_false_case->clone());

        // construct the program
        const auto l_true_program = construct_program(
            l_helpers.begin(), l_helpers.end(), l_true_case_main);

        // construct the program
        const auto l_false_program = construct_program(
            l_helpers.begin(), l_helpers.end(), l_false_case_main);

        // reduce the programs
        const auto l_true_result = l_true_program->normalize();
        const auto l_false_result = l_false_program->normalize();

        assert(l_true_result.m_step_excess == false);
        assert(l_true_result.m_size_excess == false);
        assert(l_false_result.m_step_excess == false);
        assert(l_false_result.m_size_excess == false);

        std::cout << "true reduced: ";
        l_true_result.m_expr->print(std::cout);
        std::cout << std::endl;

        std::cout << "false reduced: ";
        l_false_result.m_expr->print(std::cout);
        std::cout << std::endl;

        // test the reductions.
        // NOTE: after reduction of a program, the main function's locals BECOME
        // globals.
        assert(l_true_result.m_expr->equals(f(g(10))));
        assert(l_false_result.m_expr->equals(f(g(11))));
    }

    // add church numerals

    // 0
    const auto ZERO = g(l_helpers.size());
    l_helpers.emplace_back(f(f(l(1))));

    // succ
    const auto SUCC = g(l_helpers.size());
    l_helpers.emplace_back(f(f(f(a(l(1), a(a(l(0), l(1)), l(2)))))));

    // test succ church numerals
    {
        // construct 1 - 5

        const auto ONE = a(SUCC->clone(), ZERO->clone());
        const auto TWO = a(SUCC->clone(), ONE->clone());
        const auto THREE = a(SUCC->clone(), TWO->clone());
        const auto FOUR = a(SUCC->clone(), THREE->clone());
        const auto FIVE = a(SUCC->clone(), FOUR->clone());

        // construct the programs
        const auto ZERO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ZERO);
        const auto ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ONE);
        const auto TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), TWO);
        const auto THREE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), THREE);
        const auto FOUR_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), FOUR);
        const auto FIVE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), FIVE);

        // reduce zero
        const auto ZERO_RESULT = ZERO_PROGRAM->normalize();
        assert(ZERO_RESULT.m_step_excess == false);
        assert(ZERO_RESULT.m_size_excess == false);
        std::cout << "zero reduced: ";
        ZERO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        // define one
        const auto ONE_RESULT = ONE_PROGRAM->normalize();
        assert(ONE_RESULT.m_step_excess == false);
        assert(ONE_RESULT.m_size_excess == false);
        std::cout << "one reduced: ";
        ONE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        // define two
        const auto TWO_RESULT = TWO_PROGRAM->normalize();
        assert(TWO_RESULT.m_step_excess == false);
        assert(TWO_RESULT.m_size_excess == false);
        std::cout << "two reduced: ";
        TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        // define three
        const auto THREE_RESULT = THREE_PROGRAM->normalize();
        assert(THREE_RESULT.m_step_excess == false);
        assert(THREE_RESULT.m_size_excess == false);
        std::cout << "three reduced: ";
        THREE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        // define four
        const auto FOUR_RESULT = FOUR_PROGRAM->normalize();
        assert(FOUR_RESULT.m_step_excess == false);
        assert(FOUR_RESULT.m_size_excess == false);
        std::cout << "four reduced: ";
        FOUR_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        // define five
        const auto FIVE_RESULT = FIVE_PROGRAM->normalize();
        assert(FIVE_RESULT.m_step_excess == false);
        assert(FIVE_RESULT.m_size_excess == false);
        std::cout << "five reduced: ";
        FIVE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        assert(ONE_RESULT.m_expr->equals(f(f(a(g(0), g(1))))));
        assert(TWO_RESULT.m_expr->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(
            THREE_RESULT.m_expr->equals(f(f(a(g(0), a(g(0), a(g(0), g(1))))))));
        assert(FOUR_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(FIVE_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1))))))))));
    }

    // add
    const auto ADD = g(l_helpers.size());
    l_helpers.emplace_back(
        f(f(f(f(a(a(l(0), l(2)), a(a(l(1), l(2)), l(3))))))));

    // test add church numerals
    {
        // construct 1 - 5

        const auto ONE = a(SUCC->clone(), ZERO->clone());
        const auto TWO = a(SUCC->clone(), ONE->clone());
        const auto THREE = a(SUCC->clone(), TWO->clone());
        const auto FOUR = a(SUCC->clone(), THREE->clone());
        const auto FIVE = a(SUCC->clone(), FOUR->clone());

        // add one and one
        const auto ADD_ONE_ONE = a(a(ADD->clone(), ONE->clone()), ONE->clone());

        // add one and two
        const auto ADD_ONE_TWO = a(a(ADD->clone(), ONE->clone()), TWO->clone());

        // add two and two
        const auto ADD_TWO_TWO = a(a(ADD->clone(), TWO->clone()), TWO->clone());

        // add three and two
        const auto ADD_THREE_TWO =
            a(a(ADD->clone(), THREE->clone()), TWO->clone());

        // add five and five
        const auto ADD_FIVE_FIVE =
            a(a(ADD->clone(), FIVE->clone()), FIVE->clone());

        const auto ADD_ONE_ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_ONE_ONE);
        const auto ADD_ONE_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_ONE_TWO);
        const auto ADD_TWO_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_TWO_TWO);
        const auto ADD_THREE_TWO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), ADD_THREE_TWO);
        const auto ADD_FIVE_FIVE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), ADD_FIVE_FIVE);

        // reduce the programs
        const auto ADD_ONE_ONE_RESULT = ADD_ONE_ONE_PROGRAM->normalize();
        const auto ADD_ONE_TWO_RESULT = ADD_ONE_TWO_PROGRAM->normalize();
        const auto ADD_TWO_TWO_RESULT = ADD_TWO_TWO_PROGRAM->normalize();
        const auto ADD_THREE_TWO_RESULT = ADD_THREE_TWO_PROGRAM->normalize();
        const auto ADD_FIVE_FIVE_RESULT = ADD_FIVE_FIVE_PROGRAM->normalize();

        assert(ADD_ONE_ONE_RESULT.m_step_excess == false);
        assert(ADD_ONE_ONE_RESULT.m_size_excess == false);
        assert(ADD_ONE_TWO_RESULT.m_step_excess == false);
        assert(ADD_ONE_TWO_RESULT.m_size_excess == false);
        assert(ADD_TWO_TWO_RESULT.m_step_excess == false);
        assert(ADD_TWO_TWO_RESULT.m_size_excess == false);
        assert(ADD_THREE_TWO_RESULT.m_step_excess == false);
        assert(ADD_THREE_TWO_RESULT.m_size_excess == false);
        assert(ADD_FIVE_FIVE_RESULT.m_step_excess == false);
        assert(ADD_FIVE_FIVE_RESULT.m_size_excess == false);

        std::cout << "add one one: ";
        ADD_ONE_ONE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        std::cout << "add one two: ";
        ADD_ONE_TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        std::cout << "add two two: ";
        ADD_TWO_TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        std::cout << "add three two: ";
        ADD_THREE_TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        std::cout << "add five five: ";
        ADD_FIVE_FIVE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        // assertions
        assert(ADD_ONE_ONE_RESULT.m_expr->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(ADD_ONE_TWO_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), g(1))))))));
        assert(ADD_TWO_TWO_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(ADD_THREE_TWO_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1))))))))));
        assert(ADD_FIVE_FIVE_RESULT.m_expr->equals(f(f(a(
            g(0),
            a(g(0),
              a(g(0),
                a(g(0),
                  a(g(0),
                    a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))))))))));
    }

    // define MULT
    const auto MULT = g(l_helpers.size());
    l_helpers.emplace_back(f(f(f(f(a(a(l(0), a(l(1), l(2))), l(3)))))));

    // test mult church numerals
    {
        // construct 1 - 5
        const auto ONE = a(SUCC->clone(), ZERO->clone());
        const auto TWO = a(SUCC->clone(), ONE->clone());
        const auto THREE = a(SUCC->clone(), TWO->clone());
        const auto FOUR = a(SUCC->clone(), THREE->clone());
        const auto FIVE = a(SUCC->clone(), FOUR->clone());

        // construct unreduced main functions
        const auto MULT_ZERO_ZERO =
            a(a(MULT->clone(), ZERO->clone()), ZERO->clone());
        const auto MULT_ZERO_ONE =
            a(a(MULT->clone(), ZERO->clone()), ONE->clone());
        const auto MULT_ONE_ONE =
            a(a(MULT->clone(), ONE->clone()), ONE->clone());
        const auto MULT_ONE_TWO =
            a(a(MULT->clone(), ONE->clone()), TWO->clone());
        const auto MULT_TWO_TWO =
            a(a(MULT->clone(), TWO->clone()), TWO->clone());
        const auto MULT_THREE_TWO =
            a(a(MULT->clone(), THREE->clone()), TWO->clone());
        const auto MULT_FIVE_FIVE =
            a(a(MULT->clone(), FIVE->clone()), FIVE->clone());

        // construct programs
        const auto MULT_ZERO_ZERO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_ZERO_ZERO);
        const auto MULT_ZERO_ONE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_ZERO_ONE);
        const auto MULT_ONE_ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), MULT_ONE_ONE);
        const auto MULT_ONE_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), MULT_ONE_TWO);
        const auto MULT_TWO_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), MULT_TWO_TWO);
        const auto MULT_THREE_TWO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_THREE_TWO);
        const auto MULT_FIVE_FIVE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_FIVE_FIVE);

        // reduce the programs (with printing in between)
        const auto MULT_ZERO_ZERO_RESULT = MULT_ZERO_ZERO_PROGRAM->normalize();
        assert(MULT_ZERO_ZERO_RESULT.m_step_excess == false);
        assert(MULT_ZERO_ZERO_RESULT.m_size_excess == false);
        std::cout << "mult zero zero: ";
        MULT_ZERO_ZERO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        const auto MULT_ZERO_ONE_RESULT = MULT_ZERO_ONE_PROGRAM->normalize();
        assert(MULT_ZERO_ONE_RESULT.m_step_excess == false);
        assert(MULT_ZERO_ONE_RESULT.m_size_excess == false);
        std::cout << "mult zero one: ";
        MULT_ZERO_ONE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        const auto MULT_ONE_ONE_RESULT = MULT_ONE_ONE_PROGRAM->normalize();
        assert(MULT_ONE_ONE_RESULT.m_step_excess == false);
        assert(MULT_ONE_ONE_RESULT.m_size_excess == false);
        std::cout << "mult one one: ";
        MULT_ONE_ONE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        const auto MULT_ONE_TWO_RESULT = MULT_ONE_TWO_PROGRAM->normalize();
        assert(MULT_ONE_TWO_RESULT.m_step_excess == false);
        assert(MULT_ONE_TWO_RESULT.m_size_excess == false);
        std::cout << "mult one two: ";
        MULT_ONE_TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        const auto MULT_TWO_TWO_RESULT = MULT_TWO_TWO_PROGRAM->normalize();
        assert(MULT_TWO_TWO_RESULT.m_step_excess == false);
        assert(MULT_TWO_TWO_RESULT.m_size_excess == false);
        std::cout << "mult two two: ";
        MULT_TWO_TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        const auto MULT_THREE_TWO_RESULT = MULT_THREE_TWO_PROGRAM->normalize();
        assert(MULT_THREE_TWO_RESULT.m_step_excess == false);
        assert(MULT_THREE_TWO_RESULT.m_size_excess == false);
        std::cout << "mult three two: ";
        MULT_THREE_TWO_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;
        const auto MULT_FIVE_FIVE_RESULT = MULT_FIVE_FIVE_PROGRAM->normalize();
        assert(MULT_FIVE_FIVE_RESULT.m_step_excess == false);
        assert(MULT_FIVE_FIVE_RESULT.m_size_excess == false);
        std::cout << "mult five five: ";
        MULT_FIVE_FIVE_RESULT.m_expr->print(std::cout);
        std::cout << std::endl;

        assert(MULT_ZERO_ZERO_RESULT.m_expr->equals(f(f(g(1)))));
        assert(MULT_ZERO_ONE_RESULT.m_expr->equals(f(f(g(1)))));
        assert(MULT_ONE_ONE_RESULT.m_expr->equals(f(f(a(g(0), g(1))))));
        assert(
            MULT_ONE_TWO_RESULT.m_expr->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(MULT_TWO_TWO_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(MULT_THREE_TWO_RESULT.m_expr->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))))));

        const auto TWENTY_FIVE = f(f(a(
            g(0),
            a(g(0),
              a(g(0),
                a(g(0),
                  a(g(0),
                    a(g(0),
                      a(g(0),
                        a(g(0),
                          a(g(0),
                            a(g(0),
                              a(g(0),
                                a(g(0),
                                  a(g(0),
                                    a(g(0),
                                      a(g(0),
                                        a(g(0),
                                          a(g(0),
                                            a(g(0),
                                              a(g(0),
                                                a(g(0),
                                                  a(g(0),
                                                    a(g(0),
                                                      a(g(0),
                                                        a(g(0),
                                                          a(g(0),
                                                            g(1))))))))))))))))))))))))))));

        assert(MULT_FIVE_FIVE_RESULT.m_expr->equals(TWENTY_FIVE->clone()));
    }

    // test for fun
    {
        // this should never terminate -- omega combinator
        const auto OMEGA = a(f(a(v(0), v(0))), f(a(v(0), v(0))));
        auto l_result = OMEGA->normalize(999);
        // omega becomes itself forever
        // With limit 999, does 999 reductions (hits step limit)
        assert(l_result.m_step_excess == true);
        assert(l_result.m_size_excess == false);
        assert(l_result.m_step_count == 999);
        assert(l_result.m_expr->equals(OMEGA->clone()));
    }

    // Test combinator identities (I, K, S)
    // These are very sensitive to capture/shifting bugs
    {
        // Define combinators
        // I = λx. x
        const auto I = f(v(0));

        // K = λx. λy. x
        const auto K = f(f(v(0)));

        // S = λx. λy. λz. x z (y z)
        const auto S = f(f(f(a(a(v(0), v(2)), a(v(1), v(2))))));

        // Test I a → a
        {
            auto a_arg = v(5);
            auto expr = a(I->clone(), a_arg->clone());
            auto result = expr->normalize();
            assert(result.m_step_excess == false);
            assert(result.m_size_excess == false);
            std::cout << "I a: ";
            result.m_expr->print(std::cout);
            std::cout << std::endl;
            assert(result.m_expr->equals(a_arg));
        }

        // Test K a b → a
        {
            auto a_arg = v(7);
            auto b_arg = v(8);
            auto expr = a(a(K->clone(), a_arg->clone()), b_arg->clone());
            auto result = expr->normalize();
            assert(result.m_step_excess == false);
            assert(result.m_size_excess == false);
            std::cout << "K a b: ";
            result.m_expr->print(std::cout);
            std::cout << std::endl;
            assert(result.m_expr->equals(a_arg));
        }

        // Test S K K a → a
        {
            auto a_arg = v(10);
            auto expr =
                a(a(a(S->clone(), K->clone()), K->clone()), a_arg->clone());
            auto result = expr->normalize();
            assert(result.m_step_excess == false);
            assert(result.m_size_excess == false);
            std::cout << "S K K a: ";
            result.m_expr->print(std::cout);
            std::cout << std::endl;
            assert(result.m_expr->equals(a_arg));
        }

        // Test S I I a → a a
        {
            auto a_arg = v(12);
            auto expr =
                a(a(a(S->clone(), I->clone()), I->clone()), a_arg->clone());
            auto result = expr->normalize();
            assert(result.m_step_excess == false);
            assert(result.m_size_excess == false);
            std::cout << "S I I a: ";
            result.m_expr->print(std::cout);
            std::cout << std::endl;
            auto expected = a(a_arg->clone(), a_arg->clone());
            assert(result.m_expr->equals(expected));
        }

        // Additional test: Use closed terms instead of free variables
        // Test K (λ.5) (λ.6) → λ.5
        {
            auto a_arg = f(v(5));
            auto b_arg = f(v(6));
            auto expr = a(a(K->clone(), a_arg->clone()), b_arg->clone());
            auto result = expr->normalize();
            assert(result.m_step_excess == false);
            assert(result.m_size_excess == false);
            std::cout << "K (λ.5) (λ.6): ";
            result.m_expr->print(std::cout);
            std::cout << std::endl;
            assert(result.m_expr->equals(a_arg));
        }

        // Test S I I (λ.7) → (λ.7) (λ.7) → 6
        {
            auto a_arg = f(v(7));
            auto expr =
                a(a(a(S->clone(), I->clone()), I->clone()), a_arg->clone());
            auto result = expr->normalize();
            assert(result.m_step_excess == false);
            assert(result.m_size_excess == false);
            std::cout << "S I I (λ.7): ";
            result.m_expr->print(std::cout);
            std::cout << std::endl;
            auto expected = v(6);
            assert(result.m_expr->equals(expected));
        }
    }
}

void lambda_test_main()
{
    constexpr bool ENABLE_DEBUG_LOGS = true;

    TEST(test_var_constructor);
    TEST(test_func_constructor);
    TEST(test_app_constructor);

    TEST(test_var_equals);
    TEST(test_func_equals);
    TEST(test_app_equals);

    TEST(test_var_lift);
    TEST(test_func_lift);
    TEST(test_app_lift);

    TEST(test_var_substitute);
    TEST(test_func_substitute);
    TEST(test_app_substitute);

    TEST(test_var_normalize);
    TEST(test_func_normalize);
    TEST(test_app_normalize);

    TEST(test_var_reduce_one_step);
    TEST(test_func_reduce_one_step);
    TEST(test_app_reduce_one_step);

    TEST(test_var_size);
    TEST(test_func_size);
    TEST(test_app_size);

    TEST(generic_use_case_test);
}

#endif
