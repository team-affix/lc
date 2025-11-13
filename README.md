# lc

A minimal, efficient C++ implementation of lambda calculus with normal order reduction.

## What This Project Implements

This project provides a complete implementation of untyped lambda calculus, supporting:
- Lambda abstractions (functions)
- Variables with De Bruijn levels
- Function applications
- Beta-reduction with normal order evaluation strategy
- Expression normalization with customizable constraints

## Technical Details

### Reduction Strategy
- **Normal Order Reduction**: Leftmost-outermost redex is reduced first
- **Beta-Reduction Only**: Only β-reductions are performed (no η-reduction)
- **Call-by-Name**: Arguments are not reduced before substitution

### Variable Representation
- **De Bruijn Levels** (not indices): Variables reference binders by their level from the outermost scope
  - `v(0)` refers to the outermost lambda
  - `v(1)` refers to the next outermost lambda
  - `v(n)` refers to the nth lambda from outside
- This differs from De Bruijn indices where counting is from innermost scope

### Core Operations
- **Lifting**: Adjusts variable indices when moving expressions into deeper binding contexts
- **Substitution**: Replaces bound variables with argument expressions, handling:
  - Variable lifting for free variables in arguments
  - Variable decrementation for free variables in the body
  - Correct handling of capture-avoiding substitution

## Expression Types

Lambda calculus expressions are built from three fundamental types:

### `var` - Variable
Represents a variable reference using De Bruijn levels.
- **Example**: `v(0)` references the outermost lambda binder
- **Semantics**: A variable is bound if it refers to a lambda in scope, or free otherwise
- **Size**: Always 1

### `func` - Lambda Abstraction
Represents a function (lambda abstraction) with a single parameter and a body.
- **Example**: `f(v(0))` is the identity function (returns its argument), assuming it is declared at depth=0
- **Semantics**: Creates a new binding scope; the bound variable can be referenced in the body using the appropriate De Bruijn level
- **Size**: 1 + body_size

### `app` - Application
Represents function application (applying a function to an argument).
- **Example**: `a(f(v(0)), v(5))` applies the identity function to variable 5, (valid ID fxn assuming depth=0)
- **Semantics**: When lhs is a lambda, performs beta-reduction by substituting the argument into the function body
- **Size**: 1 + lhs_size + rhs_size

All expressions are:
- **Immutable**: Operations create new expressions rather than modifying existing ones
- **Polymorphic**: Accessed through the base `expr` interface
- **Memory-safe**: Managed via `std::unique_ptr`

## Features

### Expression Normalization

The `normalize()` method reduces expressions to normal form with several optional parameters:

```cpp
std::unique_ptr<expr> normalize(
    size_t* a_step_count = nullptr,           // Output: number of reductions performed
    size_t a_step_limit = SIZE_MAX,           // Input: maximum reductions allowed
    size_t* a_size_peak = nullptr,            // Output: peak expression size during reduction
    size_t a_size_limit = SIZE_MAX            // Input: maximum expression size allowed
) const;
```

#### Parameters:

1. **`a_step_count`** (output): Pointer to receive the number of beta-reductions performed
   - Returns the count even if a limit was exceeded
   - Useful for detecting non-termination or limit violations

2. **`a_step_limit`** (input): Maximum number of reduction steps
   - Prevents infinite loops (e.g., omega combinator)
   - Algorithm performs **at most** `a_step_limit` steps (never exceeds)
   - Detection: `step_count >= step_limit` means the limit was reached

3. **`a_size_peak`** (output): Pointer to receive the peak expression size
   - Tracks the maximum size reached during normalization
   - Useful for detecting exponential growth

4. **`a_size_limit`** (input): Maximum expression size allowed
   - Prevents exponential explosion during reduction
   - Normalization continues while `expr->size() <= a_size_limit`
   - Unlike step limit, reductions continue until size **exceeds** the limit

#### Limit Behavior:

**Step Limit:** Algorithm stops when reaching `a_step_limit` steps. Detection: `count >= step_limit`.

**Size Limit:** Algorithm continues until expression size exceeds `a_size_limit`. The returned expression and `peak` reflect the state **after** exceeding. Detection: `peak > size_limit`.

### Expression Size

Every expression has a `size()` method that returns its structural size:

```cpp
size_t size() const;
```

**Size calculation:**
- `var`: size = 1
- `func`: size = 1 + body_size
- `app`: size = 1 + lhs_size + rhs_size

This allows tracking expression growth during reduction and preventing size explosions.

### Expression Deep Cloning

Since expressions are managed by `std::unique_ptr`, the `clone()` method provides deep copying:

```cpp
std::unique_ptr<expr> clone() const;
```

- **Purpose**: Creates an independent deep copy of an expression
- **Use case**: Required when reusing an expression multiple times, as `std::unique_ptr` enforces single ownership
- **Example**: `auto copy = expr->clone();`

## Starter Examples

> Factory functions `v()`, `f()`, `a()` create expressions, and produce `std::unique_ptr<expr>`

### Basic Construction

```cpp
using namespace lambda;

// Lambda abstraction: λ.0 (identity function)
auto identity = f(v(0));

// Application: (λ.0) 5
auto expr = a(f(v(0)), v(5));
```

### Simple Reduction

```cpp
// Identity function: (λ.0) 5 → 5
auto identity = f(v(0));  // λ.0
auto expr = a(identity->clone(), v(5));
auto result = expr->normalize();
// result equals v(5)

// K combinator: ((λ.λ.0) 7) 8 → 7
auto K = f(f(v(0)));  // λ.λ.0
auto expr = a(a(K->clone(), v(7)), v(8));
auto result = expr->normalize();
// result equals v(7)
```

### Church Numerals

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

### Reduction with Constraints

```cpp
// Limit reduction steps
// Omega combinator: (λ.(0 0)) (λ.(0 0))
auto omega = a(f(a(v(0), v(0))), f(a(v(0), v(0))));
size_t count = 0;
auto result = omega->normalize(&count, 100);  // Stop after 100 steps
// count will be 100, omega reduces to itself indefinitely

// Limit expression size
// Growing combinator: (λ.(0 0)) (λ.(0 0 0))
auto growing = a(f(a(v(0), v(0))), f(a(a(v(0), v(0)), v(0))));
size_t peak = 0;
result = growing->normalize(nullptr, SIZE_MAX, &peak, 20);
// Stops when expression size exceeds 20

// Track both
size_t steps = 0;
size_t peak_size = 0;
result = expr->normalize(&steps, 1000, &peak_size, 100);
std::cout << "Steps: " << steps << ", Peak size: " << peak_size << std::endl;
```

### Combinator Identities

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
// result equals v(10)
```

### Measuring Size

```cpp
// Expression: (λ.0) 5
auto expr = a(f(v(0)), v(5));
std::cout << "Size: " << expr->size() << std::endl;  // Prints: 4
// Size breakdown: 1 (app) + 2 (func with var body) + 1 (var) = 4

auto normalized = expr->normalize();
std::cout << "Size after: " << normalized->size() << std::endl;  // Prints: 1
// Just v(5) remains
```

## Building

```bash
make
```

This creates:
- `build/liblc.a` - Static library

## Testing

The project includes comprehensive unit tests covering:
- Expression construction and equality
- Lifting and substitution
- Beta-reduction (one-step and normalization)
- Size calculation
- Reduction constraints (step and size limits)
- Combinator identities (I, K, S)
- Church numeral arithmetic

Build tests with:
```bash
make debug
```

Run tests with:
```bash
./build/main
```

## License

See LICENSE file for details.
