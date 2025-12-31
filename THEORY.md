# Logi: Theoretical Foundation Exploration

## Executive Summary

This document explores theoretical foundations for `logi`, a minimal logic verification language that unifies computation and proof. We examine several candidate calculi and work toward a consistent operational semantics.

---

## Core Design Insight (from your ideation)

Your key realization: **The `->` operator as a computational pattern matcher**

From your notes:
> "The arrow operator denotes pattern matching. When it is applied to an input, the input is checked to be exactly beta-equivalent to the lhs of the arrow, and if so, the rhs of the arrow will be produced."

This suggests a calculus where:
- Lambda provides abstraction (parameterization)
- Arrow provides pattern matching (case analysis)
- Both are first-class computational constructs
- The `theorem` keyword tags certain values for static analysis

---

## Theoretical Approach 1: Lambda Calculus + Pattern Arrows

### Abstract Syntax

```
Terms (M, N, P):
  M ::= x                    (variable)
      | a                    (atom)
      | λx. M                (abstraction)
      | M N                  (application)
      | M -> N               (pattern arrow)
      | M ; N                (pattern choice)
      | !M                   (quotation)
      | theorem M            (theorem marker)
```

### Informal Semantics

**Values:**
```
V ::= a | λx. M | (P -> M) | (V ; V) | !M | theorem V
```

**Beta reduction (standard):**
```
(λx. M) V  ⟹  M[x := V]
```

**Arrow reduction (pattern matching):**
```
(P -> R) V  ⟹  R    if V ≡ₐ P  (alpha-equivalence/beta-normal forms are equal)
(P -> R) V  ⟹  STUCK  otherwise
```

**Choice reduction (sequential pattern matching):**
```
(M₁ ; M₂) V  ⟹  M₁ V    if M₁ V doesn't get stuck
(M₁ ; M₂) V  ⟹  M₂ V    if M₁ V gets stuck
```

### Example: Boolean Logic

```
true  = !true_atom
false = !false_atom

not = (true -> false) ; (false -> true)

// Reduction:
not true
= ((true -> false) ; (false -> true)) true
= ((true -> false) true) ; ...     [try first branch]
= false                             [pattern matches]
```

### The `(bool x)` Problem - Solution Attempt

```
and = λa. λb. a ⟹
  | (!bool false) -> (!bool false)
  | (!bool true)  -> b

// Usage:
and (!bool true) (!bool false)
= (λa. λb. <pattern matcher>) (!bool true) (!bool false)
= (λb. <pattern matcher>[a := (!bool true)]) (!bool false)
= <pattern matcher>[a := (!bool true), b := (!bool false)]
```

**Problem:** How do we extract the `false` from `(!bool false)`?

The pattern `(!bool false)` matches the entire structure, but doesn't bind a variable to the inner part.

**Revised attempt with pattern variables:**
```
and = λa. λb. a ⟹
  | (!bool false) -> (!bool false)
  | (!bool true)  -> (b ⟹
      | (!bool x) -> (!bool x))
```

This works! Inner pattern match on `b` extracts `x` from `(!bool x)`.

But this is getting verbose. Is there a better way?

---

## Theoretical Approach 2: Pattern-Lambda Unification

### Idea: Lambda can bind in patterns

What if we allow lambdas to introduce variables that are matched, not just supplied?

```
M ::= x | a | λp. M | M N | theorem M | !M

p ::= x                    (variable pattern - binds)
    | a                    (atom pattern - must match exactly)
    | (p p)               (application pattern - recurse)
    | !p                   (quoted pattern)
```

**Lambda with pattern:**
```
λp. M
```

When applied to a value V:
1. Try to match V against pattern p
2. If match succeeds with bindings σ, return M[σ]
3. If match fails, STUCK

**Multiple branches:**
Use sequencing `M₁ | M₂` to try alternatives.

### Example: Boolean Logic

```
not = (λtrue. false) | (λfalse. true)

// Reduction:
not true
= ((λtrue. false) | (λfalse. true)) true
= (λtrue. false) true             [try first]
= false                            [true matches atom true]
```

### The `(bool x)` Problem - Solution

```
and = λa. λb. 
  (λ(!bool false). (!bool false))  |
  (λ(!bool true). λ(!bool x). (!bool x))

// Simplified with implicit branching:
and = λ(!bool false). λb. (!bool false)
    | λ(!bool true). λ(!bool x). (!bool x)
```

