# Pattern Matching Directionality and Free Metavariables

## Core Principle: Semantic Stability

**Pattern matching should only DESTRUCTURE (read), never CONSTRUCT (modify).**

**Information flow: Value → Pattern Variables (one-way)**

---

## The Directionality Rule

### Case 1: Atom Pattern vs Variable Argument

```logi
f = a => b
f A    // Does this reduce?
```

**Pattern:** `a` (concrete atom)  
**Argument:** `A` (variable)

**To match:** We'd need to instantiate `a` with `A`, making the pattern become `A`.

**This modifies the function's pattern! ✗**

**Result:** Does NOT reduce.

**Why?** We'd be changing the value (the function) to match the argument. Information would flow backwards: Argument → Pattern.

---

### Case 2: Variable Pattern vs Atom Argument

```logi
f = A => b
f a    // Does this reduce?
```

**Pattern:** `A` (pattern variable)  
**Argument:** `a` (concrete atom)

**To match:** We bind pattern variable `A` to value `a`.

**This is proper destructuring! ✓**

**Result:** DOES reduce to `b` (with substitution `A = a`).

**Why?** We're reading the value `a` into the pattern variable `A`. Information flows correctly: Value → Pattern Variable.

---

## The General Rule

**Pattern matching succeeds when:**
- Pattern variables in the pattern get bound to concrete values
- No modification of the matched value is required

**Pattern matching fails when:**
- Matching would require instantiating part of the value
- Information would need to flow from pattern to value

---

## Examples

### Example 1: Matching Concrete Against Concrete

```logi
f = a => b
f a    // Pattern `a` matches argument `a` ✓
// Result: reduces to `b`

f c    // Pattern `a` doesn't match argument `c` ✗
// Result: doesn't reduce
```

### Example 2: Variable Pattern (Always Matches)

```logi
f = A => (square A)
f a       // A binds to a → (square a) ✓
f b       // A binds to b → (square b) ✓
f five    // A binds to five → (square five) ✓
```

Pattern variable matches ANY value.

### Example 3: Atom Pattern (Only Matches That Atom)

```logi
f = a => (square a)
f a       // Matches! → (square a) ✓
f b       // Doesn't match → (f b) ✗
f X       // Doesn't match → (f X) ✗
```

Atom pattern only matches that specific atom.

### Example 4: Complex Patterns

```logi
f = (cons X Y) => (plus X Y)

f (cons two three)    // X=two, Y=three → (plus two three) ✓
f (cons A B)          // X=A, Y=B → (plus A B) ✓
f five                // Doesn't match (not a cons) → (f five) ✗
```

Pattern `(cons X Y)` destructures the argument.

### Example 5: The Problem Case

```logi
f = (cons a b) => result

f (cons A B)    // Would need to bind a=A, b=B
                // But a and b are atoms in the pattern!
                // This would modify the function's pattern ✗
                // Result: doesn't reduce
```

**This is the key insight:** We can't match atom patterns against variable arguments because that would require modifying the pattern.

---

## Implications for Function Application

### Normal Application (Variable Pattern)

```logi
id = X => X
id five    // X=five → five ✓
```

Pattern variable `X` accepts anything - this is a **total function**.

### Constrained Application (Atom Pattern)

```logi
only_five = five => true
only_five five    // Matches → true ✓
only_five six     // Doesn't match → (only_five six) ✗
```

Atom pattern only matches that specific value - this is a **partial function**.

### The Asymmetry

```logi
f = a => X    // Pattern: atom `a`, Body: variable X

f a    // Matches! a=a, but what is X? 
       // X is a FREE VARIABLE in the body
       // Result: X (with X unbound)

f b    // Doesn't match (b ≠ a) → (f b) ✗
```

Wait, this is interesting. `X` in the body is a free variable, not a pattern variable. Let me think about this more carefully...

Actually, let me reconsider. In `a => X`, is `X` a:
1. Pattern variable (from some outer scope)?
2. Free variable?
3. A value variable?

This connects to your question about free metavariables!

---

## The Free Metavariable Question

### What Are Metavariables?

In the context of pattern matching, **metavariables** (pattern variables) are introduced during destructuring:

```logi
f = (cons X Y) => (plus X Y)
```

Here, `X` and `Y` are metavariables that get bound when we apply `f` to an argument.

### The Question: Can Metavariables Be Free?

**Scenario 1: Metavariable introduced but not bound**

```logi
term = (plus X Y)    // X and Y are free
```

**What does this mean?**
- In logic programming (Prolog): X and Y are logic variables to be solved for
- In functional programming: This is an error (undefined variables)
- In theorem provers: X and Y are schematic variables (universally quantified)

### The Question: Can Metavariables Escape Their Binding Scope?

**Scenario 2: Metavariable bound in pattern, but used outside that scope**

