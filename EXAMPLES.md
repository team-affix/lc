# Logi: Proof Examples and Demonstrations

## Purpose

This document demonstrates the Logi language with concrete examples, showing how to:
1. Define logical connectives and operations
2. State axioms and inference rules
3. Derive theorems step-by-step
4. Work with quantifiers and higher-order logic
5. Build reusable proof tactics

---

## 1. Boolean Logic Foundations

### 1.1 Basic Definitions

```
// Boolean values (atoms)
true = true
false = false

// Negation
not = true => false | false => true

// Conjunction
and = true => (X => X) | false => (_ => false)

// Disjunction
or = true => (_ => true) | false => (X => X)

// Implication (constructor for propositions)
imp = P => Q => (imp P Q)

// Negation (constructor for propositions)
neg = P => (neg P)
```

### 1.2 Boolean Computations

**NOT:**
```
not true
= (true => false | false => true) true
= (true => false) true
= false  ✓

not false
= (true => false | false => true) false
= (false => true) false     // First case doesn't match
= true  ✓
```

**AND:**
```
and true true
= (true => (X => X) | false => (_ => false)) true true
= (X => X) true
= true  ✓

and true false
= (X => X) false
= false  ✓

and false true
= (true => (X => X) | false => (_ => false)) false true
= (_ => false) true         // First case doesn't match
= false  ✓
```

**OR:**
```
or true false
= (true => (_ => true) | false => (X => X)) true false
= (_ => true) false
= true  ✓

or false false
= (false => (X => X)) false
= (X => X) false
= false  ✓
```

---

## 2. Propositional Logic - Inference Rules

### 2.1 Rule Definitions

```
// Modus Ponens: (P → Q), P ⊢ Q
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Modus Tollens: (P → Q), ¬Q ⊢ ¬P
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

// Hypothetical Syllogism: (P → Q), (Q → R) ⊢ (P → R)
hs = (theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R)

// Conjunction Introduction: P, Q ⊢ (P ∧ Q)
conj_intro = (theorem P) => (theorem Q) => theorem (and P Q)

// Conjunction Elimination Left: (P ∧ Q) ⊢ P
conj_elim_l = (theorem (and P Q)) => theorem P

// Conjunction Elimination Right: (P ∧ Q) ⊢ Q
conj_elim_r = (theorem (and P Q)) => theorem Q

// Contrapositive: (P → Q) ⊢ (¬Q → ¬P)
contrapositive = (theorem (imp P Q)) => theorem (imp (neg Q) (neg P))
```

---

## 3. Example Proof 1: Simple Modus Ponens

### 3.1 Given

**Axioms:**
```
ax1 = theorem (imp p q)    // P → Q
ax2 = theorem p             // P
```

**Goal:** Derive Q

### 3.2 Proof

```
result = mp ax1 ax2
```

### 3.3 Detailed Reduction

```
mp ax1 ax2
= mp (theorem (imp p q)) (theorem p)

= ((theorem (imp P Q)) => (theorem P) => theorem Q) 
  (theorem (imp p q)) 
  (theorem p)

// Apply first argument - pattern matches (imp p q) with (imp P Q)
[Bindings: P := p, Q := q]

= ((theorem P) => theorem Q)[P := p, Q := q] (theorem p)

= ((theorem p) => theorem q) (theorem p)

// Apply second argument - pattern matches p with p
[Bindings: none needed, already bound]

= theorem q

✓ Derived: theorem q
```

---

## 4. Example Proof 2: Hypothetical Syllogism

### 4.1 Given

**Axioms:**
```
ax1 = theorem (imp p q)    // P → Q
ax2 = theorem (imp q r)    // Q → R
```

**Goal:** Derive P → R

### 4.2 Proof

```
result = hs ax1 ax2
```

### 4.3 Detailed Reduction