When we write `λ(!bool x). M`, this:
- Matches values of form `(!bool <something>)`
- Binds `x` to `<something>`
- Evaluates to `M` with `x` bound

**Advantage:** Only ONE binding mechanism (lambda), but lambdas can do pattern matching.

**Disadvantage:** Multiple branches syntax unclear. Do we need `|` operator? Is that a primitive?

---

## Theoretical Approach 3: Guarded Lambda Calculus

### Idea: Lambda + explicit guard syntax

```
M ::= x | a | λx. M | M N | theorem M | !M
    | match M { case p -> M; ... }

p ::= x | a | (p p) | !p
```

**Lambda** for parameterization (must supply argument).
**Match** for pattern analysis (inspect structure).

### Example: Boolean Logic

```
not = λb. match b {
  case true -> false;
  case false -> true;
}
```

### The `(bool x)` Problem - Solution

```
and = λa. λb. match a {
  case (!bool false) -> (!bool false);
  case (!bool true) -> match b {
    case (!bool x) -> (!bool x);
  };
}
```

**Advantage:** Clear separation of concerns - lambda for parameters, match for branching.

**Disadvantage:** Two constructs instead of one. Is this minimal?

---

## Theoretical Approach 4: Your Original Vision (Refined)

Let me formalize what I think you were driving at in DOC 1.

### Core Calculus: Lambda + Arrow + Choice

```
M ::= x                    (variable)
    | a                    (atom)  
    | λx. M                (abstraction - parameterization)
    | M N                  (application)
    | M -> M               (arrow - pattern match on equal values)
    | M ; M                (choice - try alternatives)
    | !M                   (quote - prevent reduction)
    | theorem M            (theorem marker)
```

### Key Insight: Arrows are Values

An arrow `P -> R` is itself a value (a function) that when applied to an argument:
1. Checks if argument is structurally equal to P (after normalization)
2. If yes, returns R
3. If no, STUCK

Choice `;` combines functions: try first, if stuck, try second.

### Operational Semantics (Small-Step)

**Evaluation contexts:**
```
E ::= □ | E M | V E | E -> M | V -> E | E ; M
```

**Reduction rules:**

```
(λx. M) V  ⟹  M[x := V]                           [BETA]

(V₁ -> R) V₂  ⟹  R          if V₁ ≡ᵦ V₂         [ARROW-MATCH]
(V₁ -> R) V₂  ⟹  stuck      if V₁ ≢ᵦ V₂         [ARROW-FAIL]

(M₁ ; M₂) V  ⟹  M₁ V                              [CHOICE-TRY]
stuck ; M₂   ⟹  M₂                                [CHOICE-RECOVER]
```

Where `V₁ ≡ᵦ V₂` means "beta-equivalent" (same normal form).

### Example Reductions

**Boolean NOT:**
```
not = (true -> false) ; (false -> true)

not true
= ((true -> false) ; (false -> true)) true
= (true -> false) true ; ...                [CHOICE-TRY]
= false                                      [ARROW-MATCH, since true ≡ᵦ true]
```

**Boolean AND:**
```
// First attempt - doesn't handle extraction
and = λa. λb. (
  (true -> b) ;
  (false -> false)
) a

// Problem: we want to TYPE-CHECK that b is a bool!
```

### The Variable Binding Problem

With `P -> R` where P is checked for beta-equivalence, we can't bind variables in P!

```
(x -> foo) something    // x is a variable - matches anything?
```

If `x` is a variable, it would match ANY value and bind x to it. But then checking beta-equivalence doesn't make sense - x isn't a value to compare against!

**Implication:** Pattern arrows `P -> R` require P to be a closed term (no free variables).

**But then how do we extract substructure?**

This is the core tension.

---

## Theoretical Approach 5: Dual Arrows (Computational + Pattern)

### Idea: Two kinds of arrows

```
M ::= x | a | λx. M | M N | !M | theorem M
    | M -> M              (computation arrow - requires exact match)
    | case M of { p => M; ... }    (pattern arrow - binds variables)

p ::= x | a | (p p) | !p
```

