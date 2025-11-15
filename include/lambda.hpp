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

} // namespace lambda

#endif