```
hs (theorem (imp p q)) (theorem (imp q r))

= ((theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R))
  (theorem (imp p q))
  (theorem (imp q r))

// First application
[Match (imp p q) with (imp P Q): P := p, Q := q]

= ((theorem (imp Q R)) => theorem (imp P R))[P := p, Q := q]
  (theorem (imp q r))

= ((theorem (imp q R)) => theorem (imp p R))
  (theorem (imp q r))

// Second application
[Match (imp q r) with (imp q R): R := r]

= theorem (imp p R)[R := r]

= theorem (imp p r)

✓ Derived: theorem (imp p r)  // P → R
```

---

## 5. Example Proof 3: Chain of Implications

### 5.1 Given

**Axioms:**
```
ax1 = theorem (imp p q)    // P → Q
ax2 = theorem (imp q r)    // Q → R
ax3 = theorem p             // P
```

**Goal:** Derive R

### 5.2 Proof

```
// Step 1: Derive P → R
step1 = hs ax1 ax2         // theorem (imp p r)

// Step 2: Apply modus ponens
result = mp step1 ax3      // theorem r
```

### 5.3 Detailed Reduction

**Step 1:** (already shown above)
```
hs ax1 ax2  →*  theorem (imp p r)
```

**Step 2:**
```
mp (theorem (imp p r)) (theorem p)

= ((theorem (imp P Q)) => (theorem P) => theorem Q)
  (theorem (imp p r))
  (theorem p)

[Match: P := p, Q := r]

= ((theorem p) => theorem r) (theorem p)

= theorem r

✓ Derived: theorem r
```

---

## 6. Example Proof 4: Modus Tollens Application

### 6.1 Given

**Axioms:**
```
ax1 = theorem (imp p q)       // P → Q
ax2 = theorem (neg q)          // ¬Q
```

**Goal:** Derive ¬P

### 6.2 Proof

```
result = mt ax1 ax2
```

### 6.3 Detailed Reduction

```
mt (theorem (imp p q)) (theorem (neg q))

= ((theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P))
  (theorem (imp p q))
  (theorem (neg q))

[Match: P := p, Q := q]

= ((theorem (neg Q)) => theorem (neg P))[P := p, Q := q]
  (theorem (neg q))

= ((theorem (neg q)) => theorem (neg p))
  (theorem (neg q))

[Match: Q matches q]

= theorem (neg p)

✓ Derived: theorem (neg p)  // ¬P
```

---

## 7. Example Proof 5: Conjunction Introduction and Elimination

### 7.1 Given

**Axioms:**
```
ax1 = theorem p
ax2 = theorem q
ax3 = theorem r
```

**Goal:** Derive (P ∧ Q) ∧ R, then extract P

### 7.2 Proof

```
// Build (P ∧ Q)
step1 = conj_intro ax1 ax2        // theorem (and p q)

// Build ((P ∧ Q) ∧ R)
step2 = conj_intro step1 ax3      // theorem (and (and p q) r)

// Extract P from (P ∧ Q)
step3 = conj_elim_l step1         // theorem p

// Verify we get back our original axiom
// step3 = ax1  ✓
```

### 7.3 Detailed Reduction

**Step 1:**
```
conj_intro (theorem p) (theorem q)

= ((theorem P) => (theorem Q) => theorem (and P Q))
  (theorem p)
  (theorem q)

= ((theorem Q) => theorem (and p Q)) (theorem q)

= theorem (and p q)
```

**Step 2:**
```
conj_intro (theorem (and p q)) (theorem r)

= ((theorem P) => (theorem Q) => theorem (and P Q))
  (theorem (and p q))
  (theorem r)

= theorem (and (and p q) r)
```

**Step 3:**
```
conj_elim_l (theorem (and p q))

= ((theorem (and P Q)) => theorem P) (theorem (and p q))

[Match: (and p q) with (and P Q), P := p, Q := q]

= theorem P[P := p]

= theorem p
```

---

## 8. Quantifiers with Higher-Order Abstract Syntax (HOAS)

### 8.1 Quantifier Definitions

