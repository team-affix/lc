# lc

**A minimal, efficient C++ implementation of lambda calculus with normal order reduction.**

----

| [Overview](#overview) - [Design & Implementation](#design--implementation) - [Usage](#usage) - [Integration](#integration) |
:----------------------------------------------------------: |
| [Expression Types](#expression-types) - [Features](#features) - [Examples](#examples) - [Building & Testing](#building--testing) - [License](#license) |

### Overview

**lc** is a **lightweight, self-contained lambda calculus library for C++**. It provides a complete implementation of untyped lambda calculus with normal order reduction (leftmost-outermost evaluation), making it ideal for:

- Educational purposes: learning and experimenting with lambda calculus
- Research: prototyping computational models and reduction strategies  
- Embedding: adding lambda calculus evaluation to existing applications
- Verification: testing lambda calculus properties with comprehensive constraints

The library is designed to be:
- **Minimal**: Three expression types (`var`, `func`, `app`), clean API
- **Safe**: Immutable expressions with `std::unique_ptr` memory management
- **Controlled**: Configurable step and size limits with detailed results prevent non-termination
- **Debuggable**: Returns reduction steps, peak sizes, and limit violation flags
- **Fast**: Efficient substitution and reduction with De Bruijn levels

### Design & Implementation

**lc** implements lambda calculus using **De Bruijn levels** for variable representation and **normal order reduction** for evaluation.

#### Reduction Strategy
- **Normal Order Reduction**: Leftmost-outermost redex is always reduced first
- **Beta-Reduction Only**: Only β-reductions are performed (no η-reduction)
- **Call-by-Name**: Arguments are not reduced before substitution

#### Variable Representation: De Bruijn Levels
Variables reference lambda binders by their level from the **outermost** scope:
- `v(0)` refers to the outermost lambda
- `v(1)` refers to the next outermost lambda  
- `v(n)` refers to the nth lambda from outside

**Important**: This differs from De Bruijn **indices** where counting is from the innermost scope.

#### Core Operations
The implementation handles three critical operations:
- **Reduction**: Single-step beta-reduction following normal order
- **Lifting**: Adjusts variable indices when moving expressions into deeper binding contexts
- **Substitution**: Replaces variables with argument expressions, handling:
  - Variable lifting for free variables in arguments
  - Variable decrementation for free variables in the body
  - Capture-avoiding substitution

### Usage

After including the library, you can construct and reduce lambda expressions:

```cpp
using namespace lambda;

// Identity function: (λ.0) 5 → 5
auto identity = f(v(0));  // λ.0
auto expr = a(identity->clone(), v(5));
auto result = expr->normalize();
// result.m_expr equals v(5)
```

The library uses factory functions for construction:
- `v(index)` creates a variable
- `f(body)` creates a lambda abstraction
- `a(lhs, rhs)` creates an application

All functions return `std::unique_ptr<expr>`.

### Expression Types

Lambda calculus expressions are built from three fundamental types:

#### `var` - Variable
Represents a variable reference using De Bruijn levels.
- **Syntax**: `v(0)` references the outermost lambda binder
- **Semantics**: A variable is bound if it refers to a lambda in scope, or free otherwise
- **Size**: Always 1

#### `func` - Lambda Abstraction  
Represents a function (lambda abstraction) with a single parameter and a body.
- **Syntax**: `f(v(0))` is `λ.0`, the identity function
- **Semantics**: Creates a new binding scope; the bound variable can be referenced in the body
- **Size**: `1 + body_size`

#### `app` - Application
Represents function application (applying a function to an argument).
- **Syntax**: `a(f(v(0)), v(5))` applies identity to variable 5
- **Semantics**: When lhs is a lambda, performs beta-reduction by substitution
- **Size**: `1 + lhs_size + rhs_size`

**Expression Properties:**
- **Immutable**: Operations create new expressions rather than modifying existing ones
- **Polymorphic**: Accessed through the base `expr` interface
- **Memory-safe**: Managed via `std::unique_ptr`

### Features

#### Expression Normalization

The `normalize()` method reduces expressions to normal form and returns a `normalize_result` struct with detailed information:

```cpp
struct normalize_result {
    bool m_step_excess;           // true if step limit was hit
    bool m_size_excess;           // true if size limit was hit
    size_t m_step_count;          // number of reductions performed
    size_t m_size_peak;           // maximum intermediate result size
    std::unique_ptr<expr> m_expr; // the normalized expression
};

normalize_result normalize(
    size_t a_step_limit = SIZE_MAX,    // maximum reductions allowed
    size_t a_size_limit = SIZE_MAX     // maximum expression size allowed
) const;
```

**Return Value: `normalize_result` struct**

- **`m_step_excess`**: `true` if step limit was reached while more reductions are possible
- **`m_size_excess`**: `true` if a reduction would have exceeded the size limit
- **`m_step_count`**: Number of reductions actually performed (never exceeds `a_step_limit`)
- **`m_size_peak`**: Maximum size of intermediate results (excludes limit-violating reductions)
- **`m_expr`**: The normalized expression (or partially normalized if limits hit)

**Limit Precedence:**

Step limit **ALWAYS** takes precedence over size limit. When both limits would block a reduction:
- `m_step_excess = true`
- `m_size_excess = false`

**When Limits Are Exceeded:**

- **Step limit**: `m_step_excess = true` when `m_step_count` reaches `a_step_limit` AND more reductions are possible
- **Size limit**: `m_size_excess = true` when a reduction would produce a result with `size > a_size_limit`
- Only ONE excess flag can be `true` (step takes precedence)
- If `m_step_excess = true`, then `m_step_count == a_step_limit`
- If `m_size_excess = true`, then `m_step_count < a_step_limit`

**Reduction Lookahead Behavior:**

The normalizer uses lookahead to enforce limits:
1. The next reduction is computed (lookahead)
2. Limits are checked *before* applying it
3. If a limit would be violated
   1. The appropriate excess flag is set
   2. The reduction is **discarded** (not committed)
   3. Normalization halts immediately
4. The returned expression is the **last valid state** (before the limit-violating reduction)

This ensures:
- The returned expression is always within limits OR is the initial expression
- `m_step_count` reflects only successful, committed reductions
- `m_size_peak` only includes sizes of committed reductions

**Size Checking:**

- **Initial expression size is NEVER checked** against `a_size_limit`
- Only reduction **results** are checked
- This allows large expressions that reduce to smaller results
- Example: Expression of size 100 can normalize with `a_size_limit = 50` if all intermediate results are ≤ 50

**Size Peak Behavior:**

- Tracks maximum size of intermediate **results** (not original expression)
- Updated after each successful reduction
- If no reductions occur: `m_size_peak = std::numeric_limits<size_t>::min()`
- Does **NOT** include sizes of rejected (limit-violating) reductions

#### Emulating Delta Reductions with `construct_program`

The `construct_program()` function enables **delta reductions** (named function definitions) to be emulated through pure beta-reductions. This is particularly useful for building complex programs with reusable helper functions.

**What are Delta Reductions?**

In lambda calculus, a **delta reduction** is a named reduction rule like `SUCC → λf.λx.f(fx)`. While pure lambda calculus only has beta-reduction, delta reductions allow convenient definitions of functions by name.

**How `construct_program` Works:**

Given a list of helper expressions `[h₀, h₁, h₂, ...]` and a main expression `M`, `construct_program` builds a tower of lambda abstractions:

```
((λ.((λ.((λ.M) h₂)) h₁)) h₀)
```

When normalized, this structure binds each helper to its De Bruijn level (h₀→0, h₁→1, etc.), allowing the main expression to reference helpers as global variables. After normalization, the helper bindings are eliminated through substitution.

**Function Signature:**

```cpp
template <typename IT>
std::unique_ptr<expr> construct_program(
    IT a_helpers_begin,
    IT a_helpers_end,
    const std::unique_ptr<expr>& a_main_fn
);
```

- **Template parameter**: `IT` can be any iterator type (`std::list`, `std::vector`, etc.)
- **Returns**: A program expression ready for normalization

**The `l()` and `g()` Helper Pattern:**

When building helpers and main expressions, use these lambda helpers to correctly reference variables:

```cpp
std::list<std::unique_ptr<expr>> helpers;

// l(index) creates a LOCAL variable reference
auto l = [&helpers](size_t a_local_index) {
    return v(helpers.size() + a_local_index);
};

// g(index) creates a GLOBAL variable reference (to a helper)
auto g = [&helpers](size_t a_global_index) {
    return v(a_global_index);
};
```

- **`l(index)`**: References local variables/parameters within the current helper or main function
- **`g(index)`**: References previously defined helpers by their index in the helpers list

**Example: Church Booleans with NOT Combinator**

```cpp
using namespace lambda;
std::list<std::unique_ptr<expr>> helpers;

auto l = [&helpers](size_t a_local_index) {
    return v(helpers.size() + a_local_index);
};
auto g = [&helpers](size_t a_global_index) {
    return v(a_global_index);
};

// Helper 0: TRUE = λ.λ.0 (returns first argument)
const auto TRUE = g(helpers.size());
helpers.emplace_back(f(f(l(0))));

// Helper 1: FALSE = λ.λ.1 (returns second argument)
const auto FALSE = g(helpers.size());
helpers.emplace_back(f(f(l(1))));

// Helper 2: NOT = λ.((arg FALSE) TRUE)
// Uses earlier helpers (FALSE and TRUE)
const auto NOT = g(helpers.size());
helpers.emplace_back(
    f(a(a(l(0), FALSE->clone()), TRUE->clone())));

// Main: Apply NOT to TRUE
auto main_expr = a(NOT->clone(), TRUE->clone());

// Construct and normalize the program
auto program = construct_program(helpers.begin(), helpers.end(), main_expr);
auto result = program->normalize();

// Result: FALSE = λ.λ.1
assert(result.m_expr->equals(f(f(v(1)))));
```

**Key Insight: Locals Become Globals**

After normalization, helper bindings are eliminated through substitution, effectively **decrementing all variable indices** by the number of helpers. This means:
- Locals in the main function become globals (zero-indexed from the outermost scope)
- A local `l(0)` (which was `v(N)` where N = helpers.size()) becomes `v(0)` after N helpers are eliminated

**Use Cases:**

- Building libraries of reusable combinators (TRUE, FALSE, NOT, AND, OR)
- Church numeral arithmetic (ZERO, SUCC, ADD, MULT)
- Complex recursive functions using Y-combinator
- Testing reduction properties with named definitions
- Educational demonstrations of lambda calculus encodings

#### Expression Size

Every expression has a `size()` method that returns its structural size:

```cpp
size_t size() const;
```

**Size calculation:**
- `var`: size = 1
- `func`: size = 1 + body_size
- `app`: size = 1 + lhs_size + rhs_size

This allows tracking expression growth during reduction and preventing size explosions.

#### Expression Deep Cloning

Since expressions are managed by `std::unique_ptr`, the `clone()` method provides deep copying:

```cpp
std::unique_ptr<expr> clone() const;
```

- **Purpose**: Creates an independent deep copy of an expression
- **Use case**: Required when reusing an expression, as `std::unique_ptr` enforces single ownership
- **Example**: `auto copy = expr->clone();`

### Examples

#### Basic Construction

```cpp
using namespace lambda;

// Lambda abstraction: λ.0 (identity function)
auto identity = f(v(0));

// Application: (λ.0) 5
auto expr = a(f(v(0)), v(5));
```

#### Simple Reduction

```cpp
// Identity function: (λ.0) 5 → 5
auto identity = f(v(0));  // λ.0
auto expr = a(identity->clone(), v(5));
auto result = expr->normalize();
// result.m_expr equals v(5)
// result.m_step_count == 1

// K combinator: ((λ.λ.0) 7) 8 → 7
auto K = f(f(v(0)));  // λ.λ.0
auto expr = a(a(K->clone(), v(7)), v(8));
auto result = expr->normalize();
// result.m_expr equals v(7)
// result.m_step_count == 2
```

#### Church Numerals

```cpp
// Church numeral ZERO: λf.λx. x
// In the body, x is bound by the inner lambda (depth 1), so v(1)
auto zero = f(f(v(1)));

// Church numeral ONE: λf.λx. f x
// In the body: f is outermost (depth 0) = v(0), x is inner (depth 1) = v(1)
auto one = f(f(a(v(0), v(1))));

// Church numeral TWO: λf.λx. f (f x)
// f is outermost (depth 0) = v(0), x is inner (depth 1) = v(1)
auto two = f(f(a(v(0), a(v(0), v(1)))));
```

#### Reduction with Constraints

```cpp
// Step limit with omega combinator
// Omega combinator: (λ.(0 0)) (λ.(0 0))
auto omega = a(f(a(v(0), v(0))), f(a(v(0), v(0))));
auto result = omega->normalize(100);  // Limit to 100 steps
// result.m_step_excess == true (more reductions possible)
// result.m_step_count == 100 (exactly 100 reductions were applied)
// The 101st reduction was computed but NOT applied
// omega reduces to itself indefinitely

// Size limit with growing expression
// Growing combinator: (λ.(0 0)) (λ.(0 0 0))
auto growing = a(f(a(v(0), v(0))), f(a(a(v(0), v(0)), v(0))));
auto result = growing->normalize(SIZE_MAX, 20);  // size_limit=20
if (result.m_size_excess) {
    std::cout << "Stopped due to size after " 
              << result.m_step_count << " steps\n";
    // The reduction that would exceed 20 was detected via lookahead and discarded
    // result.m_expr->size() <= 20
    // result.m_size_peak <= 20
}

// Track both metrics
auto result = expr->normalize(1000, 100);  // step_limit=1000, size_limit=100
std::cout << "Steps: " << result.m_step_count 
          << ", Peak: " << result.m_size_peak << "\n";
if (result.m_step_excess) {
    std::cout << "Hit step limit\n";
}
if (result.m_size_excess) {
    std::cout << "Hit size limit\n";
}

// Limit precedence demonstration
auto expr = ...;  // Expression needing many steps
auto result = expr->normalize(5, 10);  // Both limits restrictive
// If step limit hits after 5 reductions:
// result.m_step_excess == true
// result.m_size_excess == false  // Step takes precedence
// result.m_step_count == 5

// Initial size doesn't matter
auto large_expr = ...; // Expression with size 100
auto result = large_expr->normalize(SIZE_MAX, 50);  // size_limit=50
// Succeeds if all intermediate results have size <= 50
// Initial size of 100 is not checked

// Understanding lookahead behavior
auto expr = ...; // Expression needing 10 steps to normalize
auto result = expr->normalize(5);  // step_limit=5
// result.m_step_count == 5 (only 5 reductions applied)
// The 6th reduction was computed (lookahead) but discarded
// result.m_expr is the state after exactly 5 reductions
```

#### Combinator Identities

```cpp
// I combinator: λ.0
auto I = f(v(0));

// K combinator: λ.λ.0
auto K = f(f(v(0)));

// S combinator: λ.λ.λ.((0 2) (1 2))
auto S = f(f(f(a(a(v(0), v(2)), a(v(1), v(2))))));

// Test: S K K a → a (SKK is equivalent to I)
auto expr = a(a(a(S->clone(), K->clone()), K->clone()), v(10));
auto result = expr->normalize();
// result.m_expr equals v(10)
```

#### Measuring Expression Size

```cpp
// Expression: (λ.0) 5
auto expr = a(f(v(0)), v(5));
std::cout << "Size: " << expr->size() << std::endl;  // Prints: 4
// Size breakdown: 1 (app) + 2 (func with var body) + 1 (var) = 4

auto result = expr->normalize();
std::cout << "Size after: " << result.m_expr->size() << std::endl;  // Prints: 1
// Just v(5) remains
```

### Integration

**The core of lc is self-contained within a few platform-agnostic files** which you can easily compile into your application. All source files are in the repository:
- `include/lambda.hpp` - Public interface
- `src/lambda.cpp` - Implementation

**Building and linking against the library is required for usage in your project**.

### Building & Testing

Currently, building only has supported methods for Linux, but the build is simple.

#### Build Library

```bash
make
```

This creates:
- `build/liblc.a` - Static library

#### Run Tests

The project includes comprehensive unit tests covering:
- Expression construction and equality
- Lifting and substitution  
- Beta-reduction (one-step and normalization)
- Size calculation
- Reduction constraints (step and size limits)
- Combinator identities (I, K, S)
- Church numeral arithmetic
- `construct_program` with helpers and dependencies

Build tests with:
```bash
make debug
```

Run tests with:
```bash
./build/main
```

### License

See [LICENSE](LICENSE) file for details.