```logi
extract = (cons X Y) => X    // X is bound in the pattern
result = extract (cons a b)   // X is bound to a, then returned
// result = a ✓
```

This is fine - X is bound in the pattern's scope and used in the body.

**Scenario 3: What if the argument doesn't match?**

```logi
extract = (cons X Y) => X
result = extract five    // Pattern doesn't match!
// result = (extract five) - doesn't reduce
```

No issue - the function doesn't reduce, so X never gets bound.

**Scenario 4: Free variable in body**

```logi
f = a => X    // X is FREE in the body (not bound by pattern)
result = f a
// result = X (but what is X?)
```

**This is the critical question!**

---

## Three Interpretations of Free Variables

### Interpretation 1: Free Variables Are Errors (Functional Programming)

**Rule:** All variables in a term must be bound by:
- Pattern matching
- Let bindings (if we had them)
- Top-level definitions

**If we encounter a free variable:** Error (undefined variable)

```logi
f = a => X    // ERROR: X is not bound
```

**Pros:**
- Simple and safe
- Catches typos
- Clear semantics

**Cons:**
- Restrictive
- Can't represent "holes" or unknowns

---

### Interpretation 2: Free Variables Are Logic Variables (Logic Programming)

**Rule:** Free variables represent unknowns that can be instantiated through unification.

```logi
term = (plus X five)    // X is a logic variable
```

When this term is used in pattern matching or computation, X could get bound.

**Example:**
```logi
// Solve: (plus X five) = (plus three five)
// Unification: X = three
```

**Pros:**
- Very expressive
- Natural for logic programming
- Enables meta-reasoning

**Cons:**
- Complex semantics
- Need unification algorithm
- Unclear when variables get bound

---

### Interpretation 3: Free Variables Are Schematic (Theorem Proving)

**Rule:** Free variables are implicitly universally quantified at the top level.

```logi
term = (plus X Y)    // Implicitly: ∀X. ∀Y. (plus X Y)
```

**Example:**
```logi
theorem (eq (plus X Y) (plus Y X))    // For all X, Y: commutativity
```

**Pros:**
- Natural for theorem proving
- Concise notation
- Standard in mathematics

**Cons:**
- Implicit quantification can be confusing
- Need to track scope carefully
- Mixing with computation is tricky

---

## My Analysis: What Should Logi Do?

### Consider the Design Goals

1. **Logic programming language** - suggests logic variables might be useful
2. **Pattern matching calculus** - pattern variables are fundamental
3. **Minimal and consistent** - should have clear, simple rules
4. **Theorem proving** - schematic variables are natural

### The Tension

**In function bodies:**
```logi
f = (cons X Y) => (plus X Y)    // X, Y bound by pattern - clear!
```

**In top-level terms:**
```logi
term = (plus X Y)    // X, Y free - what does this mean?
```

### Option A: Forbid Free Variables (Strict Functional)

**Rule:** All variables must be bound.

**Enforcement:** Static check - every variable must trace back to:
- A pattern binding
- A top-level definition

**Impact:**
```logi
f = a => X        // ERROR: X not bound
term = (plus X Y)  // ERROR: X, Y not bound

// Must write:
f = a => x        // x is a defined constant (atom)
```

**Verdict:** Simple and safe, but might be too restrictive for logic programming.

---

### Option B: Free Variables Are Logic Variables (Prolog-like)

**Rule:** Free variables are unknowns that persist until bound through unification.

**Semantics:**
```logi
term = (plus X five)    // X is unbound
// Later, through unification:
// term = other_term might bind X
```

**Impact:**
```logi
f = (cons X Y) => (plus X Y)
g = (cons A B) => (times A B)

// What if we match f against g?
// (cons X Y) => (plus X Y)  vs  (cons A B) => (times A B)
// Would X=A, Y=B, and (plus X Y) = (times A B)?
// This is getting complex...
```

**Verdict:** Very powerful but complex. Requires full unification, which goes beyond pattern matching.

---

### Option C: Free Variables Are Schematic (Top-Level Only)

**Rule:** 
- In function bodies: variables must be bound by patterns
- At top-level definitions: free variables are implicitly universal

**Semantics:**
```logi
// In function body - must be bound:
f = (cons X Y) => (plus X Y)    // X, Y bound by pattern ✓
g = a => X                      // ERROR: X not bound by pattern ✗

// At top level - implicitly universal:
theorem (eq (plus X Y) (plus Y X))    // ∀X, Y: commutativity ✓
```

**Enforcement:**
- Check that variables in function bodies are bound by their patterns
- Allow free variables in top-level definitions (implicitly ∀)