```
// Universal quantifier: ∀P where P is a predicate (function from terms to props)
forall = P => (forall P)

// Existential quantifier: ∃P
exists = P => (exists P)

// Universal Instantiation: (∀P) ⊢ P(t)
ui = (theorem (forall P)) => T => theorem (P T)

// Existential Introduction: P(t) ⊢ ∃P
ei = (theorem Prop) => P => theorem (exists P)
```

### 8.2 Example: Universal Instantiation

**Given:**
```
// Predicate: IsEven(x)
iseven = N => (iseven N)

// Axiom: ∀x. IsEven(x) → IsEven(succ(succ(x)))
ax = theorem (forall (X => imp (iseven X) (iseven (succ (succ X)))))

// Constant: 2
two = (succ (succ zero))
```

**Goal:** Derive IsEven(2) → IsEven(4)

**Proof:**
```
result = ui ax two
```

**Reduction:**
```
ui (theorem (forall (X => imp (iseven X) (iseven (succ (succ X)))))) 
   (succ (succ zero))

= ((theorem (forall P)) => T => theorem (P T))
  (theorem (forall (X => imp (iseven X) (iseven (succ (succ X))))))
  (succ (succ zero))

[Match: P := (X => imp (iseven X) (iseven (succ (succ X))))]

= (T => theorem (P T))[P := ...] (succ (succ zero))

= (T => theorem ((X => imp (iseven X) (iseven (succ (succ X)))) T))
  (succ (succ zero))

[Apply: T := (succ (succ zero))]

= theorem ((X => imp (iseven X) (iseven (succ (succ X)))) (succ (succ zero)))

// Beta-reduce the lambda application inside theorem
= theorem (imp (iseven (succ (succ zero))) 
               (iseven (succ (succ (succ (succ zero))))))

// Simplify
= theorem (imp (iseven two) (iseven four))

✓ Derived: IsEven(2) → IsEven(4)
```

### 8.3 Example: Existential Introduction

**Given:**
```
// Axiom: IsPositive(5)
ax = theorem (ispositive five)

// Predicate
ispositive = N => (ispositive N)
```

**Goal:** Derive ∃x. IsPositive(x)

**Proof:**
```
result = ei ax (X => (ispositive X))
```

**Reduction:**
```
ei (theorem (ispositive five)) (X => (ispositive X))

= ((theorem Prop) => P => theorem (exists P))
  (theorem (ispositive five))
  (X => (ispositive X))

[Match: Prop := (ispositive five), P := (X => (ispositive X))]

= theorem (exists (X => (ispositive X)))

✓ Derived: ∃x. IsPositive(x)
```

---

## 9. List Operations

### 9.1 Definitions

```
// Constructors
nil = nil
cons = X => Xs => (cons X Xs)

// Head (partial function)
head = (cons X Xs) => X

// Tail (partial function)
tail = (cons X Xs) => Xs

// Length
length = nil => zero | (cons _ Xs) => (succ (length Xs))

// Append
append = nil => (Ys => Ys) | (cons X Xs) => (Ys => (cons X (append Xs Ys)))
```

### 9.2 Example: Computing List Length

```
mylist = (cons one (cons two (cons three nil)))

length mylist
```

**Reduction:**
```
length (cons one (cons two (cons three nil)))

= (nil => zero | (cons _ Xs) => (succ (length Xs)))
  (cons one (cons two (cons three nil)))

// First case (nil) doesn't match, try second case
= (cons _ Xs => (succ (length Xs)))
  (cons one (cons two (cons three nil)))

[Match: _ matches one, Xs := (cons two (cons three nil))]

= (succ (length Xs))[Xs := (cons two (cons three nil))]

= (succ (length (cons two (cons three nil))))

// Recurse
= (succ (succ (length (cons three nil))))

= (succ (succ (succ (length nil))))

// Base case
= (succ (succ (succ zero)))

✓ Result: three
```

