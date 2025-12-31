# Logi: Quick Start Guide

## What is Logi?

**Logi** is a minimal logic verification language where:
- Functions are defined by pattern matching
- Partial functions arise naturally (no match = no error)
- Computation and proof are unified
- Uppercase = variables, lowercase = atoms
- One pattern per arrow (curry for multiple arguments)

---

## The Language in 60 Seconds

### Core Syntax

```
// Define a function with pattern matching
not = true => false | false => true

// Curry for multiple arguments
and = true => (X => X) | false => (_ => false)

// Mark propositions with 'theorem'
axiom1 = theorem (imp p q)

// Inference rules are just functions
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Quotation prevents reduction (use curly braces)
unreduced = {not true}    // stays as {not true}, doesn't reduce to {false}
```

### Key Features

1. **One pattern per `=>`** - Must curry: `X => Y => body`
2. **Binary `|` operator** - Join alternatives: `case1 | case2`
3. **No `end` token** - Pipe connects cases directly
4. **Partial functions** - No match just means undefined (not an error!)
5. **Case convention** - `Uppercase` = variables, `lowercase` = atoms
6. **Quotation with `{M}`** - Prevent reduction while allowing substitution

---

## Example 1: Boolean Logic

```
// Values
true = true
false = false

// Negation
not = true => false | false => true

// Usage
not true   // → false
not false  // → true
not xyz    // → (not xyz)  // Partial! No match, doesn't reduce
```

**Key insight:** `not` is only defined for `true` and `false`. For anything else, it just doesn't reduce.

---

## Example 2: Multi-Argument Functions

```
// Boolean AND - curried style
and = true => (X => X) | false => (_ => false)

// Usage
and true false
= (true => (X => X) | false => (_ => false)) true false
= (X => X) false
= false  ✓
```

**Why curry?**
- Each `=>` takes ONE pattern
- `X Y => body` looks like application (confusing!)
- `X => Y => body` is clear (curried)

---

## Example 3: Axioms and Theorems

```
// Propositional variables (atoms)
p = p
q = q

// Axioms (marked with 'theorem')
ax1 = theorem (imp p q)    // P → Q
ax2 = theorem p             // P

// Implication constructor
imp = P => Q => (imp P Q)
```

---

## Example 4: Inference Rules

```
// Modus Ponens: (P → Q), P ⊢ Q
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Apply it
result = mp ax1 ax2
// → theorem q  ✓
```

**Breakdown:**
1. First pattern: `(theorem (imp P Q))` matches `ax1`, binds `P := p, Q := q`
2. Returns function: `(theorem P) => theorem Q` with `P` and `Q` bound
3. Second pattern: `(theorem P)` matches `ax2`
4. Returns: `theorem Q` = `theorem q`

---

## Example 5: Pattern Matching on Structures

```
// List constructors
nil = nil
cons = X => Xs => (cons X Xs)

// Head (partial function)
head = (cons X Xs) => X

// Usage
head (cons 1 (cons 2 nil))   // → 1
head nil                      // → (head nil)  // Partial! Doesn't reduce
```

**Structured pattern:** `(cons X Xs)` matches a cons cell and binds both parts.

---

## Example 6: Recursive Functions

```
// List length
length = nil => zero | (cons _ Xs) => (succ (length Xs))

// Usage
length (cons 1 (cons 2 nil))
// → (succ (succ zero))  ✓
```

**Pattern alternatives:**
- First case: `nil => zero` (base case)
- Second case: `(cons _ Xs) => ...` (recursive case)
- Connected by `|` (pipe operator)

---

## Example 7: Quotation (Prevent Reduction)

```
// Without braces - reduces
term1 = not true
// → false

// With braces - doesn't reduce
term2 = {not true}
// → {not true}  ✓

// Extract from quote
extract = {X} => X

result = extract {not true}
// → not true  (now can reduce)
// → false
```

**Key insight:** Curly braces `{M}` freeze reduction inside, but substitution still works!

```
build_neg = X => {not X}

term = build_neg true
// → {not true}  ✓  (substituted but not reduced)
```

---

## Key Concepts

### 1. Case Convention

**Lowercase** = atoms (match exactly)
```
true, false, nil, zero, imp, theorem
```

**Uppercase** = variables (bind anything in patterns, refer in terms)
```
X, Y, P, Q, Head, Tail
```

