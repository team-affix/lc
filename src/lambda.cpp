#include "../include/lambda.hpp"

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

    return m_lhs->equals(l_casted->m_lhs) && m_rhs->equals(l_casted->m_rhs);
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

// EXPR CLONE METHOD

std::unique_ptr<expr> var::clone() const
{
    return v(m_index);
}

std::unique_ptr<expr> func::clone() const
{
    return f(m_body->clone());
}

std::unique_ptr<expr> app::clone() const
{
    return a(m_lhs->clone(), m_rhs->clone());
}

// UPDATE SIZE METHODS

void var::update_size()
{
    m_size = 1;
}

void func::update_size()
{
    m_size = 1 + m_body->m_size;
}

void app::update_size()
{
    m_size = 1 + m_lhs->m_size + m_rhs->m_size;
}

// LIFT METHODS

void var::lift(size_t a_lift_amount, size_t a_cutoff)
{
    // the variable is bound, so don't lift it
    if(m_index < a_cutoff)
        return;

    // lift the variable
    m_index += a_lift_amount;
}

void func::lift(size_t a_lift_amount, size_t a_cutoff)
{
    // we don't increment here, since the goal is to lift the WHOLE function
    // (all locals inside) by the same amount (provided they are >= cutoff).
    m_body->lift(a_lift_amount, a_cutoff);
}

void app::lift(size_t a_lift_amount, size_t a_cutoff)
{
    // lift the lhs and rhs
    m_lhs->lift(a_lift_amount, a_cutoff);
    m_rhs->lift(a_lift_amount, a_cutoff);
}

// CONSTRUCTORS
expr::expr() : m_size(0)
{
}

var::var(size_t a_index) : expr(), m_index(a_index)
{
    update_size();
}

func::func(std::unique_ptr<expr>&& a_body) : expr(), m_body(std::move(a_body))
{
    update_size();
}

app::app(std::unique_ptr<expr>&& a_lhs, std::unique_ptr<expr>&& a_rhs)
    : expr(), m_lhs(std::move(a_lhs)), m_rhs(std::move(a_rhs))
{
    update_size();
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

// operator for printing expressions to ostreams
std::ostream& operator<<(std::ostream& a_ostream, const expr& a_expr)
{
    a_expr.print(a_ostream);
    return a_ostream;
}

// REWRITING FUNCTIONS

// between root of a_expr and the occurrance of the variable.
void substitute(std::unique_ptr<expr>& a_expr, size_t a_lift_amount,
                size_t a_var_index, const std::unique_ptr<expr>& a_arg)
{
    if(var* l_var = dynamic_cast<var*>(a_expr.get()))
    {
        if(l_var->m_index > a_var_index)
        {
            // this var is defined inside the redex (free), so it is
            //     now 1 level shallower.
            --l_var->m_index;
            return;
        }

        if(l_var->m_index < a_var_index)
        {
            // leave the var alone, it was declared outside the redex
            // (bound)
            return;
        }

        // this var is the one we are substituting, so we must substitute it
        a_expr = a_arg->clone();
        a_expr->lift(a_lift_amount, a_var_index);

        return;
    }

    if(func* l_func = dynamic_cast<func*>(a_expr.get()))
    {
        // increment the binder depth
        substitute(l_func->m_body, a_lift_amount + 1, a_var_index, a_arg);

        // update the size of the function
        l_func->update_size();

        return;
    }

    if(app* l_app = dynamic_cast<app*>(a_expr.get()))
    {
        // substitute the lhs and rhs
        substitute(l_app->m_lhs, a_lift_amount, a_var_index, a_arg);
        substitute(l_app->m_rhs, a_lift_amount, a_var_index, a_arg);

        // update the size of the application
        l_app->update_size();

        return;
    }

    // if we get here, error
    throw std::runtime_error("substitute: invalid expression type");
}

bool reduce_one_step(std::unique_ptr<expr>& a_expr, size_t a_depth)
{
    if(var* l_var = dynamic_cast<var*>(a_expr.get()))
    {
        // variables cannot reduce
        return false;
    }

    if(func* l_func = dynamic_cast<func*>(a_expr.get()))
    {
        // just try to reduce the body by 1 step
        if(reduce_one_step(l_func->m_body, a_depth + 1))
        {
            // update the size of the function
            l_func->update_size();

            return true;
        }

        return false;
    }

    if(app* l_app = dynamic_cast<app*>(a_expr.get()))
    {
        // if this app is a beta-redex, beta-contract the body
        if(func* l_lhs_func = dynamic_cast<func*>(l_app->m_lhs.get()))
        {
            // perform the beta-contraction
            substitute(l_lhs_func->m_body, 0, a_depth, l_app->m_rhs);

            // throw away the lambda binder
            // NOTE: a_expr already knows its new size, so we don't need to
            // update it here.
            a_expr = std::move(l_lhs_func->m_body);

            return true;
        }

        // try to reduce lhs IF FAIL, rhs (in that order)
        if(reduce_one_step(l_app->m_lhs, a_depth) ||
           reduce_one_step(l_app->m_rhs, a_depth))
        {
            // update the size of the application
            l_app->update_size();

            return true;
        }

        return false;
    }

    // if we get here, error
    throw std::runtime_error("reduce_one_step: invalid expression type");
}

} // namespace lambda

#ifdef UNIT_TEST

#include "../testing/test_utils.hpp"
#include <iostream>
#include <limits>
#include <list>

using namespace lambda;

void test_var_constructor()
{
    // index 0
    {
        auto l_var = v(0);
        const var* l_var_casted = dynamic_cast<var*>(l_var.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 0);
        assert(l_var->m_size == 1);
    }

    // index 1
    {
        auto l_var = v(1);
        const var* l_var_casted = dynamic_cast<var*>(l_var.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 1);
        assert(l_var->m_size == 1);
    }
}

void test_func_constructor()
{
    // local body
    {
        auto l_func = f(v(0));
        assert(l_func->m_size == 2);
        // get body
        const func* l_func_casted = dynamic_cast<func*>(l_func.get());
        assert(l_func_casted != nullptr);
        const auto& l_body = l_func_casted->m_body;
        // check if the body is a local
        const var* l_var = dynamic_cast<var*>(l_body.get());
        assert(l_var != nullptr);
        // check if the index is correct
        assert(l_var->m_index == 0);
    }
}

void test_app_constructor()
{
    // local application
    {
        auto l_app = a(v(0), v(1));
        assert(l_app->m_size == 3);
        // get the lhs
        const app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);
        const auto& l_lhs = l_app_casted->m_lhs;
        // get the rhs
        const auto& l_rhs = l_app_casted->m_rhs;

        // make sure they both are locals
        const var* l_lhs_var = dynamic_cast<var*>(l_lhs.get());
        const var* l_rhs_var = dynamic_cast<var*>(l_rhs.get());
        assert(l_lhs_var != nullptr);
        assert(l_rhs_var != nullptr);

        // make sure the indices are correct
        assert(l_lhs_var->m_index == 0);
        assert(l_rhs_var->m_index == 1);
    }
}

void test_var_equals()
{
    // index 0, equals index 0
    {
        auto l_var = v(0);
        auto l_local_other = v(0);
        assert(l_var->equals(l_local_other));
    }

    // index 0, equals index 1
    {
        auto l_var = v(0);
        auto l_local_other = v(1);
        assert(!l_var->equals(l_local_other));
    }

    // index 1, equals index 1
    {
        auto l_var = v(1);
        auto l_local_other = v(1);
        assert(l_var->equals(l_local_other));
    }

    // local equals func
    {
        auto l_var = v(0);
        auto l_func = f(v(0));
        assert(!l_var->equals(l_func));
    }

    // local equals app
    {
        auto l_var = v(0);
        auto l_app = a(v(0), v(0));
        assert(!l_var->equals(l_app));
    }
}