### 9.3 Example: List Append

```
list1 = (cons a (cons b nil))
list2 = (cons c nil)

append list1 list2
```

**Reduction:**
```
append (cons a (cons b nil)) (cons c nil)

= (nil => (Ys => Ys) | (cons X Xs) => (Ys => (cons X (append Xs Ys))))
  (cons a (cons b nil))
  (cons c nil)

// Try second case
= (cons X Xs => (Ys => (cons X (append Xs Ys))))
  (cons a (cons b nil))
  (cons c nil)

[Match: X := a, Xs := (cons b nil)]

= (Ys => (cons X (append Xs Ys)))[X := a, Xs := (cons b nil)]
  (cons c nil)

= (Ys => (cons a (append (cons b nil) Ys)))
  (cons c nil)

= (cons a (append (cons b nil) (cons c nil)))

// Recurse
= (cons a (cons b (append nil (cons c nil))))

// Base case
= (cons a (cons b (cons c nil)))

✓ Result: [a, b, c]
```

---

## 10. Partial Functions in Action

### 10.1 Head of Empty List

```
head nil
```

**Reduction:**
```
head nil
= ((cons X Xs) => X) nil

// Pattern (cons X Xs) doesn't match nil
// No alternative case provided
// No reduction occurs

Result: (head nil)  // Stays as-is, not an error!
```

**This is the natural behavior of a partial function.**

### 10.2 NOT on Non-Boolean

```
not (cons a b)
```

**Reduction:**
```
not (cons a b)
= (true => false | false => true) (cons a b)

// Try first case: (cons a b) doesn't match true
// Try second case: (cons a b) doesn't match false
// No match

Result: (not (cons a b))  // Stays as-is
```

**The function `not` is simply undefined for non-booleans.**

---

## 11. User-Defined Derived Rules

### 11.1 Three-Step Implication Chain

```
// Chain three implications: (P→Q), (Q→R), (R→S) ⊢ (P→S)
chain3 = (theorem (imp P Q)) => 
         (theorem (imp Q R)) => 
         (theorem (imp R S)) => 
         hs (hs (theorem (imp P Q)) (theorem (imp Q R))) (theorem (imp R S))
```

**Usage:**
```
ax1 = theorem (imp p q)
ax2 = theorem (imp q r)
ax3 = theorem (imp r s)

result = chain3 ax1 ax2 ax3
// → theorem (imp p s)  ✓
```

### 11.2 Biconditional Rules

```
// Biconditional constructor
iff = P => Q => (iff P Q)

// Biconditional Introduction: (P → Q), (Q → P) ⊢ (P ↔ Q)
iff_intro = (theorem (imp P Q)) => (theorem (imp Q P)) => theorem (iff P Q)

// Biconditional Elimination Forward: (P ↔ Q) ⊢ (P → Q)
iff_elim_fwd = (theorem (iff P Q)) => theorem (imp P Q)

// Biconditional Elimination Backward: (P ↔ Q) ⊢ (Q → P)
iff_elim_bwd = (theorem (iff P Q)) => theorem (imp Q P)
```

---

## 12. Natural Number Arithmetic

### 12.1 Definitions

```
// Zero
zero = zero

// Successor
succ = N => (succ N)

// Numerals
one = (succ zero)
two = (succ one)
three = (succ two)

// Addition
add = zero => (N => N) 
    | (succ M) => (N => (succ (add M N)))

// Multiplication
mul = zero => (_ => zero) 
    | (succ M) => (N => (add N (mul M N)))
```

### 12.2 Example: Addition

```
add two three
```

