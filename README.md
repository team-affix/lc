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

**lc** prioritizes efficiency: expressions mutate in place during reduction, minimizing allocations. Immutability is opt-in via explicit cloning.

The library is designed to be:
- **Minimal**: Three expression types (`var`, `func`, `app`), clean API
- **Efficient**: In-place mutation minimizes allocations; immutability is opt-in via `clone()`
- **Flexible**: Manual reduction loops give full control over termination conditions
- **Safe**: Memory-safe with `std::unique_ptr` management
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

#### Mutation Philosophy

**lc** prioritizes **efficiency** through in-place mutation:
- **Default behavior**: Operations mutate expressions directly, avoiding clones of unchanged sub-trees
- **Opt-in immutability**: Explicitly `clone()` before mutation when you need to preserve originals
- **Performance benefit**: Large expressions no longer copied during reduction
- **Free functions for rewriting**: `reduce_one_step()` and `substitute()` operate on `std::unique_ptr<expr>&` because they need parent context during traversal

#### Core Operations
The implementation handles four critical operations:
- **Reduction**: Single-step beta-reduction following normal order (mutates in place)
- **Substitution**: Replaces variables with argument expressions (mutates in place)
- **Lifting**: Adjusts variable indices when moving expressions into deeper binding contexts (mutates in place)
- **Size Updates**: `update_size()` synchronizes cached sizes after mutations

### Usage

After including the library, you can construct and reduce lambda expressions:

```cpp
using namespace lambda;

// Identity function: (λ.0) 5 → 5
auto identity = f(v(0));
auto expr = a(identity->clone(), v(5));

// Normalize to beta-normal form (mutates expr in place)
while(reduce_one_step(expr));

assert(expr->equals(v(5)));
```

**Opt-in Immutability:**

By default, reduction mutates expressions in place for efficiency. To preserve the original, clone explicitly:

```cpp
auto original = a(f(v(0)), v(5));
auto copy = original->clone();  // Explicit clone for immutability

while(reduce_one_step(copy));   // Only copy is mutated

assert(original->equals(a(f(v(0)), v(5))));  // Original preserved
assert(copy->equals(v(5)));                   // Copy normalized
```

**Factory Functions:**
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
- **In-place mutation**: Operations modify expressions directly for efficiency
- **Opt-in immutability**: Use `clone()` explicitly to preserve originals before mutation
- **Polymorphic**: Accessed through the base `expr` interface
- **Memory-safe**: Managed via `std::unique_ptr`
- **Direct member access**: Use `m_size`, `m_lhs`, `m_rhs`, `m_body`, `m_index` directly

### Features

#### Expression Normalization

Normalize expressions to beta-normal form using `reduce_one_step()`:

```cpp
auto expr = a(f(v(0)), v(5));
while(reduce_one_step(expr));  // Mutates in place
```

**The `reduce_one_step()` Function:**

```cpp
bool reduce_one_step(std::unique_ptr<expr>& a_expr, size_t a_depth = 0);
```

- **Returns**: `true` if reduction performed, `false` if beta-normal form reached
- **Mutates in place**: The expression is modified directly for efficiency
- **Parameters**: 
  - `a_expr`: Expression to reduce (modified in place)
  - `a_depth`: Current binding depth (default 0, rarely needs specification)

**Controlling Normalization:**

Full control over the reduction loop for custom termination logic:

```cpp
// Step limit: reduce at most 100 times
size_t step_count = 0;
while(step_count < 100 && reduce_one_step(expr))
    ++step_count;

// Size limit: stop if expression exceeds size
while(expr->m_size <= 1000 && reduce_one_step(expr));

// Combined: step limit with size tracking
size_t steps = 0;
size_t max_size = 0;
while(steps < 100 && reduce_one_step(expr)) {
    ++steps;
    max_size = std::max(max_size, expr->m_size);
}
```

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

// Normalize in place
while(reduce_one_step(program));
assert(program->equals(f(f(v(1)))));

// Or preserve original with clone()
auto program_copy = program->clone();
while(reduce_one_step(program_copy));
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

Every expression maintains a cached size in the `m_size` member:

```cpp
auto expr = a(f(v(0)), v(5));
std::cout << "Size: " << expr->m_size << std::endl;  // Direct member access
```

**Size calculation:**
- `var`: size = 1
- `func`: size = 1 + body_size
- `app`: size = 1 + lhs_size + rhs_size

Size is cached in `m_size` for O(1) access. After mutations, sizes are automatically synchronized via `update_size()`. User code doesn't call this manually—it's handled internally for efficiency.

#### Core Rewriting Functions

Two main functions mutate expressions in place for efficiency:

**`reduce_one_step()`** - Performs one beta-reduction:

```cpp
bool reduce_one_step(std::unique_ptr<expr>& a_expr, size_t a_depth = 0);
```