void test_func_equals()
{
    // local body, equals local body
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        auto l_func_other = f(l_var->clone());
        assert(l_func->equals(l_func_other));
    }

    // func equals local
    {
        auto l_func = f(v(0));
        auto l_var = v(0);
        assert(!l_func->equals(l_var));
    }

    // func with different bodies
    {
        auto l_var = v(0);
        auto l_local_other = v(1);
        auto l_func = f(l_var->clone());
        auto l_func_other = f(l_local_other->clone());
        assert(!l_func->equals(l_func_other));
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
        assert(l_app->equals(l_app_other));
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

void test_var_clone()
{
    // basic clone - check index is preserved
    {
        auto l_var = v(0);
        auto l_cloned = l_var->clone();
        assert(l_var->equals(l_cloned));

        const var* l_var_casted = dynamic_cast<var*>(l_cloned.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 0);
    }

    // clone with different index
    {
        auto l_var = v(5);
        auto l_cloned = l_var->clone();
        assert(l_var->equals(l_cloned));

        const var* l_var_casted = dynamic_cast<var*>(l_cloned.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 5);
    }

    // ensure deep copy - modifying clone doesn't affect original
    {
        auto l_var = v(3);
        auto l_cloned = l_var->clone();

        // modify the clone
        var* l_cloned_var = dynamic_cast<var*>(l_cloned.get());
        assert(l_cloned_var != nullptr);
        l_cloned_var->m_index = 10;

        // check original is unchanged
        const var* l_original_var = dynamic_cast<var*>(l_var.get());
        assert(l_original_var != nullptr);
        assert(l_original_var->m_index == 3);

        // check they're no longer equal
        assert(!l_var->equals(l_cloned));
    }

    // check size is preserved
    {
        auto l_var = v(42);
        auto l_cloned = l_var->clone();
        assert(l_var->m_size == l_cloned->m_size);
        assert(l_cloned->m_size == 1);
    }

    // test with large index
    {
        auto l_var = v(999);
        auto l_cloned = l_var->clone();
        assert(l_var->equals(l_cloned));

        const var* l_var_casted = dynamic_cast<var*>(l_cloned.get());
        assert(l_var_casted != nullptr);
        assert(l_var_casted->m_index == 999);
    }

    // verify distinct unique_ptr and independent m_size modification
    {
        auto l_var = v(7);
        auto l_cloned = l_var->clone();

        // verify pointers are different
        assert(l_var.get() != l_cloned.get());

        // verify initial sizes are equal
        assert(l_var->m_size == l_cloned->m_size);
        assert(l_var->m_size == 1);

        // modify clone's m_size
        l_cloned->m_size = 42;

        // verify original's m_size is unchanged
        assert(l_var->m_size == 1);
        assert(l_cloned->m_size == 42);
    }
}

void test_func_clone()
{
    // basic clone - simple body
    {
        auto l_func = f(v(0));
        auto l_cloned = l_func->clone();
        assert(l_func->equals(l_cloned));

        const func* l_func_casted = dynamic_cast<func*>(l_cloned.get());
        assert(l_func_casted != nullptr);
        assert(l_func_casted->m_body != nullptr);
    }

    // clone with application body
    {
        auto l_func = f(a(v(0), v(1)));
        auto l_cloned = l_func->clone();
        assert(l_func->equals(l_cloned));

        const func* l_func_casted = dynamic_cast<func*>(l_cloned.get());
        assert(l_func_casted != nullptr);

        const app* l_body_app = dynamic_cast<app*>(l_func_casted->m_body.get());
        assert(l_body_app != nullptr);
    }

    // nested functions
    {
        auto l_func = f(f(v(0)));
        auto l_cloned = l_func->clone();
        assert(l_func->equals(l_cloned));

        const func* l_outer = dynamic_cast<func*>(l_cloned.get());
        assert(l_outer != nullptr);

        const func* l_inner = dynamic_cast<func*>(l_outer->m_body.get());
        assert(l_inner != nullptr);
    }

    // ensure deep copy - modifying clone doesn't affect original
    {
        auto l_func = f(v(2));
        auto l_cloned = l_func->clone();

        // modify the clone's body
        func* l_cloned_func = dynamic_cast<func*>(l_cloned.get());
        assert(l_cloned_func != nullptr);
        l_cloned_func->m_body = v(99);

        // check original is unchanged
        const func* l_original_func = dynamic_cast<func*>(l_func.get());
        assert(l_original_func != nullptr);
        const var* l_original_body =
            dynamic_cast<var*>(l_original_func->m_body.get());
        assert(l_original_body != nullptr);
        assert(l_original_body->m_index == 2);

        // check they're no longer equal
        assert(!l_func->equals(l_cloned));
    }

    // check size is preserved
    {
        auto l_func = f(a(v(0), v(1)));
        auto l_cloned = l_func->clone();
        assert(l_func->m_size == l_cloned->m_size);
        assert(l_cloned->m_size == 4); // func + app + 2 vars
    }

    // complex nested structure
    {
        // λ.(λ.((0 1) 2))
        auto l_func = f(f(a(a(v(0), v(1)), v(2))));
        auto l_cloned = l_func->clone();
        assert(l_func->equals(l_cloned));
        assert(l_func->m_size == l_cloned->m_size);
    }

    // clone preserves structure after lift
    {
        auto l_func = f(v(5));
        l_func->lift(3, 0);
        auto l_cloned = l_func->clone();

        const func* l_func_casted = dynamic_cast<func*>(l_cloned.get());
        assert(l_func_casted != nullptr);
        const var* l_body_var = dynamic_cast<var*>(l_func_casted->m_body.get());
        assert(l_body_var != nullptr);
        assert(l_body_var->m_index == 8); // 5 + 3
    }

    // verify distinct unique_ptr and independent m_size modification
    {
        auto l_func = f(a(v(0), v(1)));
        auto l_cloned = l_func->clone();

        // verify pointers are different
        assert(l_func.get() != l_cloned.get());

        // verify initial sizes are equal
        assert(l_func->m_size == l_cloned->m_size);
        assert(l_func->m_size == 4);

        // modify clone's m_size
        l_cloned->m_size = 999;

        // verify original's m_size is unchanged
        assert(l_func->m_size == 4);
        assert(l_cloned->m_size == 999);
    }
}

void test_app_clone()
{
    // basic clone - simple application
    {
        auto l_app = a(v(0), v(1));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));

        const app* l_app_casted = dynamic_cast<app*>(l_cloned.get());
        assert(l_app_casted != nullptr);
        assert(l_app_casted->m_lhs != nullptr);
        assert(l_app_casted->m_rhs != nullptr);
    }

    // clone with function on lhs
    {
        auto l_app = a(f(v(0)), v(1));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));

        const app* l_app_casted = dynamic_cast<app*>(l_cloned.get());
        assert(l_app_casted != nullptr);

        const func* l_lhs_func = dynamic_cast<func*>(l_app_casted->m_lhs.get());
        assert(l_lhs_func != nullptr);
    }

    // clone with function on rhs
    {
        auto l_app = a(v(0), f(v(1)));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));

        const app* l_app_casted = dynamic_cast<app*>(l_cloned.get());
        assert(l_app_casted != nullptr);

        const func* l_rhs_func = dynamic_cast<func*>(l_app_casted->m_rhs.get());
        assert(l_rhs_func != nullptr);
    }

    // nested applications
    {
        auto l_app = a(a(v(0), v(1)), v(2));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));

        const app* l_outer = dynamic_cast<app*>(l_cloned.get());
        assert(l_outer != nullptr);

        const app* l_inner = dynamic_cast<app*>(l_outer->m_lhs.get());
        assert(l_inner != nullptr);
    }

    // ensure deep copy - modifying clone doesn't affect original
    {
        auto l_app = a(v(3), v(4));
        auto l_cloned = l_app->clone();

        // modify the clone's lhs
        app* l_cloned_app = dynamic_cast<app*>(l_cloned.get());
        assert(l_cloned_app != nullptr);
        l_cloned_app->m_lhs = v(99);

        // check original is unchanged
        const app* l_original_app = dynamic_cast<app*>(l_app.get());
        assert(l_original_app != nullptr);
        const var* l_original_lhs =
            dynamic_cast<var*>(l_original_app->m_lhs.get());
        assert(l_original_lhs != nullptr);
        assert(l_original_lhs->m_index == 3);

        // check they're no longer equal
        assert(!l_app->equals(l_cloned));
    }

    // check size is preserved
    {
        auto l_app = a(f(v(0)), a(v(1), v(2)));
        auto l_cloned = l_app->clone();
        assert(l_app->m_size == l_cloned->m_size);
        assert(l_cloned->m_size == 6); // app + func + var + app + var + var
    }

    // complex nested structure with multiple levels
    {
        // ((λ.(0 1)) (λ.2))
        auto l_app = a(f(a(v(0), v(1))), f(v(2)));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));
        assert(l_app->m_size == l_cloned->m_size);
    }

    // right-nested applications
    {
        // (0 (1 (2 3)))
        auto l_app = a(v(0), a(v(1), a(v(2), v(3))));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));
        assert(l_app->m_size == l_cloned->m_size);
        assert(l_cloned->m_size == 7);
    }

    // clone preserves structure after lift
    {
        auto l_app = a(v(5), v(6));
        l_app->lift(2, 0);
        auto l_cloned = l_app->clone();

        const app* l_app_casted = dynamic_cast<app*>(l_cloned.get());
        assert(l_app_casted != nullptr);

        const var* l_lhs_var = dynamic_cast<var*>(l_app_casted->m_lhs.get());
        const var* l_rhs_var = dynamic_cast<var*>(l_app_casted->m_rhs.get());
        assert(l_lhs_var != nullptr);
        assert(l_rhs_var != nullptr);
        assert(l_lhs_var->m_index == 7); // 5 + 2
        assert(l_rhs_var->m_index == 8); // 6 + 2
    }

    // clone of redex structure (beta-reducible)
    {
        // ((λ.0) 5)
        auto l_app = a(f(v(0)), v(5));
        auto l_cloned = l_app->clone();
        assert(l_app->equals(l_cloned));

        const app* l_cloned_app = dynamic_cast<app*>(l_cloned.get());
        assert(l_cloned_app != nullptr);

        const func* l_lhs = dynamic_cast<func*>(l_cloned_app->m_lhs.get());
        const var* l_rhs = dynamic_cast<var*>(l_cloned_app->m_rhs.get());
        assert(l_lhs != nullptr);
        assert(l_rhs != nullptr);
        assert(l_rhs->m_index == 5);
    }

    // verify distinct unique_ptr and independent m_size modification
    {
        auto l_app = a(f(v(0)), a(v(1), v(2)));
        auto l_cloned = l_app->clone();

        // verify pointers are different
        assert(l_app.get() != l_cloned.get());

        // verify initial sizes are equal
        assert(l_app->m_size == l_cloned->m_size);
        assert(l_app->m_size == 6);

        // modify clone's m_size
        l_cloned->m_size = 12345;

        // verify original's m_size is unchanged
        assert(l_app->m_size == 6);
        assert(l_cloned->m_size == 12345);
    }
}

void test_var_update_size()
{
    // basic update - var size is always 1
    {
        auto l_var = v(0);
        assert(l_var->m_size == 1);

        l_var->update_size();
        assert(l_var->m_size == 1);
    }

    // set m_size to garbage, then update
    {
        auto l_var = v(5);
        assert(l_var->m_size == 1);

        // corrupt the size
        l_var->m_size = 999;
        assert(l_var->m_size == 999);

        // update_size should fix it
        l_var->update_size();
        assert(l_var->m_size == 1);
    }

    // different indices don't affect size
    {
        auto l_var = v(100);
        assert(l_var->m_size == 1);

        l_var->m_size = 42;
        l_var->update_size();
        assert(l_var->m_size == 1);
    }

    // multiple updates should be idempotent
    {
        auto l_var = v(7);
        l_var->m_size = 12345;

        l_var->update_size();
        assert(l_var->m_size == 1);

        l_var->update_size();
        assert(l_var->m_size == 1);

        l_var->update_size();
        assert(l_var->m_size == 1);
    }
}

void test_func_update_size()
{
    // basic update - simple body
    {
        auto l_func = f(v(0));
        assert(l_func->m_size == 2); // func + var

        l_func->update_size();
        assert(l_func->m_size == 2);
    }

    // replace body with smaller expression, verify size doesn't auto-update
    {
        auto l_func = f(a(v(0), v(1))); // size: 1 + 3 = 4
        assert(l_func->m_size == 4);

        // replace body with a single var (size 1)
        func* l_func_casted = dynamic_cast<func*>(l_func.get());
        assert(l_func_casted != nullptr);
        l_func_casted->m_body = v(0);

        // size should still be old value (4) - not auto-updated
        assert(l_func->m_size == 4);

        // now update_size should fix it
        l_func->update_size();
        assert(l_func->m_size == 2); // 1 + 1
    }

    // replace body with larger expression
    {
        auto l_func = f(v(0)); // size: 1 + 1 = 2
        assert(l_func->m_size == 2);

        // replace body with nested application (size 5)
        func* l_func_casted = dynamic_cast<func*>(l_func.get());
        assert(l_func_casted != nullptr);
        l_func_casted->m_body =
            a(a(v(0), v(1)), v(2)); // size: 1 + (1 + 1 + 1) + 1 = 5

        // size should still be old value (2)
        assert(l_func->m_size == 2);

        // now update_size should fix it
        l_func->update_size();
        assert(l_func->m_size == 6); // 1 + 5
    }

    // nested function - replace inner body
    {
        auto l_func = f(f(v(0))); // size: 1 + (1 + 1) = 3
        assert(l_func->m_size == 3);

        func* l_outer = dynamic_cast<func*>(l_func.get());
        assert(l_outer != nullptr);
        func* l_inner = dynamic_cast<func*>(l_outer->m_body.get());
        assert(l_inner != nullptr);

        // replace inner body with application (size 3)
        l_inner->m_body = a(v(5), v(6)); // size 3

        // outer size should still be 3
        assert(l_func->m_size == 3);

        // update inner first
        l_inner->update_size();
        assert(l_inner->m_size == 4); // 1 + 3

        // outer still hasn't updated
        assert(l_func->m_size == 3);

        // now update outer
        l_outer->update_size();
        assert(l_func->m_size == 5); // 1 + 4
    }

    // set m_size to garbage, then update
    {
        auto l_func = f(a(v(0), v(1)));
        l_func->m_size = 999;

        l_func->update_size();
        assert(l_func->m_size == 4); // 1 + 3
    }

    // complex body with multiple levels
    {
        // λ.(λ.((0 1) 2))
        auto l_func = f(f(a(a(v(0), v(1)), v(2))));
        // Inner app: 1 + 1 + 1 = 3
        // Outer app: 1 + 3 + 1 = 5
        // Inner func: 1 + 5 = 6
        // Outer func: 1 + 6 = 7
        assert(l_func->m_size == 7);

        func* l_outer = dynamic_cast<func*>(l_func.get());
        l_outer->m_body = v(42); // replace with single var

        assert(l_func->m_size == 7); // still old size

        l_func->update_size();
        assert(l_func->m_size == 2); // 1 + 1
    }
}