**Computation arrow** (`->`): Used for defining rewrite rules on closed terms.
**Pattern case** (`case...of`): Used for destructuring with variable binding.

### Example: Boolean Logic

```
not = (true -> false) ; (false -> true)      // computational

and = λa. λb. case a of {
  (!bool false) => (!bool false);
  (!bool true) => case b of {
    (!bool x) => (!bool x);
  };
}                                              // pattern matching
```

**Advantage:** Each construct has clear purpose.

**Disadvantage:** Now we have THREE mechanisms: lambda, comp-arrow, pattern-case.

---

## Deep Dive: What is Minimal?

Let me examine what we ACTUALLY need:

### Required Capabilities

1. **Abstraction** - defining functions with parameters
2. **Application** - calling functions
3. **Branching** - choosing between alternatives
4. **Destructuring** - extracting subparts of structures
5. **Construction** - building compound values
6. **Quotation** - controlling evaluation (for atoms, theorems)

### Can We Collapse Any?

**Lambda provides:** abstraction + application
**Pattern matching provides:** branching + destructuring

Can we unify them?

#### Option A: Lambda-only

Make lambda powerful enough to do pattern matching.

```
λp. M    where p can be a pattern
```

But how do we specify multiple branches? Need some operator like `|`.

```
(λtrue. false) | (λfalse. true)
```

Is `|` a primitive? If so, we have lambda + choice operator.

#### Option B: Pattern-only  

Make pattern matching powerful enough to do abstraction.

```
x => M    is a function taking one argument

(x => M) V  ⟹  M[x := V]
```

Multiple branches:
```
{ p₁ => M₁; p₂ => M₂; ... }
```

This looks like Approach 3 (match syntax).

**But** we still need some way to make multi-argument functions (currying):

```
x => (y => M)
```

So this is basically lambda with pattern syntax.

#### Option C: Arrow-only?

Can we build everything from arrows?

```
Lambda:     λx. M  ≈  ???
```

Not obvious how to encode lambda using `P -> R` where P must be closed.

---

## Resolution Attempt: The Matching Lambda

### Proposal: Unify lambda and pattern matching into one construct

```
M ::= x | a | M N | !M | theorem M
    | { p₁ => M₁ | p₂ => M₂ | ... }

p ::= x | a | (p p) | !p
```

**Semantics:**

A term `{ p₁ => M₁ | ... | pₙ => Mₙ }` is a function value.

When applied to an argument V:
1. Try to match V against p₁
2. If match succeeds with bindings σ, reduce to M₁[σ]
3. If match fails, try p₂
4. Continue until a pattern matches
5. If no pattern matches, STUCK

**Lambda as syntactic sugar:**
```
λx. M  ≡  { x => M }
```

**Concrete syntax example:**
```
not = { true => false | false => true }

and = { 
  (!bool false) => { _ => (!bool false) } |
  (!bool true) => { (!bool x) => (!bool x) }
}
```

**Advantage:** Single mechanism for both abstraction and pattern matching.

**Disadvantage:** Multi-argument functions require nesting:
```
{ x => { y => M } }
```

But this is exactly how currying works anyway!

---

## The Quotation Issue

Your DOC 1 mentions `!` for quotation. What is its purpose?

### Hypothesis 1: Prevent Reduction

```
!M  =  a quoted value that doesn't reduce further
```

Used for atoms:
```
true = !true_atom
```

### Hypothesis 2: Reification (Make Syntax Available)

```
!M  =  the syntactic structure of M as data
```

This would allow meta-programming:

```
check_structure = λx. case x of {
  !(theorem p) => p;
  _ => error;
}
```

### Hypothesis 3: Both

Quotation serves dual purpose:
- At the base level: atoms are quoted (opaque values)
- At meta level: can quote arbitrary terms to inspect structure

### Your Example from DOC 1

```
mkeq = λx. λy. (x -> !(eq x y)) y
```

Analysis:
- `x` and `y` are parameters
- `(x -> !(eq x y))` is a pattern arrow
- When applied to `y`, it checks if `y ≡ x`
- If so, produces `!(eq x y)` - a quoted equality term

This seems to use quotation to BUILD syntax as data.

So `!(eq x y)` constructs the term structure `(eq x y)` where x and y are substituted in.

**Implication:** Quotation is like quasi-quotation in Lisp - build syntax trees.

---