Finds and reduces the leftmost-outermost redex following normal-order reduction. Returns `false` when expression reaches beta-normal form. Mutates in place.

**`substitute()`** - Performs variable substitution:

```cpp
void substitute(std::unique_ptr<expr>& a_expr, 
                size_t a_lift_amount,
                size_t a_var_index, 
                const std::unique_ptr<expr>& a_arg);
```

Replaces all occurrences of variable `a_var_index` with `a_arg`, handling:
- Variable lifting for free variables in arguments
- Variable decrementation for free variables in body
- Capture-avoiding substitution

**Why Free Functions:** These are free functions (not methods) because they need parent context during tree traversal and must modify the `std::unique_ptr` itself for in-place mutation.

#### Direct Member Access

Expressions expose internal state via public members for efficiency:

```cpp
// Base expr class
size_t m_size;  // Cached structural size (O(1) access)

// var class
size_t m_index;  // De Bruijn level

// func class
std::unique_ptr<expr> m_body;  // Function body

// app class  
std::unique_ptr<expr> m_lhs;   // Function being applied
std::unique_ptr<expr> m_rhs;   // Argument
```

**Example:**

```cpp
auto func_expr = f(v(0));
std::cout << "Size: " << func_expr->m_size << std::endl;

if(auto func_ptr = dynamic_cast<func*>(func_expr.get())) {
    std::cout << "Body size: " << func_ptr->m_body->m_size << std::endl;
}
```

#### Expression Deep Cloning

**Opt-in Immutability:** Since expressions mutate in place for efficiency, use `clone()` to preserve originals:

```cpp
std::unique_ptr<expr> clone() const;
```

- **Purpose**: Creates an independent deep copy for opt-in immutability
- **Use case**: Call before mutation when you need to preserve the original
- **Memory**: Allocates completely independent copy with own memory

**The Pattern: Clone Before Mutation**

```cpp
auto original = a(f(v(0)), v(5));
auto mutable_copy = original->clone();  // Explicit choice for immutability

while(reduce_one_step(mutable_copy));   // Only copy mutates

// Original is preserved
assert(original->equals(a(f(v(0)), v(5))));
// Copy is normalized
assert(mutable_copy->equals(v(5)));
```

**When to clone:**
- Before normalization if you need the original expression
- When reusing an expression in multiple contexts
- When `std::unique_ptr` ownership needs duplication

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
// Identity function: (λ.0) 5 → 5 (mutates in place)
auto identity = f(v(0));
auto expr = a(identity->clone(), v(5));

while(reduce_one_step(expr));
assert(expr->equals(v(5)));

// K combinator: ((λ.λ.0) 7) 8 → 7 (with step counting)
auto K = f(f(v(0)));
auto expr2 = a(a(K->clone(), v(7)), v(8));

size_t steps = 0;
while(reduce_one_step(expr2))
    ++steps;

assert(expr2->equals(v(7)));
assert(steps == 2);
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
auto omega = a(f(a(v(0), v(0))), f(a(v(0), v(0))));
auto omega_copy = omega->clone();  // Clone to preserve original

size_t step_count = 0;
while(step_count < 100 && reduce_one_step(omega_copy))
    ++step_count;

assert(step_count == 100);         // Hit step limit
assert(omega_copy->equals(omega)); // Omega reduces to itself

// Size limit
auto expr = a(f(a(v(0), v(0))), f(a(a(v(0), v(0)), v(0))));
while(expr->m_size <= 1000 && reduce_one_step(expr));

if(expr->m_size > 1000) {
    std::cout << "Stopped: expression too large\n";
}

// Custom control: combined limits
auto expr2 = a(a(a(f(f(f(v(0)))), v(1)), v(2)), v(3));
size_t steps = 0;
size_t max_size = 0;
while(steps < 100 && expr2->m_size <= 1000 && reduce_one_step(expr2)) {
    ++steps;
    max_size = std::max(max_size, expr2->m_size);
}

std::cout << "Steps: " << steps << ", Max size: " << max_size << "\n";
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
while(reduce_one_step(expr));  // Mutates in place
assert(expr->equals(v(10)));
```

#### Measuring Expression Size

```cpp
// Expression: (λ.0) 5
auto expr = a(f(v(0)), v(5));
std::cout << "Size: " << expr->m_size << std::endl;  // Direct access: 4
// Size breakdown: 1 (app) + 2 (func with var body) + 1 (var) = 4

while(reduce_one_step(expr));
std::cout << "Size after: " << expr->m_size << std::endl;  // Direct access: 1
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
- Lifting and substitution (in-place mutation)
- Beta-reduction with manual reduction loops
- Size calculation and synchronization
- Manual reduction loops with custom control
- In-place mutation and opt-in immutability patterns
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