void test_app_update_size()
{
    // basic update - simple application
    {
        auto l_app = a(v(0), v(1));
        assert(l_app->m_size == 3); // app + var + var

        l_app->update_size();
        assert(l_app->m_size == 3);
    }

    // replace lhs with smaller expression
    {
        auto l_app = a(f(v(0)), v(1)); // size: 1 + 2 + 1 = 4
        assert(l_app->m_size == 4);

        // replace lhs with a single var (size 1)
        app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);
        l_app_casted->m_lhs = v(5);

        // size should still be old value (4) - not auto-updated
        assert(l_app->m_size == 4);

        // now update_size should fix it
        l_app->update_size();
        assert(l_app->m_size == 3); // 1 + 1 + 1
    }

    // replace rhs with larger expression
    {
        auto l_app = a(v(0), v(1)); // size: 1 + 1 + 1 = 3
        assert(l_app->m_size == 3);

        // replace rhs with application (size 3)
        app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);
        l_app_casted->m_rhs = a(v(2), v(3)); // size 3

        // size should still be old value (3)
        assert(l_app->m_size == 3);

        // now update_size should fix it
        l_app->update_size();
        assert(l_app->m_size == 5); // 1 + 1 + 3
    }

    // replace both lhs and rhs
    {
        auto l_app = a(v(0), v(1)); // size: 3
        assert(l_app->m_size == 3);

        app* l_app_casted = dynamic_cast<app*>(l_app.get());
        assert(l_app_casted != nullptr);

        // replace lhs with func (size 2)
        l_app_casted->m_lhs = f(v(0));
        // replace rhs with application (size 5)
        l_app_casted->m_rhs = a(a(v(1), v(2)), v(3));

        // size should still be old value
        assert(l_app->m_size == 3);

        // now update_size should fix it
        l_app->update_size();
        assert(l_app->m_size == 8); // 1 + 2 + 5
    }

    // nested application - replace inner expression
    {
        auto l_app = a(a(v(0), v(1)), v(2)); // size: 1 + 3 + 1 = 5
        assert(l_app->m_size == 5);

        app* l_outer = dynamic_cast<app*>(l_app.get());
        assert(l_outer != nullptr);
        app* l_inner = dynamic_cast<app*>(l_outer->m_lhs.get());
        assert(l_inner != nullptr);

        // replace inner's lhs with function (size 2)
        l_inner->m_lhs = f(v(10));

        // outer size should still be 5
        assert(l_app->m_size == 5);

        // update inner first
        l_inner->update_size();
        assert(l_inner->m_size == 4); // 1 + 2 + 1

        // outer still hasn't updated
        assert(l_app->m_size == 5);

        // now update outer
        l_outer->update_size();
        assert(l_app->m_size == 6); // 1 + 4 + 1
    }

    // set m_size to garbage, then update
    {
        auto l_app = a(f(v(0)), a(v(1), v(2)));
        l_app->m_size = 12345;

        l_app->update_size();
        assert(l_app->m_size == 6); // 1 + 2 + 3
    }

    // complex nested structure
    {
        // ((λ.(0 1)) (λ.2))
        auto l_app = a(f(a(v(0), v(1))), f(v(2)));
        // Left app: 1 + 1 + 1 = 3
        // Left func: 1 + 3 = 4
        // Right func: 1 + 1 = 2
        // Outer app: 1 + 4 + 2 = 7
        assert(l_app->m_size == 7);

        app* l_app_casted = dynamic_cast<app*>(l_app.get());
        l_app_casted->m_lhs = v(99); // replace with single var

        assert(l_app->m_size == 7); // still old size

        l_app->update_size();
        assert(l_app->m_size == 4); // 1 + 1 + 2
    }

    // deeply right-nested applications
    {
        // (0 (1 (2 3)))
        auto l_app = a(v(0), a(v(1), a(v(2), v(3))));
        // Innermost: 1 + 1 + 1 = 3
        // Middle: 1 + 1 + 3 = 5
        // Outer: 1 + 1 + 5 = 7
        assert(l_app->m_size == 7);

        app* l_outer = dynamic_cast<app*>(l_app.get());
        l_outer->m_rhs = v(42); // replace with single var

        assert(l_app->m_size == 7); // still old size

        l_app->update_size();
        assert(l_app->m_size == 3); // 1 + 1 + 1
    }

    // replace with same-sized but different structure
    {
        auto l_app = a(a(v(0), v(1)), v(2)); // size 5
        assert(l_app->m_size == 5);

        app* l_app_casted = dynamic_cast<app*>(l_app.get());
        l_app_casted->m_lhs = v(99);         // replace with var (size 1)
        l_app_casted->m_rhs = a(v(3), v(4)); // replace with app (size 3)

        assert(l_app->m_size == 5); // still old size

        l_app->update_size();
        assert(l_app->m_size ==
               5); // 1 + 1 + 3 (same size, different structure)
    }
}

void test_var_lift()
{
    // index 0, lift 1 level
    {
        auto l_var = v(0);
        l_var->lift(1, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // index 1, lift 1 level
    {
        auto l_var = v(1);
        l_var->lift(1, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 2);
    }

    // index 1, lift 0 levels
    {
        auto l_var = v(1);
        l_var->lift(0, 0);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // index 0, lift 1 level, cutoff 1
    {
        auto l_var = v(0);
        l_var->lift(1, 1);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 0);
    }

    // index 1, lift 2 levels, cutoff 1
    {
        auto l_var = v(1);
        l_var->lift(2, 1);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 3);
    }

    // index 1, lift 2 levels, cutoff 2
    {
        auto l_var = v(1);
        l_var->lift(2, 2);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // Edge case: index equals cutoff, should be lifted (>= cutoff)
    // index 3, lift 5 levels, cutoff 3
    {
        auto l_var = v(3);
        l_var->lift(5, 3);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 8); // 3 + 5
    }

    // Edge case: index just below cutoff
    // index 4, lift 3 levels, cutoff 5
    {
        auto l_var = v(4);
        l_var->lift(3, 5);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 4); // not lifted
    }

    // Higher cutoff value test
    // index 7, lift 10 levels, cutoff 3
    {
        auto l_var = v(7);
        l_var->lift(10, 3);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 17); // 7 + 10
    }

    // Multiple different cutoffs - lower index below cutoff
    // index 2, lift 4 levels, cutoff 10
    {
        auto l_var = v(2);
        l_var->lift(4, 10);
        const var* l_lifted_var = dynamic_cast<var*>(l_var.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 2); // not lifted
    }
}

void test_func_lift()
{
    // local body, lift 1 level
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        l_func->lift(1, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_func.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 1);
    }

    // local body, lift 2 levels
    {
        auto l_var = v(1);
        auto l_func = f(l_var->clone());
        l_func->lift(2, 0);
        const func* l_lifted_func = dynamic_cast<func*>(l_func.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 3);
    }

    // local body, lift 1 level, cutoff 1
    {
        auto l_var = v(0);
        auto l_func = f(l_var->clone());
        l_func->lift(1, 1);
        const func* l_lifted_func = dynamic_cast<func*>(l_func.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 0);
    }

    // local body, lift 2 levels, cutoff 2
    {
        auto l_var = v(2);
        auto l_func = f(l_var->clone());
        l_func->lift(2, 2);
        const func* l_lifted_func = dynamic_cast<func*>(l_func.get());
        assert(l_lifted_func != nullptr);
        const var* l_lifted_var =
            dynamic_cast<var*>(l_lifted_func->m_body.get());
        assert(l_lifted_var != nullptr);
        assert(l_lifted_var->m_index == 4);
    }

    // func with app body, mixed locals with various cutoffs
    // Body: (2 5 8), lift 3, cutoff 5
    {
        auto l_body = a(a(v(2), v(5)), v(8));
        auto l_func = f(l_body->clone());
        l_func->lift(3, 5);

        const func* l_lifted_func = dynamic_cast<func*>(l_func.get());
        assert(l_lifted_func != nullptr);

        // Expected: (2 8 11)
        // l(2) < 5, not lifted
        // l(5) >= 5, lifted to 8
        // l(8) >= 5, lifted to 11
        auto l_expected = f(a(a(v(2), v(8)), v(11)));
        assert(l_func->equals(l_expected));
    }

    // func with nested func, testing cutoff propagation
    // f(f((1 3 6))), lift 2, cutoff 3
    {
        auto l_inner_body = a(a(v(1), v(3)), v(6));
        auto l_body = f(l_inner_body->clone());
        auto l_func = f(l_body->clone());
        l_func->lift(2, 3);

        // Expected: f(f((1 5 8)))
        // l(1) < 3, not lifted
        // l(3) >= 3, lifted to 5
        // l(6) >= 3, lifted to 8
        auto l_expected = f(f(a(a(v(1), v(5)), v(8))));
        assert(l_func->equals(l_expected));
    }

    // func with higher cutoff than any local
    // f(l(2)), lift 5, cutoff 10
    {
        auto l_func = f(v(2));
        l_func->lift(5, 10);

        // l(2) < 10, not lifted
        auto l_expected = f(v(2));
        assert(l_func->equals(l_expected));
    }
}

void test_app_lift()
{
    // local lhs, local rhs, lift 1 level
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        l_app->lift(1, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_app.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_lhs;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_rhs;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 3);
    }

    // local lhs, local rhs, lift 2 levels
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        l_app->lift(2, 0);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_app.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_lhs;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 3);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_rhs;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 4);
    }

    // local lhs, local rhs, lift 1 level, cutoff 1
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        l_app->lift(1, 1);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_app.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_lhs;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 2);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_rhs;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 3);
    }

    // local lhs, local rhs, lift 2 levels, cutoff 2
    {
        auto l_lhs_var = v(1);
        auto l_rhs_var = v(2);
        auto l_app = a(l_lhs_var->clone(), l_rhs_var->clone());
        l_app->lift(2, 2);

        // get the lifted app (still an app)
        const app* l_lifted_app = dynamic_cast<app*>(l_app.get());
        assert(l_lifted_app != nullptr);

        // get the lifted lhs
        const auto& l_lifted_lhs = l_lifted_app->m_lhs;
        const var* l_lifted_lhs_local = dynamic_cast<var*>(l_lifted_lhs.get());
        assert(l_lifted_lhs_local != nullptr);
        assert(l_lifted_lhs_local->m_index == 1);

        // get the lifted rhs
        const auto& l_lifted_rhs = l_lifted_app->m_rhs;
        const var* l_lifted_rhs_local = dynamic_cast<var*>(l_lifted_rhs.get());
        assert(l_lifted_rhs_local != nullptr);
        assert(l_lifted_rhs_local->m_index == 4);
    }

    // app with mixed locals, lift 4, cutoff 3
    // (1 2 3 4 5) - mix below, at, and above cutoff
    {
        auto l_app = a(a(a(a(v(1), v(2)), v(3)), v(4)), v(5));
        l_app->lift(4, 3);

        // Expected: (1 2 7 8 9)
        // l(1), l(2) < 3, not lifted
        // l(3), l(4), l(5) >= 3, lifted by 4
        auto l_expected = a(a(a(a(v(1), v(2)), v(7)), v(8)), v(9));
        assert(l_app->equals(l_expected));
    }

    // app with nested funcs, cutoff applies throughout
    // (f(l(2)) f(l(4))), lift 3, cutoff 3
    {
        auto l_lhs = f(v(2));
        auto l_rhs = f(v(4));
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        l_app->lift(3, 3);

        // Expected: (f(l(2)) f(l(7)))
        // l(2) < 3, not lifted
        // l(4) >= 3, lifted to 7
        auto l_expected = a(f(v(2)), f(v(7)));
        assert(l_app->equals(l_expected));
    }

    // app with complex nested structure, various cutoffs
    // ((1 6) (f(3) f(8))), lift 2, cutoff 5
    {
        auto l_func_left = a(v(1), v(6));
        auto l_func_right = a(f(v(3)), f(v(8)));
        auto l_app = a(l_func_left->clone(), l_func_right->clone());
        l_app->lift(2, 5);

        // Expected: ((1 8) (f(3) f(10)))
        // l(1) < 5, not lifted
        // l(6) >= 5, lifted to 8
        // l(3) < 5, not lifted
        // l(8) >= 5, lifted to 10
        auto l_expected = a(a(v(1), v(8)), a(f(v(3)), f(v(10))));
        assert(l_app->equals(l_expected));
    }

    // app with high cutoff, nothing gets lifted
    // (3 4 5), lift 10, cutoff 20
    {
        auto l_app = a(a(v(3), v(4)), v(5));
        l_app->lift(10, 20);

        // All < 20, none lifted
        auto l_expected = a(a(v(3), v(4)), v(5));
        assert(l_app->equals(l_expected));
    }

    // app with cutoff 0, everything gets lifted
    // (0 1 2), lift 5, cutoff 0
    {
        auto l_app = a(a(v(0), v(1)), v(2));
        l_app->lift(5, 0);

        // All >= 0, all lifted by 5
        auto l_expected = a(a(v(5), v(6)), v(7));
        assert(l_app->equals(l_expected));
    }
}