**Wildcard** = `_` (matches anything, doesn't bind)
```
_ => false   // Matches anything but doesn't name it
```

### 2. One Pattern Per Arrow

**Wrong:**
```
add = X Y => plus X Y    // Looks like application!
```

**Right:**
```
add = X => Y => plus X Y    // Clearly curried
```

**Exception:** Structured patterns are ONE pattern:
```
head = (cons X Xs) => X    // One pattern: (cons X Xs)
```

### 3. Pipe as Binary Operator

**Not a prefix:**
```
not = | true => false    // NO!
    | false => true
```

**Binary infix:**
```
not = true => false | false => true    // YES!
```

**Right-associative:**
```
a => b | c => d | e => f
= (a => b) | ((c => d) | (e => f))
```

### 4. Partial Functions are Natural

When no pattern matches, the term just doesn't reduce:

```
not xyz       // stays as (not xyz)
head nil      // stays as (head nil)
divide 10 0   // stays as (divide 10 0)
```

**This is not an error!** It's simply an undefined value.

---

## Precedence (Low to High)

```
|     (choice - lowest precedence)
=>    (function)
app   (application - highest precedence)
```

**Examples:**
```
f X Y              = (f X) Y           // Left-associative application
X => Y => Z        = X => (Y => Z)     // Right-associative function
a => b | c => d    = (a => b) | (c => d)
```

---

## Complete Example: Propositional Logic Proof

```
// Implication constructor
imp = P => Q => (imp P Q)

// Negation constructor
neg = P => (neg P)

// Axioms
ax1 = theorem (imp p q)      // P → Q
ax2 = theorem (neg q)         // ¬Q

// Modus Tollens: (P → Q), ¬Q ⊢ ¬P
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

// Derive ¬P
result = mt ax1 ax2
// → theorem (neg p)  ✓
```

**Trace:**
```
mt (theorem (imp p q)) (theorem (neg q))

// First application matches (imp p q) with (imp P Q)
= ((theorem (neg Q)) => theorem (neg P))[P := p, Q := q] (theorem (neg q))

= ((theorem (neg q)) => theorem (neg p)) (theorem (neg q))

// Second application matches (neg q) with (neg q)
= theorem (neg p)  ✓
```

---

## Quantifiers (Advanced)

Logi uses **Higher-Order Abstract Syntax (HOAS)** for quantifiers:

```
// Universal quantifier
forall = P => (forall P)

// Universal instantiation
ui = (theorem (forall P)) => T => theorem (P T)
```

**Example:**
```
// Axiom: ∀x. P(x)
ax = theorem (forall (X => (prop X)))

// Instantiate with 'a'
instance = ui ax a
// → theorem (prop a)  ✓
```

**Key:** The predicate `P` is a function from terms to propositions.

---

## Common Patterns

### Total Function (No Pipe)

```
id = X => X
compose = F => G => X => F (G X)
```

### Partial Function (With Alternatives)

```
not = true => false | false => true
head = (cons X Xs) => X
```

### Curried Multi-Argument

```
add = X => Y => plus X Y
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
```

### Structured Pattern Matching

```
length = nil => zero | (cons _ Xs) => (succ (length Xs))
get_fst = (pair X Y) => X
```

---

## Why This Design?

1. **Minimal** - 6 constructs total
2. **Unambiguous** - One pattern per arrow eliminates confusion
3. **Natural** - Partial functions just don't reduce (elegant!)
4. **Elegant** - No end token needed, pipe connects alternatives
5. **Practical** - Currying enables partial application

---

## Next Steps

**To learn more:**
- **FORMAL_SPECIFICATION.md** - Complete formal semantics
- **EXAMPLES.md** - Detailed worked examples
- **RECOMMENDATIONS.md** - Implementation guide

**To start using:**
1. Define your atoms (lowercase)
2. Use variables (uppercase) in patterns
3. Curry for multiple arguments
4. Connect alternatives with `|`
5. Mark theorems with `theorem`

---

## Quick Reference Card

```
// Atoms
true, false, nil

// Variables in patterns bind
X => body

// Variables in terms refer
X

// Wildcard matches anything
_ => body

// Application (left-assoc)
f x y = ((f x) y)

// Function (right-assoc)
X => Y => body = X => (Y => body)

// Choice (right-assoc, binary infix)
a => b | c => d

// Theorem marker
theorem M

// Quotation (prevent reduction)
{M}

// Structured patterns (ONE pattern!)
(cons X Xs) => body
(theorem (imp P Q)) => body
{not X} => body
```

---

**Welcome to Logi - where computation and proof are unified!** 🎉
