# The Logi Language: Complete Definition

## Single Source of Truth

**This document contains the complete, correct, authoritative definition of Logi.**

**Last updated:** Dec 31, 2025  
**Status:** Design complete, ready for implementation

---

## Syntax

### The 7 Constructs

```
M ::= atom           // lowercase identifier: true, false, imp
    | Var            // uppercase identifier: X, Y, P, Q
    | M M            // application (left-associative)
    | p => M         // function (right-associative)
    | M | M          // choice (binary infix, right-associative)
    | theorem M      // theorem marker
    | {M}            // quotation (prevent reduction)

p ::= atom           // atom pattern (match exactly)
    | Var            // variable pattern (bind anything)
    | _              // wildcard (match, don't bind)
    | (p p)          // application pattern
    | theorem p      // theorem pattern
    | {p}            // quoted pattern
```

### Critical Syntax Rules

1. **Arrow is `=>`** (not `->` or `→`)
2. **Pipe `|` is binary infix** (not prefix with braces)
3. **Curly braces `{M}` are for quotation** (not pattern matching)
4. **One pattern per arrow** (curry for multiple args: `X => Y => body`)
5. **Uppercase = variables, lowercase = atoms** (case convention)

---

## Semantics

### Values

```
V ::= atom | (p => M) | (M | M) | theorem V | {M}
```

### Pattern Matching

```
match(Var, V)               = [Var ↦ V]
match(_, V)                 = []
match(atom, atom)           = []
match(atom₁, atom₂)         = FAIL
match((p₁ p₂), (V₁ V₂))    = σ₁ ∪ σ₂  (if both succeed)
match(theorem p, theorem V) = match(p, V)
match({p}, {M})             = match(p, M)
match(_, _)                 = FAIL
```

### Reduction Rules

**Function application:**
```
(p => M) V  ⟶  M[σ]    if match(p, V) = σ
(p => M) V  ⟶  stuck   if match(p, V) = FAIL (partial function)
```

**Choice:**
```
(M₁ | M₂) V  ⟶  M₁ V    if M₁ V reduces
(M₁ | M₂) V  ⟶  M₂ V    if M₁ V doesn't reduce
```

**Evaluation strategy:** Call-by-value (eager evaluation of arguments)

**Quotation:** `{M}` is a value and never reduces. Reduction does not occur inside quotes.

### Substitution

Capture-avoiding substitution applies to all constructs:

```
{M}[σ] = {M[σ]}    // Substitution penetrates quotes
```

---

## Examples

### Boolean Logic

```logi
not = true => false | false => true

and = true => (X => X) | false => (_ => false)

or = true => (_ => true) | false => (X => X)
```

### Propositional Constructors

```logi
imp = P => Q => (imp P Q)
neg = P => (neg P)
```

### Inference Rules

```logi
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

hs = (theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R)
```

### Axioms and Derivation

```logi
ax1 = theorem (imp p q)
ax2 = theorem p

result = mp ax1 ax2    // → theorem q
```

### Lists

```logi
nil = nil
cons = X => Xs => (cons X Xs)

head = (cons X Xs) => X

length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

### Quotation

```logi
// Prevent reduction
term = {not true}           // → {not true} (doesn't reduce to {false})

// Extract via pattern matching
extract = {X} => X

result = extract term       // → not true → false

// Substitution works inside quotes
build = X => {not X}
term2 = build true          // → {not true}
```

### Quantifiers (HOAS)

```logi
forall = P => (forall P)

ui = (theorem (forall P)) => T => theorem (P T)