void test_var_substitute()
{
    // index 0, occurrance depth 0, substitute with a local
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        substitute(l_var, 0, 0, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);
        assert(l_substituted_var->m_index == 1);
        assert(l_var->m_size == 1);
    }

    // index 0, occurrance depth 10, substitute with a local
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        substitute(l_var, 10, 0, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);
        // it would be 1 if occurrance depth == 0, but since 10,
        //     l_substitute had to be lifted.
        assert(l_substituted_var->m_index == 11);
        assert(l_var->m_size == 1);
    }

    // index 2, occurrance depth 0, substitute with a local
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        substitute(l_var, 0, 0, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 1);
        assert(l_var->m_size == 1);
    }

    // index 1, occurrance depth 0, substitute with a local
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        substitute(l_var, 0, 0, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 0);
        assert(l_var->m_size == 1);
    }

    // index 2, occurrance depth 10, substitute with a local
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        substitute(l_var, 10, 0, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 1);
        assert(l_var->m_size == 1);
    }

    // index 1, occurrance depth 10, substitute with a local
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        substitute(l_var, 10, 0, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution decrements the lhs local index since the lhs does
        // not have var(0). var(0) is the only one that ever gets replaced due
        // to beta-reduction always first removing the outermost binder, and we
        // are using debruijn levels, which the outermost binder associates with
        // var(0). If the lhs has local vars with greater indices, then they
        // must have been defined inside the redex, so they are now 1 level
        // shallower.
        assert(l_substituted_var->m_index == 0);
        assert(l_var->m_size == 1);
    }

    // index 0, occurrance depth 0, substitute with a local, a_var_index 1
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        substitute(l_var, 0, 1, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);
        assert(l_substituted_var->m_index == 0);
        assert(l_var->m_size == 1);
    }

    // index 0, occurrance depth 10, substitute with a local, a_var_index 1
    {
        auto l_var = v(0);
        auto l_sub = v(1);
        substitute(l_var, 10, 1, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);
        // it would be 10 if a_var_index == 0, but since 1,
        //     not only were there no occurrances, but the
        //     var(0) was bound before cutoff (a_var_index == 1)
        //     so no lifting occurred.
        assert(l_substituted_var->m_index == 0);
        assert(l_var->m_size == 1);
    }

    // index 2, occurrance depth 0, substitute with a local, a_var_index 2
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        substitute(l_var, 0, 2, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution trivially finds var(2) and substitutes with
        // var(3), without lifting as the redex body has no binders.
        assert(l_substituted_var->m_index == 3);
        assert(l_var->m_size == 1);
    }

    // index 1, occurrance depth 0, substitute with a local, a_var_index 2
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        substitute(l_var, 0, 2, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution does not find var(2),
        // but var(1) is left alone since it was bound before cutoff
        // (a_var_index == 2)
        assert(l_substituted_var->m_index == 1);
        assert(l_var->m_size == 1);
    }

    // index 2, occurrance depth 10, substitute with a local, a_var_index 2
    {
        auto l_var = v(2);
        auto l_sub = v(3);
        substitute(l_var, 10, 2, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // this substitution finds var(2) and substitutes with var(3),
        // and lifts by 10 levels since the redex body has 10 binders.
        assert(l_substituted_var->m_index == 13);
        assert(l_var->m_size == 1);
    }

    // index 1, occurrance depth 10, substitute with a local, a_var_index 2
    {
        auto l_var = v(1);
        auto l_sub = v(3);
        substitute(l_var, 10, 2, l_sub);

        const var* l_substituted_var = dynamic_cast<var*>(l_var.get());
        assert(l_substituted_var != nullptr);

        // no var(2) was found, so var(1) is left alone since it was bound
        // before cutoff (a_var_index == 2)
        assert(l_substituted_var->m_index == 1);
        assert(l_var->m_size == 1);
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

        auto l_subbed = l_func->clone();
        substitute(l_subbed, 0, 0, l_var);

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the index of the substitute is lifted by 1 (one binder)
        assert(l_subbed_var->m_index == 12);
        assert(l_subbed->m_size == 2);
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

        auto l_subbed = l_func->clone();
        substitute(l_subbed, 0, 0, l_var);

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->m_body.get());

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func_2->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the index of the substitute is lifted by 2 (two binders)
        assert(l_subbed_var->m_index == 13);
        assert(l_subbed->m_size == 3);
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

        auto l_subbed = l_func->clone();
        substitute(l_subbed, 0, 1, l_var);

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the local is unchanged as it was not replaced and it was bound
        // before cutoff (a_var_index == 1) so no lifting occurred.
        assert(l_subbed_var->m_index == 0);
        assert(l_subbed->m_size == 2);
    }

    // double composition lambda, outer depth 0, occurrance not found,
    // a_var_index 1
    {
        auto l_func = f(f(v(0)->clone())->clone());
        auto l_var = v(11);

        auto l_subbed = l_func->clone();
        substitute(l_subbed, 0, 1, l_var);

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());

        // make sure still a function
        assert(l_subbed_func != nullptr);

        const func* l_subbed_func_2 =
            dynamic_cast<func*>(l_subbed_func->m_body.get());

        // get body
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func_2->m_body.get());

        // make sure the substitution took place
        assert(l_subbed_var != nullptr);

        // the local is unchanged as it was not replaced and it was bound
        // before cutoff (a_var_index == 1) so no lifting occurred.
        assert(l_subbed_var->m_index == 0);
        assert(l_subbed->m_size == 3);
    }

    // func with occurrence in its body, a_lift_amount > 0
    {
        // Create a function whose body contains a local with index 2 (is the
        // occurrence)
        auto l_func = f(v(2)->clone());
        auto l_sub = v(7);

        // substitute at depth = 6 (5 + 1), var_index=2, so a_lift_amount=6
        auto l_subbed = l_func->clone();
        substitute(l_subbed, 5, 2, l_sub);

        const func* l_subbed_func = dynamic_cast<func*>(l_subbed.get());
        assert(l_subbed_func != nullptr);

        // get body, should be a local with index = 7 + 6 = 13
        const var* l_subbed_var =
            dynamic_cast<var*>(l_subbed_func->m_body.get());
        assert(l_subbed_var != nullptr);
        assert(l_subbed_var->m_index == 13);
        assert(l_subbed->m_size == 2);
    }

    // func with app body containing mixed locals, var_index=3
    // Tests that locals < var_index are left alone
    {
        // Body: (0 1 2 3 4) - mix of locals below, at, and above var_index=3
        auto l_body = a(a(a(a(v(0), v(1)), v(2)), v(3)), v(4));
        auto l_func = f(l_body->clone());
        auto l_sub = v(99);

        // substitute var_index=3 with l(99) at depth 0
        auto l_subbed = l_func->clone();
        substitute(l_subbed, 0, 3, l_sub);

        // Expected: (0 1 2 100 3)
        // - l(0), l(1), l(2) stay unchanged (< 3)
        // - l(3) gets replaced with l(99) lifted by 1 (func binder) = l(100)
        // - l(4) decrements to l(3) (> 3)
        auto l_expected =
            f(a(a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()),
                  v(100)->clone()),
                v(3)->clone()));

        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 10);
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
        auto l_subbed = l_func->clone();
        substitute(l_subbed, 0, 2, l_sub);

        // Expected: λ.λ.(0 89 2)
        // - Inner l(0) unchanged (bound by inner lambda)
        // - l(2) at depth 2 (due to 2 binders) should match var 2
        //   and be replaced with l(88) lifted by 2 = l(90)
        // - l(3) decrements to l(2)
        auto l_expected =
            f(f(a(a(v(0)->clone(), v(90)->clone()), v(2)->clone())));

        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 7);
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
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->m_lhs.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_rhs.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 11);
        assert(l_subbed->m_size == 3);
    }

    // app of locals, lhs is an occurrance
    {
        auto l_lhs = v(0);
        auto l_rhs = v(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->m_lhs.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_rhs.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 11);
        assert(l_subbed_rhs->m_index == 0);
        assert(l_subbed->m_size == 3);
    }

    // app of locals, rhs is an occurrance
    {
        auto l_lhs = v(1);
        auto l_rhs = v(0);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->m_lhs.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_rhs.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 11);
        assert(l_subbed->m_size == 3);
    }

    // app of locals, neither are occurrances
    {
        auto l_lhs = v(1);
        auto l_rhs = v(1);
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const var* l_subbed_lhs = dynamic_cast<var*>(l_subbed_app->m_lhs.get());

        // make sure lhs is a local
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const var* l_subbed_rhs = dynamic_cast<var*>(l_subbed_app->m_rhs.get());

        // make sure rhs is a local
        assert(l_subbed_rhs != nullptr);

        // make sure they have correct indices
        assert(l_subbed_lhs->m_index == 0);
        assert(l_subbed_rhs->m_index == 0);
        assert(l_subbed->m_size == 3);
    }

    // app of funcs, both with occurrances
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = f(v(0)->clone());
        auto l_app = a(l_lhs->clone(), l_rhs->clone());
        auto l_sub = v(11);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);

        // get the outer app
        const app* l_subbed_app = dynamic_cast<app*>(l_subbed.get());

        // make sure outer binder is an app
        assert(l_subbed_app != nullptr);

        // get lhs
        const func* l_subbed_lhs =
            dynamic_cast<func*>(l_subbed_app->m_lhs.get());

        // make sure lhs is a func
        assert(l_subbed_lhs != nullptr);

        // get rhs
        const func* l_subbed_rhs =
            dynamic_cast<func*>(l_subbed_app->m_rhs.get());

        // make sure rhs is a func
        assert(l_subbed_rhs != nullptr);

        const var* l_lhs_var = dynamic_cast<var*>(l_subbed_lhs->m_body.get());

        // make sure body of lhs is a local
        assert(l_lhs_var != nullptr);

        const var* l_rhs_var = dynamic_cast<var*>(l_subbed_rhs->m_body.get());

        // make sure body of rhs is a local
        assert(l_rhs_var != nullptr);

        // make sure they have correct indices (lifted by 1 due to binders)
        assert(l_lhs_var->m_index == 12);
        assert(l_rhs_var->m_index == 12);
        assert(l_subbed->m_size == 5);
    }

    ////////////////////////////////////
    // Testing substitute with various binder depths
    ////////////////////////////////////

    // Test 1: substitute at depth 0, var_index 0 - basic substitution
    // (0 0) with var 0 -> l(5) should give (5 5)
    {
        auto l_app = a(v(0)->clone(), v(0)->clone());
        auto l_sub = v(5);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);
        auto l_expected = a(v(5)->clone(), v(5)->clone());
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 3); // 1 + 1 + 1
    }

    // Test 2: substitute at depth 0, var_index 1 - substituting higher var
    // (1 2) with var 1 -> l(7) should give (7 1)
    {
        auto l_app = a(v(1)->clone(), v(2)->clone());
        auto l_sub = v(7);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 1, l_sub);
        auto l_expected = a(v(7)->clone(), v(1)->clone());
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 3); // 1 + 1 + 1
    }

    // Test 3: substitute at depth 1, var_index 0 - inside a binder context
    // (λ.0 λ.1) with var 0 at depth 1 -> l(3) should give (λ.5 λ.0)
    {
        auto l_app = a(f(v(0)->clone()), f(v(1)->clone()));
        auto l_sub = v(3);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 1, 0, l_sub);
        auto l_expected = a(f(v(5)->clone()), f(v(0)->clone()));
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 5); // 1 + (1+1) + (1+1)
    }

    // Test 5: substitute with complex expression (app as substitution)
    // (0 1) with var 0 -> (2 3) should give ((2 3) 0)
    {
        auto l_app = a(v(0)->clone(), v(1)->clone());
        auto l_sub = a(v(2)->clone(), v(3)->clone());
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);
        auto l_expected = a(a(v(2)->clone(), v(3)->clone()), v(0)->clone());
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 5); // 1 + (1+1+1) + 1
    }

    // Test 6: substitute with func as substitution
    // (0 0) with var 0 -> λ.5 should give (λ.5 λ.5)
    {
        auto l_app = a(v(0)->clone(), v(0)->clone());
        auto l_sub = f(v(5)->clone());
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);
        auto l_expected = a(f(v(5)->clone()), f(v(5)->clone()));
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 5); // 1 + (1+1) + (1+1)
    }

    // Test 7: substitute at depth 2, var_index 1 - deeply nested
    // (λ.λ.1 λ.λ.2) with var 1 at depth 2 -> l(10) should give (λ.λ.14 λ.λ.1)
    {
        auto l_app = a(f(f(v(1)->clone())), f(f(v(2)->clone())));
        auto l_sub = v(10);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 2, 1, l_sub);
        auto l_expected = a(f(f(v(14)->clone())), f(f(v(1)->clone())));
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 7); // 1 + (1+(1+1)) + (1+(1+1))
    }

    // Test 8: no matching variable - all vars higher than target
    // (2 3) with var 0 -> l(99) should give (1 2)
    {
        auto l_app = a(v(2)->clone(), v(3)->clone());
        auto l_sub = v(99);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);
        auto l_expected = a(v(1)->clone(), v(2)->clone());
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 3); // 1 + 1 + 1
    }

    // Test 9: no matching variable - all vars lower than target
    // (0 1) with var 5 -> l(99) should give (0 1)
    {
        auto l_app = a(v(0)->clone(), v(1)->clone());
        auto l_sub = v(99);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 5, l_sub);
        auto l_expected = a(v(0)->clone(), v(1)->clone());
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 3); // 1 + 1 + 1
    }

    // Test 10: nested app with mixed locals
    // ((0 1) (2 0)) with var 0 -> l(8) should give ((8 0) (1 8))
    {
        auto l_inner1 = a(v(0)->clone(), v(1)->clone());
        auto l_inner2 = a(v(2)->clone(), v(0)->clone());
        auto l_app = a(l_inner1->clone(), l_inner2->clone());
        auto l_sub = v(8);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 0, l_sub);
        auto l_expected =
            a(a(v(8)->clone(), v(0)->clone()), a(v(1)->clone(), v(8)->clone()));
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 7); // 1 + (1+1+1) + (1+1+1)
    }

    // Test 11: substitute higher var with lower vars present
    // (0 2) with var 2 -> l(9) should give (0 9)
    // l(0) stays unchanged (< 2), l(2) gets replaced
    {
        auto l_app = a(v(0)->clone(), v(2)->clone());
        auto l_sub = v(9);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 2, l_sub);
        auto l_expected = a(v(0)->clone(), v(9)->clone());
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size == 3); // 1 + 1 + 1
    }

    // Test 12: substitute with var_index=4, multiple locals below and above
    // ((0 1 2 3) (4 5 6)) with var 4 -> l(77) should give ((0 1 2 3) (77 4 5))
    {
        auto l_func_app =
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone());
        auto l_arg_app = a(a(v(4)->clone(), v(5)->clone()), v(6)->clone());
        auto l_app = a(l_func_app->clone(), l_arg_app->clone());
        auto l_sub = v(77);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 4, l_sub);

        // Expected: ((0 1 2 3) (77 4 5))
        // l(0), l(1), l(2), l(3) unchanged (< 4)
        // l(4) replaced with l(77)
        // l(5), l(6) decremented to l(4), l(5)
        auto l_expected = a(
            a(a(a(v(0)->clone(), v(1)->clone()), v(2)->clone()), v(3)->clone()),
            a(a(v(77)->clone(), v(4)->clone()), v(5)->clone()));
        assert(l_subbed->equals(l_expected));
        assert(l_subbed->m_size ==
               13); // 1 + (1+(1+(1+1+1)+1)+1) + (1+(1+1+1)+1)
    }

    // Test 13: nested app with funcs, var_index=2
    // (λ.(0 1 2) λ.(1 2 3)) with var 2 -> l(55)
    {
        auto l_lhs_body = a(a(v(0)->clone(), v(1)->clone()), v(2)->clone());
        auto l_rhs_body = a(a(v(1)->clone(), v(2)->clone()), v(3)->clone());
        auto l_app = a(f(l_lhs_body->clone()), f(l_rhs_body->clone()));
        auto l_sub = v(55);
        auto l_subbed = l_app->clone();
        substitute(l_subbed, 0, 2, l_sub);

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
        assert(l_subbed->m_size ==
               13); // 1 + (1+(1+(1+1+1)+1)) + (1+(1+(1+1+1)+1))
    }
}