**Reduction:**
```
add (succ (succ zero)) (succ (succ (succ zero)))

= (zero => (N => N) | (succ M) => (N => (succ (add M N))))
  (succ (succ zero))
  (succ (succ (succ zero)))

// Second case matches
= (succ M => (N => (succ (add M N))))
  (succ (succ zero))
  (succ (succ (succ zero)))

[Match: M := (succ zero)]

= (N => (succ (add M N)))[M := (succ zero)]
  (succ (succ (succ zero)))

= (N => (succ (add (succ zero) N)))
  (succ (succ (succ zero)))

= (succ (add (succ zero) (succ (succ (succ zero)))))

// Recurse
= (succ (succ (add zero (succ (succ (succ zero))))))

// Base case
= (succ (succ (succ (succ (succ zero)))))

✓ Result: five (2 + 3 = 5)
```

---

## 13. Proof Tactics and Automation

### 13.1 Simple Auto-Prover

```
// Try multiple inference rules
auto_propositional = Goal => Theorems => 
  try_mp Goal Theorems 
  | try_hs Goal Theorems
  | try_mt Goal Theorems
  | fail

// Helper: Try to apply modus ponens
try_mp = Goal => Theorems =>
  // Search through theorems for applicable implication
  // This would require list operations and equality checking
  // Simplified pseudocode
  ...
```

### 13.2 Simplification Tactic

```
// Repeatedly simplify double negations
simplify = (theorem (neg (neg P))) => simplify (theorem P)
         | (theorem P) => theorem P
```

**Usage:**
```
complex = theorem (neg (neg (neg (neg p))))

simple = simplify complex
// → theorem p  ✓
```

---

## 14. Complete Example: Contrapositive Proof

### 14.1 Goal

Prove: If we have (P → Q) and ¬Q, we can derive ¬P using contrapositive.

### 14.2 Setup

```
// Axioms
ax1 = theorem (imp p q)      // P → Q
ax2 = theorem (neg q)         // ¬Q

// Inference rules (already defined)
// contrapositive: (P → Q) ⊢ (¬Q → ¬P)
// mp: (P → Q), P ⊢ Q
```

### 14.3 Proof Method 1: Direct Modus Tollens

```
result = mt ax1 ax2
// → theorem (neg p)  ✓
```

### 14.4 Proof Method 2: Via Contrapositive + MP

```
// Step 1: Derive ¬Q → ¬P
step1 = contrapositive ax1

// Step 2: Apply modus ponens with ¬Q
result = mp step1 ax2
```

**Reduction of Step 1:**
```
contrapositive (theorem (imp p q))

= ((theorem (imp P Q)) => theorem (imp (neg Q) (neg P)))
  (theorem (imp p q))

[Match: P := p, Q := q]

= theorem (imp (neg q) (neg p))

step1 = theorem (imp (neg q) (neg p))
```

**Reduction of Step 2:**
```
mp (theorem (imp (neg q) (neg p))) (theorem (neg q))

= ((theorem (imp P Q)) => (theorem P) => theorem Q)
  (theorem (imp (neg q) (neg p)))
  (theorem (neg q))

[Match: P := (neg q), Q := (neg p)]

= ((theorem (neg q)) => theorem (neg p))
  (theorem (neg q))

= theorem (neg p)

✓ Result: theorem (neg p)
```

**Both methods derive the same result!**

---

## 15. Summary

These examples demonstrate:

✅ **Boolean logic** - Simple pattern matching on atoms  
✅ **Propositional inference** - Rules as curried functions  
✅ **Quantifiers with HOAS** - Predicates as functions  
✅ **List operations** - Recursive pattern matching  
✅ **Partial functions** - Natural handling of undefined cases  
✅ **Derived rules** - Building complex tactics from primitives  
✅ **Natural numbers** - Peano-style arithmetic  
✅ **Multiple proof strategies** - Same goal, different paths  

**Key observations:**

1. **Currying is natural** - Multi-argument functions compose well
2. **One pattern per arrow** - No ambiguity, clear syntax
3. **Partial functions work** - No errors, just non-reducing terms
4. **HOAS is elegant** - Quantifiers use native function abstraction
5. **Rules are just functions** - No special proof language needed

The Logi language successfully unifies computation and proof in a minimal, elegant framework!

---

End of Examples.
