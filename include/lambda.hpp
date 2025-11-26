#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <cstddef>
#include <memory>
#include <ostream>

namespace lambda
{

struct expr
{
    virtual ~expr() = default;

    // ACCESSOR METHODS
    // checks if the expression is equal to another
    virtual bool equals(const std::unique_ptr<expr>&) const = 0;
    // prints the expression to a_ostream
    virtual void print(std::ostream& a_ostream) const = 0;
    // creates a deep copy of the expression
    virtual std::unique_ptr<expr> clone() const = 0;

    // MUTATOR METHODS
    // updates the size of the expression given the sizes of its children
    virtual void update_size() = 0;
    // lifts all free variables in a_expr by a_lift_amount,
    // given that free variables are those whose level is >= a_cutoff.
    virtual void lift(size_t a_lift_amount, size_t a_cutoff) = 0;

    expr();
    expr(const expr& other) = delete;
    expr& operator=(const expr& other) = delete;

    // MEMBER VARIABLES
    size_t m_size;
};

struct var : expr
{
    virtual ~var() = default;

    // ACCESSOR METHODS
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> clone() const override;

    // MUTATOR METHODS
    void update_size() override;
    void lift(size_t a_lift_amount, size_t a_cutoff) override;

    // MEMBER VARIABLES
    size_t m_index;

  private:
    var(size_t a_index);
    friend std::unique_ptr<expr> v(size_t a_index);
};

struct func : expr
{
    virtual ~func() = default;

    // ACCESSOR METHODS
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> clone() const override;

    // MUTATOR METHODS
    void update_size() override;
    void lift(size_t a_lift_amount, size_t a_cutoff) override;

    // MEMBER VARIABLES
    std::unique_ptr<expr> m_body;

  private:
    func(std::unique_ptr<expr>&& a_body);
    friend std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body);
};

struct app : expr
{
    virtual ~app() = default;

    // ACCESSOR METHODS
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> clone() const override;

    // MUTATOR METHODS
    void update_size() override;
    void lift(size_t a_lift_amount, size_t a_cutoff) override;

    // MEMBER VARIABLES
    std::unique_ptr<expr> m_lhs;
    std::unique_ptr<expr> m_rhs;

  private:
    app(std::unique_ptr<expr>&& a_lhs, std::unique_ptr<expr>&& a_rhs);
    friend std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_lhs,
                                   std::unique_ptr<expr>&& a_rhs);
};

// FACTORY FUNCTIONS

std::unique_ptr<expr> v(size_t a_index);
std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body);
std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_lhs,
                        std::unique_ptr<expr>&& a_rhs);

// operator for printing expressions to ostreams
std::ostream& operator<<(std::ostream& a_ostream, const expr& a_expr);

// REWRITING FUNCTIONS

// replaces all occurrances of the variable with index a_var_index in
// a_expr with a_arg, lifted by a_lift_amount plus the number of binders
// between root of a_expr and the occurrance of the variable.
// Returns the size increase due to the substitution.
void substitute(std::unique_ptr<expr>& a_expr, size_t a_lift_amount,
                size_t a_var_index, const std::unique_ptr<expr>& a_arg);

// attempts to find and reduce the leftmost-outermost redex in a_expr.
// returns true if a reduction was found and performed, false otherwise.
bool reduce_one_step(std::unique_ptr<expr>& a_expr, size_t a_depth = 0);

// construct_program: builds a tower of lambda abstractions to emulate delta
// reductions through beta-reductions.
//
// Given helpers [h0, h1, h2, ...] and a main function M, constructs:
//   ((λ.((λ.((λ.M) h2)) h1)) h0)
//
// When normalized, this binds each helper to its index (h0→0, h1→1, etc.),
// allowing the main function to reference helpers as De Bruijn level variables.
//
// Template parameter IT must support:
//   - Dereference (*it) -> const std::unique_ptr<expr>&
//   - std::next(it) -> IT
//   - Equality comparison (it == end)
template <typename IT>
std::unique_ptr<expr> construct_program(IT a_helpers_begin, IT a_helpers_end,
                                        const std::unique_ptr<expr>& a_main_fn)
{
    // Base case: no helpers, just return the main function
    if(a_helpers_begin == a_helpers_end)
        return a_main_fn->clone();

    // Recursive case: wrap in lambda and apply first helper
    // ((λ.(recursion with remaining helpers)) first_helper)
    return a(f(construct_program(std::next(a_helpers_begin), a_helpers_end,
                                 a_main_fn)),
             (*a_helpers_begin)->clone());
}

} // namespace lambda

#endif