void test_var_reduce()
{
    // local with var 0
    {
        auto l_expr = v(0);
        auto l_result = l_expr->clone();

        // reduce
        assert(!reduce_one_step(l_result));

        // cast the pointer
        const var* l_var = dynamic_cast<var*>(l_result.get());
        assert(l_var != nullptr);

        // make sure it has the same index
        assert(l_var->m_index == 0);
        assert(l_result->m_size == 1);
    }

    // local with var 1
    {
        auto l_expr = v(1);
        auto l_result = l_expr->clone();

        // reduce
        assert(!reduce_one_step(l_result));

        // cast the pointer
        const var* l_var = dynamic_cast<var*>(l_result.get());
        assert(l_var != nullptr);

        // make sure it has the same index
        assert(l_var->m_index == 1);
        assert(l_result->m_size == 1);
    }

    // var at depth 5 - cannot reduce
    {
        auto l_var = v(3);
        auto l_reduced = reduce_one_step(l_var, 5);
        assert(!l_reduced);
    }

    // var at depth 10 - cannot reduce
    {
        auto l_var = v(7);
        auto l_reduced = reduce_one_step(l_var, 10);
        assert(!l_reduced);
    }
}

void test_func_reduce()
{
    // func with body of a local
    {
        auto l_expr = f(v(0)->clone());
        auto l_result = l_expr->clone();

        // no reductions to perform
        assert(!reduce_one_step(l_result));

        // make sure still a func
        const auto* l_func = dynamic_cast<func*>(l_result.get());
        assert(l_func != nullptr);

        // get body
        const auto* l_body = dynamic_cast<var*>(l_func->m_body.get());
        assert(l_body != nullptr);

        // make sure body is still same thing
        assert(l_body->m_index == 0);
        assert(l_result->m_size == 2);
    }

    // Test reduction of func with no redex
    {
        auto l_expr = f(a(v(2), v(5)));
        auto l_result = l_expr->clone();
        assert(!reduce_one_step(l_result));
        assert(l_result->equals(f(a(v(2), v(5)))));
        assert(l_result->m_size == 4);
    }

    // Test reduction count with redex inside function body
    // λ.((λ.0) 5) -> λ.0 (1 reduction inside body)
    // Note: v(0) at depth 1 refers to outer lambda, so it's not replaced
    {
        auto l_expr = f(a(f(v(0)), v(5)));
        auto l_result = l_expr->clone();

        size_t l_step_count = 0;

        while(reduce_one_step(l_result))
            ++l_step_count;

        assert(l_step_count == 1);

        assert(l_result->equals(f(v(0))));
        assert(l_result->m_size == 2);
    }

    // Test nested functions with multiple reductions
    // λ.λ.((λ.2) ((λ.3) 5)) -> λ.λ.((λ.3) 5) -> λ.λ.2 (2 reductions)
    {
        auto l_expr = f(f(a(f(v(2)), a(f(v(3)), v(5)))));
        auto l_result = l_expr->clone();

        size_t l_step_count = 0;

        while(reduce_one_step(l_result))
            ++l_step_count;

        assert(l_step_count == 2);

        assert(l_result->equals(f(f(v(2)))));
        assert(l_result->m_size == 3);
    }

    // Test size peak tracking on func with reduction where size decreases
    // λ.((λ.0) 5) has size 5, reduces to λ.0 with size 2
    // Peak should be 2 (size after reduction)
    {
        auto l_expr = f(a(f(v(0)), v(5)));
        auto l_result = l_expr->clone();

        size_t l_size_peak = std::numeric_limits<size_t>::min();

        while(reduce_one_step(l_result))
            l_size_peak = std::max(l_size_peak, l_result->m_size);

        assert(l_size_peak == 2);
        assert(l_result->equals(f(v(0))));
        assert(l_result->m_size == 2);
    }

    // Test simple reduction inside lambda body
    {
        auto l_expr = f(a(f(v(1)), v(2)));
        auto l_result = l_expr->clone();

        size_t l_step_count = 0;

        while(reduce_one_step(l_result))
            ++l_step_count;

        assert(l_step_count == 1);
        assert(l_result->equals(f(v(2))));
        assert(l_result->m_size == 2);
    }

    // Test trace callback on func with 1 reduction
    // λ.((λ.0) 5) -> λ.0
    // Expected trace: [λ.((λ.0) 5), λ.0]
    {
        auto l_expr = f(a(f(v(0)), v(5)));

        // Expected trace: initial expression + result after 1 reduction
        std::vector<std::unique_ptr<expr>> l_expected_trace;
        l_expected_trace.push_back(f(v(0)));

        size_t l_trace_index = 0;
        auto l_trace = [&](const std::unique_ptr<expr>& a_expr)
        {
            assert(l_trace_index < l_expected_trace.size());
            assert(a_expr->equals(l_expected_trace[l_trace_index]));
            ++l_trace_index;
        };

        auto l_result = l_expr->clone();
        while(reduce_one_step(l_result))
            l_trace(l_result);

        assert(l_trace_index == l_expected_trace.size());
        assert(l_result->equals(f(v(0))));
        assert(l_result->m_size == 2);
    }

    // Test trace callback on func with 2 reductions
    // λ.λ.((λ.2) ((λ.3) 5)) -> λ.λ.((λ.3) 5) -> λ.λ.2
    // Expected trace: [initial, after 1st reduction, after 2nd reduction]
    {
        auto l_expr = f(f(a(f(v(2)), a(f(v(3)), v(5)))));

        // Expected trace: initial + intermediate + final
        std::vector<std::unique_ptr<expr>> l_expected_trace;
        l_expected_trace.push_back(f(f(a(f(v(3)), v(5)))));
        l_expected_trace.push_back(f(f(v(2))));

        size_t l_trace_index = 0;
        auto l_trace = [&](const std::unique_ptr<expr>& a_expr)
        {
            assert(l_trace_index < l_expected_trace.size());
            assert(a_expr->equals(l_expected_trace[l_trace_index]));
            ++l_trace_index;
        };

        auto l_result = l_expr->clone();

        while(reduce_one_step(l_result))
            l_trace(l_result);

        assert(l_trace_index == l_expected_trace.size());
        assert(l_result->equals(f(f(v(2)))));
        assert(l_result->m_size == 3);
    }

    // Test trace callback on func with 2 reductions
    // λ.λ.((λ.2) ((λ.2) 5)) -> λ.λ.((λ.2) 5) -> λ.λ.5
    // Expected trace: [initial, after 1st reduction, after 2nd reduction]
    {
        auto l_expr = f(f(a(f(v(2)), a(f(v(2)), v(5)))));

        // Expected trace: initial + intermediate + final
        std::vector<std::unique_ptr<expr>> l_expected_trace;
        l_expected_trace.push_back(f(f(a(f(v(2)), v(5)))));
        l_expected_trace.push_back(f(f(v(5))));

        size_t l_trace_index = 0;
        auto l_trace = [&](const std::unique_ptr<expr>& a_expr)
        {
            assert(l_trace_index < l_expected_trace.size());
            assert(a_expr->equals(l_expected_trace[l_trace_index]));
            ++l_trace_index;
        };

        auto l_result = l_expr->clone();

        while(reduce_one_step(l_result))
            l_trace(l_result);

        assert(l_trace_index == l_expected_trace.size());
        assert(l_result->equals(f(f(v(5)))));
        assert(l_result->m_size == 3);
    }

    // func with beta-redex in body at depth 3 - inner func refs outer context
    // λ.((λ.3) 2) -> λ.3
    // Note: At depth 3, inner lambda is at depth 4, v(3) refs depth 3 (outer
    // lambda)
    {
        auto l_func = f(a(f(v(3)), v(2)));
        auto l_reduced = reduce_one_step(l_func, 3);
        assert(l_reduced);
        assert(l_func->equals(f(v(3))));
        assert(l_func->m_size == 2);

        assert(!reduce_one_step(l_func));
        assert(l_func->equals(f(v(3))));
        assert(l_func->m_size == 2);
    }
}