## Refined Proposal: The Logi Calculus

Synthesizing insights:

### Syntax

```
Terms (M, N):
  M ::= x                          (variable)
      | a                          (atom literal)
      | { p => M | ... }          (matching function)
      | M N                        (application)
      | !(M)                       (quotation - builds syntax)
      | theorem M                  (theorem marker)

Patterns (p):
  p ::= x                          (variable - binds)
      | a                          (atom - must match)
      | _                          (wildcard - matches anything, doesn't bind)
      | (p p)                      (application pattern)
      | !(p)                       (quoted pattern)
      | theorem p                  (theorem pattern)
```

### Values

```
V ::= a 
    | { p => M | ... }
    | !(M)          (quoted terms are values)
    | theorem V
```

### Reduction Rules

**Application of matching function:**
```
{ p₁ => M₁ | ... | pₙ => Mₙ } V
```

Try each pattern pᵢ in order:
- If V matches pᵢ with bindings σ, reduce to Mᵢ[σ]
- If no pattern matches, STUCK

**Application (left-to-right evaluation):**
```
M N  ⟹  M' N      if M ⟹ M'
V N  ⟹  V N'      if N ⟹ N'
```

**Quotation:**
- Quoted terms `!(M)` are values (don't reduce)
- Inside quotes, perform substitution but not reduction:
  ```
  (λx. !(foo x)) bar  ⟹  !(foo bar)
  ```

### Examples

**Boolean Logic:**
```
true  = !true
false = !false

not = { !true => !false | !false => !true }

and = {
  !false => { _ => !false } |
  !true  => { x => x }
}
```

**Modus Ponens:**
```
implies = λp. λq. !(implies p q)

mp = {
  (theorem !(implies p q)) => {
    (theorem p) => theorem q
  }
}
```

Usage:
```
a = theorem !(implies P Q)    // axiom: P implies Q
b = theorem !P                 // axiom: P

c = mp a b                     // derive: theorem Q
```

Reduction:
```
mp a b
= mp (theorem !(implies P Q)) (theorem !P)
= { (theorem !(implies p q)) => ... } (theorem !(implies P Q)) (theorem !P)
= { (theorem p) => theorem q }[p := !P, q := !Q] (theorem !P)
= { (theorem !P) => theorem !Q } (theorem !P)
= theorem !Q
```

**The `(bool x)` Problem - Solved:**
```
bool = λx. !(bool x)

and = {
  !(bool !false) => { _ => !(bool !false) } |
  !(bool !true)  => { !(bool x) => !(bool x) }
}
```

Usage:
```
and (bool true) (bool false)
= and !(bool !true) !(bool !false)
= { !(bool !true) => ... } !(bool !true) !(bool !false)
= { !(bool x) => !(bool x) } !(bool !false)
= !(bool !false)
```

Perfect! The pattern `!(bool x)` matches `!(bool !false)` and binds `x := !false`.

---

## Static Analysis for Axioms

Your insight: detect axiom declarations by looking for `theorem` keyword in unreduced syntax.

With this calculus:

**Axiom (direct theorem claim):**
```
a = theorem !P
```

The term literally contains `theorem` at top level (before any reduction).

**Derived theorem (uses inference rule):**
```
b = mp axiom1 axiom2
```

This doesn't directly contain `theorem` - it will reduce to produce one.

Static analyzer:
1. Parse all definitions
2. Find definitions whose RHS syntactically contains `theorem` at root
3. These are axiomatic claims
4. Everything else is derived (computational)

This gives soundness: you can't sneak in theorems without either:
1. Declaring them axiomatically (using `theorem` keyword)
2. Deriving them from existing theorems via computation

---

## Addressing Your Concerns from DOC 2

### "Can we have just ONE binding mechanism?"

**Answer:** Yes - the matching function `{ p => M | ... }`.

Lambda is sugar: `λx. M` ≡ `{ x => M }`.

### "If we have two ways of introducing variables, is that too much?"

**Answer:** We have one way: pattern binding in `{ p => M }`.

Variables in patterns are the ONLY way to introduce new bindings.

### "You will not make it so we have 'pretty syntax' that boils down to something more complex."

**Answer:** The matching function IS the primitive.

If we offer `λx. M` as sugar, it's purely for familiarity/readability, and directly corresponds to `{ x => M }` with no hidden complexity.

We could even mandate users write `{ x => M }` and never mention lambda!

### "Should we use ONLY destructuring?"

**Answer:** In this calculus, yes.

All variable binding happens through pattern matching (destructuring).

Even "parameter passing" is just matching against a variable pattern.

---

## Advantages of This Calculus

1. **Minimal:** One binding mechanism (pattern matching)
2. **Uniform:** Functions, branching, destructuring all use same construct
3. **Expressive:** Can encode first-order logic, user-defined inference rules
4. **Integrated:** Computation and proof use identical machinery
5. **Transparent:** No hidden complexity, no sugar layers
6. **Beginner-friendly:** Pattern matching is intuitive
7. **Static analysis:** `theorem` keyword enables soundness checking

---

## Remaining Questions

### 1. Quotation Semantics - Exact Behavior

When we write:
```
f = λx. !(foo x)
f bar
```

Should this reduce to:
- `!(foo bar)` - substitution happens inside quotes, OR
- `!(foo x)` - quotes are completely opaque?

**Proposal:** Substitution happens, but not reduction.

```
!(foo x)[x := bar] = !(foo bar)
!(foo (id bar))    = !(foo (id bar))    [NOT !(foo bar)]
```

Quotation freezes reduction but allows variable capture.

### 2. Atoms vs. Quoted Terms

Are atoms `a` the same as quoted atoms `!a`?

**Option A:** Atoms are distinct from quoted terms.
```
a     - an atom (primitive value)
!a    - a quoted atom (reified syntax of atom)
```

**Option B:** Atoms are automatically quoted.
```
a  =  !a   (atoms are opaque by nature)
```

**Proposal:** Option B for simplicity.

Every atom is implicitly quoted. Writing `true` is the same as `!true`.

### 3. Recursive Definitions

How do we define recursive functions?

```
factorial = { 0 => 1 | n => n * (factorial (n - 1)) }
```

The name `factorial` appears in its own definition.

**Options:**
1. **Fixed-point combinator** (Y combinator) - can derive in calculus
2. **Let-rec construct** - add `letrec x = M in N` to syntax
3. **Top-level recursion** - definitions can refer to themselves by name

**Proposal:** Option 3 for practical usability.

Top-level definitions are implicitly recursive. Internal recursion requires fix-point combinator.

### 4. Arithmetic and Primitives

Do we have built-in arithmetic, or Church encode?

**Church encoding:** Possible but painful for users.

**Built-in primitives:** Pragmatic choice.

```
0, 1, 2, ...     (number literals)
+, -, *, /       (arithmetic operations)
<, >, ==         (comparisons)
```

These could be atoms with special reduction rules.

**Proposal:** Start with Church encoding for purity, add primitives as "standard library" after core calculus is proven.

### 5. Error Handling

What happens when pattern match fails?

```
not 42  =  ???
```

**Options:**
1. **STUCK** - computation gets stuck (no reduction possible)
2. **Error value** - reduce to special `error` atom
3. **Exception** - runtime error in interpreter

**Proposal:** STUCK for theoretical model. Interpreter can detect and report as error.

---

## Formal Specification (Draft)

### Abstract Syntax

```
Terms:
  M, N ::= x                     (variable)
         | a                     (atom)
         | { p₁ => M₁ | ... }   (matching function)
         | M N                   (application)
         | theorem M             (theorem marker)

Patterns:
  p ::= x                        (variable pattern)
      | a                        (atom pattern)
      | _                        (wildcard)
      | (p₁ p₂)                  (application pattern)
      | theorem p                (theorem pattern)
```

### Pattern Matching

Match a value V against pattern p, producing bindings:

```
match(a, a) = {}                               (atom matches itself)
match(a, b) = FAIL                             (different atoms)
match(V, x) = {x ↦ V}                          (variable matches anything)
match(V, _) = {}                               (wildcard matches, no binding)
match((V₁ V₂), (p₁ p₂)) = σ₁ ∪ σ₂             (if σ₁ = match(V₁, p₁), σ₂ = match(V₂, p₂))
match((theorem V), (theorem p)) = match(V, p)  (recurse into theorem)
match(V, p) = FAIL                             (otherwise)
```

### Small-Step Operational Semantics

Values:
```
V ::= a | { p => M | ... } | theorem V
```

Reduction relation M ⟹ M':

```
[APP-MATCH-SUCCESS]
{ p₁ => M₁ | ... | pₙ => Mₙ } V  ⟹  Mᵢ[σ]
  where match(V, pᵢ) = σ (first successful match)

[APP-LEFT]
M N ⟹ M' N
  where M ⟹ M'

[APP-RIGHT]
V N ⟹ V N'
  where N ⟹ N'

[THEOREM]
theorem M ⟹ theorem M'
  where M ⟹ M'
```

### Type System (Optional - For Soundness)

Could add a simple sort system:

```
Sorts:
  τ ::= Term | Prop | ★

Judgments:
  Γ ⊢ M : τ

Rules:
  Γ ⊢ M : Term
  ─────────────────
  Γ ⊢ theorem M : Prop
```

But this violates "no type system" goal. Skip for now.

---

## Proof of Concept: First-Order Logic

### Propositional Connectives

```
// Atoms for true/false
T = T
F = F

// Negation
not = { T => F | F => T }

// Conjunction
and = { T => { x => x } | F => { _ => F } }

// Disjunction
or = { T => { _ => T } | F => { x => x } }

// Implication
implies = λp. λq. or (not p) q
```

### Inference Rules

```
// Modus ponens
mp = {
  (theorem (implies p q)) => {
    (theorem p) => theorem q
  }
}

// Modus tollens
mt = {
  (theorem (implies p q)) => {
    (theorem (not q)) => theorem (not p)
  }
}

// Hypothetical syllogism
hs = {
  (theorem (implies p q)) => {
    (theorem (implies q r)) => theorem (implies p r)
  }
}
```

### Quantifiers (Sketch)

```
forall = λx. λp. (forall x p)    // reified as data structure

// Universal instantiation
ui = {
  (theorem (forall x p)) => {
    t => theorem (substitute p x t)
  }
}
```

This would require `substitute` as a meta-level operation.

---

## Comparison to Design Goals

### ✓ Minimalism

Single binding mechanism (pattern matching).

### ✓ User-defined inference rules

Rules are just functions on theorems.

### ✓ Tight integration

Same calculus for computation and proof.

### ✓ Beginner-friendly

Pattern matching is intuitive, no universe hierarchies.

### ✓ No dependent types

Propositions are data, not types.

### ✓ Avoid backtracking

Pattern matching is sequential, deterministic.

### ✓ No hidden complexity

Matching function is the actual primitive.

### ✓ Single mechanism for variables

Pattern binding only.

---

## Alternative: Keeping Your Arrow Operator

If you really like the `P -> R` syntax from DOC 1:

### Variant Syntax

```
M ::= x | a | M N | M -> M | M ; M | theorem M
```

**Interpretation:** Arrow as a matching function with one branch.

```
(P -> R)  ≡  { p => R }  where p is the pattern form of P
```

**Chaining:**

```
(P₁ -> R₁) ; (P₂ -> R₂)  ≡  { p₁ => R₁ | p₂ => R₂ }
```

**Example:**

```
not = (T -> F) ; (F -> T)
```

This is purely syntactic sugar over matching functions.

**Advantage:** Closer to your original intuition.

**Question:** How do we distinguish pattern variables from value variables?

```
(x -> foo)    // Is x a pattern variable or value to match?
```

If x is a value variable (free in context), we'd need to evaluate it first:

```
f = λx. (x -> bar)
f T
= (T -> bar)     // x substituted
```

If x is always a pattern variable, we can't write parameterized arrows.

**Resolution:** Variables in arrow LHS are always patterns.

To match against a parameter, use explicit equality:

```
f = λx. { y => (if (eq x y) bar stuck) }
```

But this requires `if` and `eq` primitives...

**Conclusion:** The matching function `{ p => M }` is clearer.

---

## Implementation Sketch

### Interpreter Structure

```python
# AST
class Term:
    pass

class Var(Term):
    def __init__(self, name): ...

class Atom(Term):
    def __init__(self, name): ...

class Match(Term):
    def __init__(self, branches): ...  # [(pattern, body)]

class App(Term):
    def __init__(self, func, arg): ...

class Theorem(Term):
    def __init__(self, prop): ...

# Pattern matching
def match(value, pattern):
    # returns bindings dict or None

# Evaluation
def reduce(term, env):
    if isinstance(term, App):
        func = reduce(term.func, env)
        arg = reduce(term.arg, env)
        if isinstance(func, Match):
            for (pattern, body) in func.branches:
                bindings = match(arg, pattern)
                if bindings is not None:
                    return reduce(body, {**env, **bindings})
            raise StuckError()
    # ...

# Static analysis
def contains_theorem_keyword(term):
    # Check if theorem appears syntactically
    ...
```

### Complexity

- Parser: ~200 lines
- Matcher: ~100 lines
- Evaluator: ~150 lines
- Static analyzer: ~50 lines

**Total: ~500 lines for core interpreter.**

This is extremely minimal!

---

## Open Design Decisions

### 1. Concrete Syntax

What should users actually write?

**Option A: S-expressions (Lisp-style)**
```
(define not
  (match
    (T -> F)
    (F -> T)))
```

**Option B: ML-style**
```
not = { T => F | F => T }
```

**Option C: Custom**
```
not:
  | T => F
  | F => T
```

**Recommendation:** Option B (ML-style) is familiar and clean.

### 2. Module System

Deferred, but eventually need:
- Imports/exports
- Namespacing
- Separate compilation

### 3. Standard Library

What should be built-in vs user-defined?

**Proposed built-ins:**
- Atoms: true, false
- Propositional connectives: not, and, or, implies, iff
- Quantifiers: forall, exists
- Equality: eq

**User-defined:**
- Domain-specific predicates
- Custom inference rules
- Tactics and proof strategies

---

## Next Steps for Theory Development

### 1. Formalize Small-Step Semantics

Write complete reduction rules with evaluation contexts.

### 2. Prove Meta-Theorems

- **Progress:** Well-formed terms either reduce or are values
- **Preservation:** Reduction preserves well-formedness
- **Soundness:** Can't derive `theorem F` unless asserted axiomatically

### 3. Design Surface Syntax

Concrete syntax for users to write.

### 4. Build Prototype Interpreter

Implement in Python/OCaml/Haskell to validate design.

### 5. Test Expressiveness

Encode:
- Propositional logic
- First-order logic
- Peano arithmetic
- Simple proof examples

### 6. Refine Based on Usage

Iterate on design based on writing real proofs.

---

## Conclusion: Proposed Foundation

**The Logi Calculus:**

- **One binding mechanism:** Pattern matching via `{ p => M | ... }`
- **Uniform functions:** All functions are matching functions
- **Integrated proof:** `theorem M` tags computational values as propositions
- **Static soundness:** Axioms detected via syntactic analysis
- **Minimal syntax:** 5 core constructs (var, atom, match, app, theorem)
- **No backtracking:** Sequential pattern matching
- **No unification:** Simple structural matching
- **Beginner-friendly:** Pattern matching is intuitive

This foundation achieves all stated goals while remaining theoretically clean and practically implementable.

---

## Appendix: Detailed Example

### Proving Modus Ponens in Detail

```
// Axioms
axiom1 = theorem (implies A B)
axiom2 = theorem A

// Inference rule
mp = {
  (theorem (implies p q)) => {
    (theorem p) => theorem q
  }
}

// Derivation
result = mp axiom1 axiom2
```

**Step-by-step reduction:**

```
mp axiom1 axiom2

= { (theorem (implies p q)) => { (theorem p) => theorem q } }
  (theorem (implies A B))
  (theorem A)

= { (theorem (implies p q)) => { (theorem p) => theorem q } }
  (theorem (implies A B))
  (theorem A)

[Apply matching function to first argument]
match((theorem (implies A B)), (theorem (implies p q)))
= match((implies A B), (implies p q))
= match(implies, implies) ∪ match(A, p) ∪ match(B, q)
= {} ∪ {p ↦ A} ∪ {q ↦ B}
= {p ↦ A, q ↦ B}

= { (theorem p) => theorem q }[p ↦ A, q ↦ B] (theorem A)

= { (theorem A) => theorem B } (theorem A)

[Apply matching function to argument]
match((theorem A), (theorem A))
= {}

= theorem B

[Final result]
result = theorem B
```

Perfect! We derived `theorem B` from our axioms.

---

End of theoretical exploration.