**Impact:**
```logi
// Valid:
comm = theorem (eq (plus X Y) (plus Y X))    // X, Y implicitly universal

// Invalid:
bad = (cons X Y) => Z    // ERROR: Z not bound by pattern

// Valid:
good = (cons X Y) => (plus X Y)    // X, Y bound by pattern
```

**Verdict:** Good balance! Clear rules, supports both functional and logic styles.

---

## My Recommendation: Option C (Schematic at Top-Level)

### The Rules

**Rule 1: Function Body Variables Must Be Bound**

In a function `pattern => body`:
- All free variables in `body` must appear in `pattern`
- Exception: top-level defined names (constants)

**Rule 2: Top-Level Free Variables Are Schematic**

At the top level (e.g., in theorem statements):
- Free variables are implicitly universally quantified
- They represent "for all" values

**Rule 3: Pattern Variables Scope to Body**

Pattern variables introduced in `pattern` are bound in `body`:
```logi
f = (cons X Y) => (plus X Y)    // X, Y scoped to body
```

### Examples

**Valid:**
```logi
// Pattern variables bound and used in body
sum = (cons X Y) => (plus X Y)    ✓

// Top-level schematic variables
theorem (eq (plus X Y) (plus Y X))    ✓

// Using defined constants
five = 5
f = X => (plus X five)    ✓ (five is defined)
```

**Invalid:**
```logi
// Free variable in body not bound by pattern
bad = X => Y    ✗ (Y not bound)

// Free variable in function definition body
bad = (cons X Y) => (plus X Z)    ✗ (Z not bound)
```

### Why This Works

1. **Clear scoping rules** - Easy to check statically
2. **Supports logic** - Top-level schematic variables for theorems
3. **Prevents errors** - Catches undefined variables in function bodies
4. **Natural for math** - Aligns with mathematical notation (free vars = universal)
5. **Doesn't require unification** - Just scoping/binding checks

### Implementation

**Static check:**
1. For each function `p => M`:
   - Collect all free variables in M
   - Check that all are bound by pattern p (or are top-level definitions)
   - If not: error

2. For top-level terms (like theorem arguments):
   - Free variables are allowed (implicitly universal)

---

## Examples Revisited

### Example 1: Valid Function

```logi
sum = (pair X Y) => (plus X Y)
```

**Check:**
- Pattern: `(pair X Y)` binds X, Y
- Body: `(plus X Y)` uses X, Y
- Both X, Y are bound ✓

### Example 2: Invalid Function

```logi
bad = (pair X Y) => (plus X Z)
```

**Check:**
- Pattern: `(pair X Y)` binds X, Y
- Body: `(plus X Z)` uses X, Y, Z
- Z is not bound ✗
- **ERROR: Z is free in function body**

### Example 3: Top-Level Schematic

```logi
theorem (eq (plus X Y) (plus Y X))
```

**Check:**
- This is a top-level term (argument to theorem)
- X, Y are free → implicitly universal
- Meaning: ∀X, Y: (plus X Y) = (plus Y X) ✓

### Example 4: The Directionality Case

```logi
f = a => b
result = f X    // Does this reduce?
```

**Evaluation:**
- Pattern: `a` (atom)
- Argument: `X` (free variable)
- To match: would need `a = X`, i.e., instantiate atom a with variable X
- This would modify the value (flow backwards) ✗
- **Result: does not reduce**

### Example 5: The Reverse Case

```logi
f = X => b
result = f a    // Does this reduce?
```

**Evaluation:**
- Pattern: `X` (pattern variable)
- Argument: `a` (atom)
- To match: bind `X = a` (destructuring, reading value into pattern)
- This is proper one-way flow ✓
- **Result: reduces to `b`**

---

## Summary

### Core Principles

1. **Semantic Stability:** Pattern matching only reads (destructures), never writes (modifies values)

2. **One-Way Information Flow:** Value → Pattern Variables (never backwards)

3. **Directionality Rule:**
   - Atom pattern vs variable argument: does NOT match (would need to modify pattern)
   - Variable pattern vs atom argument: DOES match (binds variable to value)

4. **Free Variable Rule:**
   - In function bodies: must be bound by pattern (error otherwise)
   - At top-level: implicitly universal (schematic variables)

### Answers to Your Questions

**Q: Should pattern matching be one-way (destructure only)?**  
**A: Yes! This preserves semantic stability.**

**Q: How should free metavariables work?**  
**A: Function bodies: forbidden (must be bound). Top-level: implicitly universal.**

### Implementation

**Add static check:**
```python
def check_function(pattern, body):
    bound_vars = collect_pattern_variables(pattern)
    free_vars = collect_free_variables(body)
    unbound = free_vars - bound_vars - defined_constants
    if unbound:
        raise Error(f"Unbound variables in function body: {unbound}")
```

---

**This gives us a clean, principled, enforceable system!**

End of Analysis.
