#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include <cstddef>
#include <limits>
#include <memory>
#include <ostream>

namespace lambda
{

struct expr
{
    // used for normalize() results:
    struct normalize_result
    {
        bool m_step_excess;
        bool m_size_excess;
        size_t m_step_count;
        size_t m_size_peak;
        std::unique_ptr<expr> m_expr;
    };
    virtual ~expr() = default;
    virtual bool equals(const std::unique_ptr<expr>&) const = 0;
    virtual void print(std::ostream& a_ostream) const = 0;
    virtual std::unique_ptr<expr> lift(size_t a_lift_amount,
                                       size_t a_cutoff) const = 0;
    virtual std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const = 0;
    virtual std::unique_ptr<expr> reduce_one_step(size_t a_depth) const = 0;
    std::unique_ptr<expr> clone() const;
    normalize_result
    normalize(size_t a_step_limit = std::numeric_limits<size_t>::max(),
              size_t a_size_limit = std::numeric_limits<size_t>::max()) const;
    size_t size() const;
    expr(size_t a_size);
    expr(const expr& other) = delete;
    expr& operator=(const expr& other) = delete;

  private:
    const size_t m_size;
};

struct var : expr
{
    virtual ~var() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_lift_amount,
                               size_t a_cutoff) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce_one_step(size_t a_depth) const override;
    size_t index() const;

  private:
    var(size_t a_index);
    friend std::unique_ptr<expr> v(size_t a_index);
    const size_t m_index;
};

struct func : expr
{
    virtual ~func() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_lift_amount,
                               size_t a_cutoff) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce_one_step(size_t a_depth) const override;
    const std::unique_ptr<expr>& body() const;

  private:
    func(std::unique_ptr<expr>&& a_body);
    friend std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body);
    const std::unique_ptr<expr> m_body;
};

struct app : expr
{
    virtual ~app() = default;
    bool equals(const std::unique_ptr<expr>&) const override;
    void print(std::ostream& a_ostream) const override;
    std::unique_ptr<expr> lift(size_t a_lift_amount,
                               size_t a_cutoff) const override;
    std::unique_ptr<expr>
    substitute(size_t a_lift_amount, size_t a_var_index,
               const std::unique_ptr<expr>& a_arg) const override;
    std::unique_ptr<expr> reduce_one_step(size_t a_depth) const override;
    const std::unique_ptr<expr>& lhs() const;
    const std::unique_ptr<expr>& rhs() const;

  private:
    app(std::unique_ptr<expr>&& a_lhs, std::unique_ptr<expr>&& a_rhs);
    friend std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_lhs,
                                   std::unique_ptr<expr>&& a_rhs);
    const std::unique_ptr<expr> m_lhs;
    const std::unique_ptr<expr> m_rhs;
};

// factory functions
std::unique_ptr<expr> v(size_t a_index);
std::unique_ptr<expr> f(std::unique_ptr<expr>&& a_body);
std::unique_ptr<expr> a(std::unique_ptr<expr>&& a_lhs,
                        std::unique_ptr<expr>&& a_rhs);

// operator for printing expressions to ostreams
std::ostream& operator<<(std::ostream& a_ostream, const expr& a_expr);

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