// Usage
ax = theorem (forall (X => (prop X)))
instance = ui ax a          // → theorem (prop a)
```

---

## Precedence

**From lowest to highest:**

1. `|` (choice) - right-associative
2. `=>` (function) - right-associative
3. Application - left-associative

**Examples:**
```logi
a => b | c => d         = (a => b) | (c => d)
X => Y => Z             = X => (Y => Z)
f x y                   = (f x) y
```

---

## Static Semantics

### Axiom Detection

Definitions with `theorem` at top level are axioms:

```logi
ax1 = theorem p          // ✅ Axiom (static analysis detects)
derived = mp ax1 ax2     // ❌ Not an axiom (computed)
```

### Soundness

Can't produce `theorem P` without:
1. Axiomatizing it (using `theorem` keyword), OR
2. Deriving it from existing theorems via computation

---

## Standard Library

### Boolean Logic

```logi
true = true
false = false
not = true => false | false => true
and = true => (X => X) | false => (_ => false)
or = true => (_ => true) | false => (X => X)
```

### Propositional Constructors

```logi
imp = P => Q => (imp P Q)
neg = P => (neg P)
conj = P => Q => (conj P Q)
disj = P => Q => (disj P Q)
iff = P => Q => (iff P Q)
```

### Inference Rules

```logi
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)
hs = (theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R)
conj_intro = (theorem P) => (theorem Q) => theorem (conj P Q)
conj_elim_l = (theorem (conj P Q)) => theorem P
conj_elim_r = (theorem (conj P Q)) => theorem Q
contrapositive = (theorem (imp P Q)) => theorem (imp (neg Q) (neg P))
```

### Lists

```logi
nil = nil
cons = X => Xs => (cons X Xs)
head = (cons X Xs) => X
tail = (cons X Xs) => Xs
length = nil => zero | (cons _ Xs) => (succ (length Xs))
append = nil => (Ys => Ys) | (cons X Xs) => (Ys => (cons X (append Xs Ys)))
map = F => (nil => nil | (cons X Xs) => (cons (F X) (map F Xs)))
filter = P => (nil => nil | (cons X Xs) => ((P X) => (cons X (filter P Xs)) | _ => (filter P Xs)))
```

### Natural Numbers

```logi
zero = zero
succ = N => (succ N)
one = (succ zero)
two = (succ one)
three = (succ two)
add = zero => (N => N) | (succ M) => (N => (succ (add M N)))
mul = zero => (_ => zero) | (succ M) => (N => (add N (mul M N)))
```

### Quantifiers (HOAS)

```logi
forall = P => (forall P)
exists = P => (exists P)
ui = (theorem (forall P)) => T => theorem (P T)
ei = (theorem Prop) => P => theorem (exists P)
```

---

## Grammar (BNF)

```
<program>    ::= <definition>*
<definition> ::= <identifier> "=" <term>

<term>       ::= <atom>
               | <variable>
               | "theorem" <term>
               | <term> <term>
               | <pattern> "=>" <term>
               | <term> "|" <term>
               | "{" <term> "}"
               | "(" <term> ")"

<pattern>    ::= <atom>
               | <variable>
               | "_"
               | "theorem" <pattern>
               | "{" <pattern> "}"
               | "(" <pattern> <pattern> ")"

<atom>       ::= [a-z][a-zA-Z0-9_]*
<variable>   ::= [A-Z][a-zA-Z0-9_]*
<identifier> ::= <atom> | <variable>
```

---

## Metatheory

### Determinism

Reduction is deterministic (at most one reduction path).

### Confluence

If M ⟶* N₁ and M ⟶* N₂, then ∃P: N₁ ⟶* P and N₂ ⟶* P.

### Static Soundness

Axioms are detected statically via `theorem` keyword at top level.

### Partial Functions

No pattern match = no reduction (not an error).

---

## Implementation Notes

### Estimated Complexity

- Lexer: 150 lines
- Parser: 300 lines
- Pattern matcher: 100 lines
- Evaluator: 200 lines
- Substitution: 100 lines
- REPL: 100 lines
- **Total: ~1000 lines**

### Estimated Time

2-3 weeks for experienced developer

### Recommended Languages

- Python (fast prototyping)
- OCaml (elegant implementation)
- Haskell (pure functional)
- Rust (performance + safety)

---

## Use Cases

### Primary: Logic Verification

```logi
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
result = mp axiom1 axiom2
```

### Secondary: General Computation

Lists, trees, arithmetic, algorithms, data processing

### Advanced: Meta-Programming

Symbolic computation, interpreters, compilers, DSLs, code generation

**See [USE_CASES.md](USE_CASES.md) for 13 categories of use cases.**

---

## Example: Complete Proof

```logi
// Constructors
imp = P => Q => (imp P Q)
neg = P => (neg P)

// Atoms
p = p
q = q

// Axioms
ax1 = theorem (imp p q)      // P → Q
ax2 = theorem (neg q)         // ¬Q

// Modus Tollens
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

// Derive ¬P
result = mt ax1 ax2
// Reduces to: theorem (neg p)  ✓
```

---

## Documentation

**For learning:**
- [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md) - Quick syntax card
- [QUICKSTART.md](QUICKSTART.md) - Introduction
- [EXAMPLES.md](EXAMPLES.md) - Worked examples

**For implementing:**
- [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md) - Complete semantics
- [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md) - Code skeleton

**For navigation:**
- [START_HERE.md](START_HERE.md) - Main entry
- [OVERVIEW.md](OVERVIEW.md) - Visual map
- [FILES_SUMMARY.md](FILES_SUMMARY.md) - Complete file status

---

## Summary

**Logi is:**
- Minimal (7 constructs)
- Expressive (first-order logic + general computation)
- Unified (computation and proof together)
- Simple (pattern matching, no dependent types)
- Powerful (quotation for meta-programming)

**Ready for implementation!**

---

End of Language Definition.