void test_app_reduce()
{
    // app with lhs and rhs both locals
    {
        auto l_lhs = v(0);
        auto l_rhs = v(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(!reduce_one_step(l_result));

        const app* l_app = dynamic_cast<app*>(l_result.get());
        assert(l_app != nullptr);

        // lhs should be local
        const var* l_reduced_lhs = dynamic_cast<var*>(l_app->m_lhs.get());
        assert(l_reduced_lhs != nullptr);

        // rhs should be local
        const var* l_reduced_rhs = dynamic_cast<var*>(l_app->m_rhs.get());
        assert(l_reduced_rhs != nullptr);

        // same on both (no reduction occurred)
        assert(l_reduced_lhs->m_index == 0);
        assert(l_reduced_rhs->m_index == 1);

        assert(l_result->m_size == 3);
    }

    // app with lhs func and rhs local
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = v(1);
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // make sure nothing changed
        // (beta reduction did not occur since rhs is local)
        assert(l_result->equals(v(1)));

        assert(l_result->m_size == 1);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 1);
    }

    // app with lhs func and rhs func
    {
        auto l_lhs = f(v(0)->clone());
        auto l_rhs = f(v(1)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // make sure beta-reduction occurred, with no lifting of indices
        assert(l_result->equals(l_rhs->clone()));
        assert(l_result->m_size == 2);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 2);
    }

    // app with lhs func (without occurrences of var 0) and rhs func
    {
        auto l_lhs = f(v(3)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // make sure beta-reduction occurred, but no replacements.
        // other vars decremented by 1.
        assert(l_result->equals(v(2)->clone()));
        assert(l_result->m_size == 1);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 1);
    }

    // app with lhs (nested func with occurrences of var 0) and rhs func
    {
        auto l_lhs = f(f(v(0)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // make sure beta-reduction occurred, with replacement,
        // and a lifting of 1 level
        assert(l_result->equals(f(f(v(6)->clone()))->clone()));
        assert(l_result->m_size == 3);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 3);
    }

    // app with lhs (nested func without occurrences of var 0) and rhs func
    {
        auto l_lhs = f(f(v(3)->clone())->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // make sure beta-reduction occurred, no replacements.
        // other vars decremented by 1.
        assert(l_result->equals(f(v(2))));
        assert(l_result->m_size == 2);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 2);
    }

    // app with lhs (app that doesnt reduce to func) and rhs func
    {
        auto l_lhs = a(v(3)->clone(), v(4)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(!reduce_one_step(l_result));

        // make sure nothing changed
        // (both lhs and rhs were fully reduced already)
        assert(l_result->equals(l_expr->clone()));
        assert(l_result->m_size == 6);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 6);
    }

    // app with lhs (app with lhs (func without occurrances), rhs local)
    // and rhs func
    {
        auto l_lhs = a(f(v(3)->clone())->clone(), v(4)->clone());
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // lhs should have beta-reduced, but cannot consume rhs of app
        assert(l_result->equals(a(v(2), f(v(5)))));
        assert(l_result->m_size == 4);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 4);
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are too many arguments supplied
    {
        auto l_lhs = a(f(v(3)), f(v(4)));
        auto l_rhs = f(v(5)->clone());
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // lhs of app should beta-reduce, but lhs is not capable of consuming 2
        // args. Thus NF is an application with LHS beta-reduced once.
        assert(l_result->equals(a(v(2), f(v(5)))));
        assert(l_result->m_size == 4);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 4);
    }

    // app with lhs (app with lhs (func without occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs = a(f(f(v(3))), f(v(4)));
        auto l_rhs = f(v(5));
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        assert(l_result->equals(a(f(v(2)), f(v(5)))));
        assert(l_result->m_size == 5);

        // reduce again!
        assert(reduce_one_step(l_result));

        // should beta-reduce twice, consuming all args. No replacements, only
        // decrementing twice.
        assert(l_result->equals(v(1)));
        assert(l_result->m_size == 1);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 1);
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs = a(f(f(v(0))), f(v(4)));
        auto l_rhs = f(v(5));
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        assert(l_result->equals(a(f(f(v(5))), f(v(5)))));
        assert(l_result->m_size == 6);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(f(v(4))));
        assert(l_result->m_size == 2);

        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 2);
    }

    // app with lhs (app with lhs (func WITH occurrances), rhs func)
    // and rhs func, where there are correct number of args supplied.
    {
        auto l_lhs = a(f(f(v(1))), f(v(4)));
        auto l_rhs = f(v(5));
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());

        // reduce the app

        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        assert(l_result->equals(a(f(v(0)), f(v(5)))));
        assert(l_result->m_size == 5);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(f(v(5))));
        assert(l_result->m_size == 2);

        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 2);
    }

    // Test reduction count with single reduction
    // (λ.0) 5 -> 5 (1 reduction)
    {
        auto l_expr = a(f(v(0)), v(5));
        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));
        assert(l_result->equals(v(5)));
        assert(l_result->m_size == 1);

        assert(!reduce_one_step(l_result));
        assert(l_result->m_size == 1);
    }

    // Test reduction count with multiple reductions
    // ((λ.0) 5) ((λ.1) 6) -> (5 ((λ.1) 6)) -> (5 0) (2 reductions)
    // Note: v(1) is a free variable, so it decrements to v(0)
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));

        // First reduction: left-outermost redex reduces
        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));

        // After 1st reduction: (5 ((λ.1) 6))
        assert(l_result->equals(a(v(5), a(f(v(1)), v(6)))));
        assert(l_result->m_size == 6); // 1 + 1 + (1+2+1)

        // Second reduction: right side reduces
        assert(reduce_one_step(l_result));

        // After 2nd reduction: (5 0)
        assert(l_result->equals(a(v(5), v(0))));
        assert(l_result->m_size == 3); // 1 + 1 + 1

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(v(5), v(0))));
        assert(l_result->m_size == 3); // 1 + 1 + 1
    }

    // Test reduction count with church numeral reduction
    // Church numeral 2 applied: many reductions
    // ((λ.λ.(0 (0 1))) (λ.10)) 5
    {
        auto l_two = f(f(a(v(0), a(v(0), v(1)))));
        auto l_f = f(v(10));
        auto l_x = v(5);
        auto l_expr = a(a(l_two->clone(), l_f->clone()), l_x->clone());

        auto l_result = l_expr->clone();

        // First reduction: apply first argument (λ.10) to church 2
        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(f(a(f(v(11)), a(f(v(11)), v(0)))), v(5))));
        assert(l_result->m_size == 10);

        // Second reduction: apply second argument 5
        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(f(v(10)), a(f(v(10)), v(5)))));
        assert(l_result->m_size == 7);

        // Third reduction: reduce outer application
        assert(reduce_one_step(l_result));
        assert(l_result->equals(v(9)));
        assert(l_result->m_size == 1);

        // beta-normal
        assert(!reduce_one_step(l_result));
        assert(l_result->equals(v(9)));
        assert(l_result->m_size == 1);
    }

    // Test application in beta-normal form - no reduction possible
    {
        auto l_expr = a(v(3), v(7));
        auto l_result = l_expr->clone();

        // Already in beta-normal form - cannot reduce
        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(v(3), v(7))));
        assert(l_result->m_size == 3); // 1 + 1 + 1
    }

    // ((λ.0) 5) ((λ.1) 6) needs 2 reductions total
    {
        auto l_expr = a(a(f(v(0)), v(5)), a(f(v(1)), v(6)));
        auto l_result = l_expr->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(5), a(f(v(1)), v(6)))));
        assert(l_result->m_size == 6);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(5), v(0))));
        assert(l_result->m_size == 3);

        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(v(5), v(0))));
        assert(l_result->m_size == 3);
    }

    // Test complex reduction with limit
    // ((λ.λ.0) 5) 6 -> (λ.6) 6 -> 5 (2 reductions total)
    // Note: v(6) in body is a free variable, so it decrements to v(5)
    {
        auto l_expr = a(a(f(f(v(0))), v(5)), v(6));
        assert(l_expr->m_size == 7);

        assert(reduce_one_step(l_expr));
        assert(l_expr->equals(a(f(v(6)), v(6))));
        assert(l_expr->m_size == 4);

        assert(reduce_one_step(l_expr));
        assert(l_expr->equals(v(5)));
        assert(l_expr->m_size == 1);

        assert(!reduce_one_step(l_expr));
        assert(l_expr->equals(v(5)));
        assert(l_expr->m_size == 1);
    }

    // Test identity combinator: (λ.0) applied to expression
    // (λ.0) (λ.5) -> λ.5 (1 reduction)
    {
        auto l_identity = f(v(0));
        auto l_arg = f(v(5));
        auto l_expr = a(l_identity->clone(), l_arg->clone());
        auto l_result = l_expr->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(f(v(5))));
        assert(l_result->m_size == 2);

        assert(!reduce_one_step(l_result));
        assert(l_result->equals(f(v(5))));
        assert(l_result->m_size == 2);
    }

    // Test normal order
    // ((λ.2) 3) ((λ.4) 5) needs 2 reductions
    {
        auto l_lhs = a(f(v(2)), v(3));
        auto l_rhs = a(f(v(4)), v(5));
        auto l_expr = a(l_lhs->clone(), l_rhs->clone());
        auto l_result = l_expr->clone();
        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), a(f(v(4)), v(5)))));
        assert(l_result->m_size == 6);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(3))));
        assert(l_result->m_size == 3);

        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(3))));
        assert(l_result->m_size == 3);
    }

    // Test self-application with limit to prevent infinite reduction
    // (λ.0 0) (λ.0 0) with limit 2 (omega combinator)
    {
        auto l_omega = f(a(v(0), v(0)));
        auto l_expr = a(l_omega->clone(), l_omega->clone());
        auto l_result = l_expr->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(f(a(v(0), v(0))), f(a(v(0), v(0))))));
        assert(l_result->m_size == 9);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(f(a(v(0), v(0))), f(a(v(0), v(0))))));
        assert(l_result->m_size == 9);
    }

    // Test reduction inside nested applications
    // (1 ((λ.0) 2)) -> (1 2) (1 reduction in rhs)
    {
        auto l_expr = a(v(1), a(f(v(0)), v(2)));
        auto l_result = l_expr->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(2))));
        assert(l_result->m_size == 3);

        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(2))));
        assert(l_result->m_size == 3);
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

        auto l_result = l_expr->clone();
        assert(l_result->m_size == 11);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(f(a(a(v(0), v(0)), v(0))), f(a(a(v(0), v(0)), v(0))))));
        assert(l_result->m_size == 13);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(a(l_dup3->clone(), l_dup3->clone()), l_dup3->clone())));
        assert(l_result->m_size == 20);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(a(a(l_dup3->clone(), l_dup3->clone()), l_dup3->clone()),
              l_dup3->clone())));
        assert(l_result->m_size == 27);
    }

    // 3-step expression
    // Expression: (λ.0) (((λ.2) 3) ((λ.4) 5))
    {
        auto l_inner =
            a(a(f(v(2)), v(3)), a(f(v(4)), v(5)));  // ((λ.2) 3) ((λ.4) 5)
        auto l_expr = a(f(v(0)), l_inner->clone()); // (λ.0) (...)
        auto l_result = l_expr->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(l_inner));
        assert(l_result->m_size == l_inner->m_size);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), a(f(v(4)), v(5)))));
        assert(l_result->m_size == 6);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(3))));
        assert(l_result->m_size == 3);
    }

    // 4-step expression
    // Expression: ((λ.0) (λ.0)) (((λ.2) 3) ((λ.4) 5))
    {
        auto l_inner =
            a(a(f(v(2)), v(3)), a(f(v(4)), v(5))); // ((λ.2) 3) ((λ.4) 5)
        auto l_wrapped =
            a(a(f(v(0)), f(v(0))), l_inner->clone()); // ((λ.0) (λ.0)) (...)
        auto l_result = l_wrapped->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(f(v(0)), a(a(f(v(2)), v(3)), a(f(v(4)), v(5))))));
        assert(l_result->m_size == 12);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(a(f(v(2)), v(3)), a(f(v(4)), v(5)))));
        assert(l_result->m_size == 9);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), a(f(v(4)), v(5)))));
        assert(l_result->m_size == 6);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(3))));
        assert(l_result->m_size == 3);

        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(v(1), v(3))));
        assert(l_result->m_size == 3);
    }

    // Expression that needs 5 steps
    // Expression: ((((λ.0) 1) ((λ.0) 2)) ((λ.0) 3)) ((λ.0) 4)) ((λ.0) 5)
    {
        auto l_r1 = a(f(v(0)), v(1)); // (λ.0) 1
        auto l_r2 = a(f(v(0)), v(2)); // (λ.0) 2
        auto l_r3 = a(f(v(0)), v(3)); // (λ.0) 3
        auto l_r4 = a(f(v(0)), v(4)); // (λ.0) 4
        auto l_r5 = a(f(v(0)), v(5)); // (λ.0) 5
        auto l_expr = a(
            a(a(a(l_r1->clone(), l_r2->clone()), l_r3->clone()), l_r4->clone()),
            l_r5->clone());

        auto l_result = l_expr->clone();

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(a(a(a(v(1), l_r2->clone()), l_r3->clone()), l_r4->clone()),
              l_r5->clone())));
        assert(l_result->m_size == 21);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(
            a(a(a(v(1), v(2)), l_r3->clone()), l_r4->clone()), l_r5->clone())));
        assert(l_result->m_size == 18);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(a(a(a(v(1), v(2)), v(3)), l_r4->clone()), l_r5->clone())));
        assert(l_result->m_size == 15);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(
            a(a(a(a(v(1), v(2)), v(3)), v(4)), l_r5->clone())));
        assert(l_result->m_size == 12);

        assert(reduce_one_step(l_result));
        assert(l_result->equals(a(a(a(a(v(1), v(2)), v(3)), v(4)), v(5))));
        assert(l_result->m_size == 9);

        assert(!reduce_one_step(l_result));
        assert(l_result->equals(a(a(a(a(v(1), v(2)), v(3)), v(4)), v(5))));
        assert(l_result->m_size == 9);
    }

    // func with beta-redex in body at depth 3 - inner func refs outer context
    // ((λ.3) 2) -> 2
    // Note: At depth 3, the var v(3) is referencing the binder in the lhs
    {
        auto l_func = a(f(v(3)), v(2));
        auto l_reduced = reduce_one_step(l_func, 3);
        assert(l_reduced);
        assert(l_func->equals(v(2)));
        assert(l_func->m_size == 1);

        assert(!reduce_one_step(l_func));
        assert(l_func->equals(v(2)));
        assert(l_func->m_size == 1);
    }
}

void construct_program_test()
{
    using namespace lambda;

    // Test 1: Empty helpers list
    {
        std::vector<std::unique_ptr<expr>> l_helpers{};
        auto l_main = v(5);
        auto l_program =
            construct_program(l_helpers.begin(), l_helpers.end(), l_main);

        size_t l_step_count = 0;
        while(reduce_one_step(l_program))
            ++l_step_count;

        assert(l_program->equals(v(5)));
        assert(l_step_count == 0);
    }

    // Test 2: Single constant helper
    {
        std::list<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: Returns v(100) regardless of input
        const auto CONST_100 = g(l_helpers.size());
        l_helpers.emplace_back(f(v(100)));

        // Main: Apply helper to dummy value
        auto l_main = a(CONST_100->clone(), l(0)->clone());

        auto l_program =
            construct_program(l_helpers.begin(), l_helpers.end(), l_main);

        while(reduce_one_step(l_program))
            ;

        // After norm, returns v(99) (100 - 1 from removing one binding level)
        assert(l_program->equals(v(99)));
    }

    // Test 3: Church booleans (TRUE and FALSE)
    {
        std::list<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: TRUE = λ.λ.0 (returns first arg)
        const auto TRUE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(0))));

        // Helper 1: FALSE = λ.λ.1 (returns second arg)
        const auto FALSE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // Test TRUE: ((TRUE branch_a) branch_b) → branch_a
        {
            auto l_main = a(a(TRUE->clone(), l(0)->clone()), l(1)->clone());
            auto l_program =
                construct_program(l_helpers.begin(), l_helpers.end(), l_main);

            while(reduce_one_step(l_program))
                ;

            // Returns first branch (after substitution and decrement)
            assert(l_program->equals(v(0)));
        }

        // Test FALSE: ((FALSE branch_a) branch_b) → branch_b
        {
            auto l_main = a(a(FALSE->clone(), l(0)->clone()), l(1)->clone());
            auto l_program =
                construct_program(l_helpers.begin(), l_helpers.end(), l_main);

            while(reduce_one_step(l_program))
                ;

            // Returns second branch (after substitution and decrement)
            assert(l_program->equals(v(1)));
        }
    }

    // Test 4: Helper depends on earlier helper
    {
        std::list<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: TRUE
        const auto TRUE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(0))));

        // Helper 1: FALSE
        const auto FALSE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // Helper 2: NOT = λ.((0 FALSE) TRUE)
        const auto NOT = g(l_helpers.size());
        l_helpers.emplace_back(
            f(a(a(l(0)->clone(), FALSE->clone()), TRUE->clone())));

        // Test: NOT TRUE → FALSE
        {
            auto l_main = a(NOT->clone(), TRUE->clone());
            auto l_program =
                construct_program(l_helpers.begin(), l_helpers.end(), l_main);

            while(reduce_one_step(l_program))
                ;

            // Result should be FALSE = λ.λ.1
            auto l_expected = f(f(v(1)));
            assert(l_program->equals(l_expected));
        }

        // Test: NOT FALSE → TRUE
        {
            auto l_main = a(NOT->clone(), FALSE->clone());
            auto l_program =
                construct_program(l_helpers.begin(), l_helpers.end(), l_main);

            while(reduce_one_step(l_program))
                ;

            // Result should be TRUE = λ.λ.0
            auto l_expected = f(f(v(0)));
            assert(l_program->equals(l_expected));
        }
    }

    // Test 5: Church numerals with SUCC
    {
        std::list<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: ZERO = λ.λ.1
        const auto ZERO = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // Helper 1: SUCC = λ.λ.λ.(1 ((0 1) 2))
        const auto SUCC = g(l_helpers.size());
        l_helpers.emplace_back(
            f(f(f(a(l(1)->clone(),
                    a(a(l(0)->clone(), l(1)->clone()), l(2)->clone()))))));

        // Test: SUCC ZERO = ONE
        auto l_main = a(SUCC->clone(), ZERO->clone());
        auto l_program =
            construct_program(l_helpers.begin(), l_helpers.end(), l_main);

        while(reduce_one_step(l_program))
            ;

        // ONE = λ.λ.(0 1)
        auto l_expected = f(f(a(v(0), v(1))));
        assert(l_program->equals(l_expected));
    }

    // Test 6: Main function with lambdas using local variables
    {
        std::list<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: TRUE
        const auto TRUE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(0))));

        // Main: λ.λ.((TRUE l(0)) l(1))
        // Two-arg function that applies TRUE to its arguments
        // l(0) = v(1), l(1) = v(2)
        auto l_main = f(f(a(a(TRUE->clone(), l(0)->clone()), l(1)->clone())));

        auto l_program =
            construct_program(l_helpers.begin(), l_helpers.end(), l_main);

        while(reduce_one_step(l_program))
            ;

        // After normalization: helpers fall away, locals become globals
        // l(0) was v(1), becomes v(0) after 1 helper removed
        // Result: λ.λ.0 (TRUE returns first arg, which was l(0), now v(0))
        auto l_expected = f(f(v(0)));
        assert(l_program->equals(l_expected));
    }

    // Test 7: Test with different iterator type (std::vector)
    {
        std::vector<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: K combinator
        const auto K = g(l_helpers.size());
        l_helpers.push_back(f(f(l(0))));

        // Helper 1: S combinator λ.λ.λ.((0 2) (1 2))
        const auto S = g(l_helpers.size());
        l_helpers.push_back(f(f(f(a(a(l(0)->clone(), l(2)->clone()),
                                    a(l(1)->clone(), l(2)->clone()))))));

        // Main: ((S K) K) applied to a local
        // l(0) = v(2) (two helpers in place)
        auto l_skk = a(a(S->clone(), K->clone()), K->clone());
        auto l_main = a(l_skk->clone(), l(0)->clone());

        auto l_program =
            construct_program(l_helpers.begin(), l_helpers.end(), l_main);

        while(reduce_one_step(l_program))
            ;

        // SKK is identity, returns its argument
        // l(0) was v(2), after 2 helpers removed: v(2) - 2 = v(0)
        assert(l_program->equals(v(0)));
    }

    // Test 8: Helper that uses multiple earlier helpers
    {
        std::list<std::unique_ptr<expr>> l_helpers{};

        auto l = [&l_helpers](size_t a_local_index)
        { return v(l_helpers.size() + a_local_index); };
        auto g = [&l_helpers](size_t a_global_index)
        { return v(a_global_index); };

        // Helper 0: TRUE
        const auto TRUE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(0))));

        // Helper 1: FALSE
        const auto FALSE = g(l_helpers.size());
        l_helpers.emplace_back(f(f(l(1))));

        // Helper 2: AND = λ.λ.((0 1) FALSE)
        const auto AND = g(l_helpers.size());
        l_helpers.emplace_back(
            f(f(a(a(l(0)->clone(), l(1)->clone()), FALSE->clone()))));

        // Test: ((AND TRUE) TRUE) → TRUE
        auto l_main = a(a(AND->clone(), TRUE->clone()), TRUE->clone());
        auto l_program =
            construct_program(l_helpers.begin(), l_helpers.end(), l_main);

        while(reduce_one_step(l_program))
            ;

        // Result: TRUE = λ.λ.0
        auto l_expected = f(f(v(0)));
        assert(l_program->equals(l_expected));
    }
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
        auto l_true_program = construct_program(
            l_helpers.begin(), l_helpers.end(), l_true_case_main);

        // construct the program
        auto l_false_program = construct_program(
            l_helpers.begin(), l_helpers.end(), l_false_case_main);

        // reduce the programs
        while(reduce_one_step(l_true_program))
            ;
        while(reduce_one_step(l_false_program))
            ;

        std::cout << "true reduced: ";
        l_true_program->print(std::cout);
        std::cout << std::endl;

        std::cout << "false reduced: ";
        l_false_program->print(std::cout);
        std::cout << std::endl;

        // test the reductions.
        // NOTE: after reduction of a program, the main function's locals BECOME
        // globals.
        assert(l_true_program->equals(f(g(10))));
        assert(l_false_program->equals(f(g(11))));
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
        auto ZERO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ZERO);
        auto ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ONE);
        auto TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), TWO);
        auto THREE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), THREE);
        auto FOUR_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), FOUR);
        auto FIVE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), FIVE);

        // reduce zero
        while(reduce_one_step(ZERO_PROGRAM))
            ;
        std::cout << "zero reduced: ";
        ZERO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        // reduce one
        while(reduce_one_step(ONE_PROGRAM))
            ;
        std::cout << "one reduced: ";
        ONE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        // reduce two
        while(reduce_one_step(TWO_PROGRAM))
            ;
        std::cout << "two reduced: ";
        TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        // reduce three
        while(reduce_one_step(THREE_PROGRAM))
            ;
        std::cout << "three reduced: ";
        THREE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        // reduce four
        while(reduce_one_step(FOUR_PROGRAM))
            ;
        std::cout << "four reduced: ";
        FOUR_PROGRAM->print(std::cout);
        std::cout << std::endl;

        // reduce five
        while(reduce_one_step(FIVE_PROGRAM))
            ;
        std::cout << "five reduced: ";
        FIVE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        assert(ONE_PROGRAM->equals(f(f(a(g(0), g(1))))));
        assert(TWO_PROGRAM->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(THREE_PROGRAM->equals(f(f(a(g(0), a(g(0), a(g(0), g(1))))))));
        assert(FOUR_PROGRAM->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(FIVE_PROGRAM->equals(
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

        auto ADD_ONE_ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_ONE_ONE);
        auto ADD_ONE_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_ONE_TWO);
        auto ADD_TWO_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), ADD_TWO_TWO);
        auto ADD_THREE_TWO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), ADD_THREE_TWO);
        auto ADD_FIVE_FIVE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), ADD_FIVE_FIVE);

        // reduce the programs
        while(reduce_one_step(ADD_ONE_ONE_PROGRAM))
            ;
        while(reduce_one_step(ADD_ONE_TWO_PROGRAM))
            ;
        while(reduce_one_step(ADD_TWO_TWO_PROGRAM))
            ;
        while(reduce_one_step(ADD_THREE_TWO_PROGRAM))
            ;
        while(reduce_one_step(ADD_FIVE_FIVE_PROGRAM))
            ;

        std::cout << "add one one: ";
        ADD_ONE_ONE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        std::cout << "add one two: ";
        ADD_ONE_TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        std::cout << "add two two: ";
        ADD_TWO_TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        std::cout << "add three two: ";
        ADD_THREE_TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        std::cout << "add five five: ";
        ADD_FIVE_FIVE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        // assertions
        assert(ADD_ONE_ONE_PROGRAM->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(
            ADD_ONE_TWO_PROGRAM->equals(f(f(a(g(0), a(g(0), a(g(0), g(1))))))));
        assert(ADD_TWO_TWO_PROGRAM->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(ADD_THREE_TWO_PROGRAM->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), a(g(0), g(1))))))))));
        assert(ADD_FIVE_FIVE_PROGRAM->equals(f(f(a(
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
        auto MULT_ZERO_ZERO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_ZERO_ZERO);
        auto MULT_ZERO_ONE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_ZERO_ONE);
        auto MULT_ONE_ONE_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), MULT_ONE_ONE);
        auto MULT_ONE_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), MULT_ONE_TWO);
        auto MULT_TWO_TWO_PROGRAM =
            construct_program(l_helpers.begin(), l_helpers.end(), MULT_TWO_TWO);
        auto MULT_THREE_TWO_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_THREE_TWO);
        auto MULT_FIVE_FIVE_PROGRAM = construct_program(
            l_helpers.begin(), l_helpers.end(), MULT_FIVE_FIVE);

        // reduce the programs (with printing in between)
        while(reduce_one_step(MULT_ZERO_ZERO_PROGRAM))
            ;
        std::cout << "mult zero zero: ";
        MULT_ZERO_ZERO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        while(reduce_one_step(MULT_ZERO_ONE_PROGRAM))
            ;
        std::cout << "mult zero one: ";
        MULT_ZERO_ONE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        while(reduce_one_step(MULT_ONE_ONE_PROGRAM))
            ;
        std::cout << "mult one one: ";
        MULT_ONE_ONE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        while(reduce_one_step(MULT_ONE_TWO_PROGRAM))
            ;
        std::cout << "mult one two: ";
        MULT_ONE_TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        while(reduce_one_step(MULT_TWO_TWO_PROGRAM))
            ;
        std::cout << "mult two two: ";
        MULT_TWO_TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        while(reduce_one_step(MULT_THREE_TWO_PROGRAM))
            ;
        std::cout << "mult three two: ";
        MULT_THREE_TWO_PROGRAM->print(std::cout);
        std::cout << std::endl;

        while(reduce_one_step(MULT_FIVE_FIVE_PROGRAM))
            ;
        std::cout << "mult five five: ";
        MULT_FIVE_FIVE_PROGRAM->print(std::cout);
        std::cout << std::endl;

        assert(MULT_ZERO_ZERO_PROGRAM->equals(f(f(g(1)))));
        assert(MULT_ZERO_ONE_PROGRAM->equals(f(f(g(1)))));
        assert(MULT_ONE_ONE_PROGRAM->equals(f(f(a(g(0), g(1))))));
        assert(MULT_ONE_TWO_PROGRAM->equals(f(f(a(g(0), a(g(0), g(1)))))));
        assert(MULT_TWO_TWO_PROGRAM->equals(
            f(f(a(g(0), a(g(0), a(g(0), a(g(0), g(1)))))))));
        assert(MULT_THREE_TWO_PROGRAM->equals(
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

        assert(MULT_FIVE_FIVE_PROGRAM->equals(TWENTY_FIVE->clone()));
    }

    // test for fun
    {
        // this should never terminate -- omega combinator
        const auto OMEGA = a(f(a(v(0), v(0))), f(a(v(0), v(0))));
        auto l_omega_expr = OMEGA->clone();

        // omega becomes itself forever
        // With limit 999, does 999 reductions (hits step limit)
        size_t l_step_count = 0;
        size_t l_step_limit = 999;
        while(l_step_count < l_step_limit && reduce_one_step(l_omega_expr))
            ++l_step_count;

        assert(l_step_count == 999);
        assert(l_omega_expr->equals(OMEGA->clone()));
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

            while(reduce_one_step(expr))
                ;

            std::cout << "I a: ";
            expr->print(std::cout);
            std::cout << std::endl;
            assert(expr->equals(a_arg));
        }

        // Test K a b → a
        {
            auto a_arg = v(7);
            auto b_arg = v(8);
            auto expr = a(a(K->clone(), a_arg->clone()), b_arg->clone());

            while(reduce_one_step(expr))
                ;

            std::cout << "K a b: ";
            expr->print(std::cout);
            std::cout << std::endl;
            assert(expr->equals(a_arg));
        }

        // Test S K K a → a
        {
            auto a_arg = v(10);
            auto expr =
                a(a(a(S->clone(), K->clone()), K->clone()), a_arg->clone());

            while(reduce_one_step(expr))
                ;

            std::cout << "S K K a: ";
            expr->print(std::cout);
            std::cout << std::endl;
            assert(expr->equals(a_arg));
        }

        // Test S I I a → a a
        {
            auto a_arg = v(12);
            auto expr =
                a(a(a(S->clone(), I->clone()), I->clone()), a_arg->clone());

            while(reduce_one_step(expr))
                ;

            std::cout << "S I I a: ";
            expr->print(std::cout);
            std::cout << std::endl;
            auto expected = a(a_arg->clone(), a_arg->clone());
            assert(expr->equals(expected));
        }

        // Additional test: Use closed terms instead of free variables
        // Test K (λ.5) (λ.6) → λ.5
        {
            auto a_arg = f(v(5));
            auto b_arg = f(v(6));
            auto expr = a(a(K->clone(), a_arg->clone()), b_arg->clone());

            while(reduce_one_step(expr))
                ;

            std::cout << "K (λ.5) (λ.6): ";
            expr->print(std::cout);
            std::cout << std::endl;
            assert(expr->equals(a_arg));
        }

        // Test S I I (λ.7) → (λ.7) (λ.7) → 6
        {
            auto a_arg = f(v(7));
            auto expr =
                a(a(a(S->clone(), I->clone()), I->clone()), a_arg->clone());

            while(reduce_one_step(expr))
                ;

            std::cout << "S I I (λ.7): ";
            expr->print(std::cout);
            std::cout << std::endl;
            auto expected = v(6);
            assert(expr->equals(expected));
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

    TEST(test_var_clone);
    TEST(test_func_clone);
    TEST(test_app_clone);

    TEST(test_var_update_size);
    TEST(test_func_update_size);
    TEST(test_app_update_size);

    TEST(test_var_lift);
    TEST(test_func_lift);
    TEST(test_app_lift);

    TEST(test_var_substitute);
    TEST(test_func_substitute);
    TEST(test_app_substitute);

    TEST(test_var_reduce);
    TEST(test_func_reduce);
    TEST(test_app_reduce);

    TEST(construct_program_test);

    TEST(generic_use_case_test);
}

#endif
