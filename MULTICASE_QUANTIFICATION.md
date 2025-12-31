# Quantification Over Multi-Case Partial Functions

## The Question

**What does this mean?**

```logi
exists (a => b | c => d)
```

This is a partial function with **TWO cases**:
- If input is `a`, return `b`
- If input is `c`, return `d`
- Otherwise, don't reduce

**Domain:** `{a, c}`

---

## Parsing

Given precedence rules (`|` is lowest, right-associative):

```logi
exists (a => b | c => d)
```

Parses as:

```logi
exists ((a => b) | (c => d))
```

This is a **single** partial function with two pattern-match cases.

---

## Semantic Analysis

### What is `exists P` asking?

"There exists at least one value X such that `P X` evaluates to true"

### Applied to our function

```logi
f = (a => b) | (c => d)
```

**Evaluation behavior:**
- `f a → b`
- `f c → d`  
- `f x → (f x)` (doesn't reduce for any other x)

### So `exists f` asks:

"Is there at least one value X such that `f X` evaluates to true?"

**Answer:** "Yes, if **either** `b` is true **or** `d` is true (or both)"

---

## The Stunning Insight

### `exists` over multi-case partial functions encodes **DISJUNCTION**!

```logi
exists ((a => b) | (c => d))
```

**Means:** "`b` is true OR `d` is true"

**Equivalently:** `b ∨ d`

### Why?

The function has domain `{a, c}`. We're asking: "Is there at least one input that produces a true result?"

- If input is `a`: result is `b`
- If input is `c`: result is `d`

At least one true result exists iff: `b ∨ d`

---

## More Examples

### Example 1: Propositions

```logi
exists ((a => (iseven a)) | (c => (iseven c)))
```

**Meaning:** "There exists a value (either `a` or `c`) that is even"

**Equivalently:** "`a` is even OR `c` is even"

**Logical form:** `(iseven a) ∨ (iseven c)`

### Example 2: Three Cases

```logi
exists ((two => (iseven two)) | (four => (iseven four)) | (six => (iseven six)))
```

**Meaning:** "At least one of {2, 4, 6} is even"

**Equivalently:** `(iseven two) ∨ (iseven four) ∨ (iseven six)`

### Example 3: Witnesses for Equation

```logi
exists ((two => (eq (square two) four)) | (neg_two => (eq (square neg_two) four)))
```

**Meaning:** "At least one of {2, -2} is a solution to x² = 4"

**Equivalently:** `(eq (square two) four) ∨ (eq (square neg_two) four)`

---

## Universal Quantification

### What about `forall`?

```logi
forall ((a => b) | (c => d))
```

**Meaning:** "For ALL values in the domain, the function returns something true"

Domain = `{a, c}`

**So:** "For all X in {a, c}, the result is true"

**Which means:** "`b` is true AND `d` is true"

### `forall` over multi-case partial functions encodes **CONJUNCTION**!

```logi
forall ((a => b) | (c => d))
```

**Means:** "`b` is true AND `d` is true"

**Equivalently:** `b ∧ d`

---

## The Pattern

### Summary Table

| Quantifier | Multi-case Partial Function | Logical Operator |
|------------|----------------------------|------------------|
| `exists`   | `(a => P) \| (c => Q)`     | `P ∨ Q` (OR)     |
| `forall`   | `(a => P) \| (c => Q)`     | `P ∧ Q` (AND)    |

### Single-case vs Multi-case

**Single case:**
```logi
exists (a => P)   ≡  P    // Just a witness
forall (a => P)   ≡  P    // Just a specific instance
```

**Multiple cases:**
```logi
exists ((a => P) | (c => Q))   ≡  P ∨ Q    // Disjunction
forall ((a => P) | (c => Q))   ≡  P ∧ Q    // Conjunction
```

---

## Concrete Examples

### Example 1: Even numbers

```logi
// "At least one of {2, 4} is even"
claim1 = theorem (exists ((two => (iseven two)) | (four => (iseven four))))

// Equivalent to:
claim1 = theorem ((iseven two) or (iseven four))
```

### Example 2: All are even

```logi
// "Both 2 and 4 are even"
claim2 = theorem (forall ((two => (iseven two)) | (four => (iseven four))))

// Equivalent to:
claim2 = theorem ((iseven two) and (iseven four))
```

### Example 3: Solutions to equation

```logi
// "At least one of {2, -2} solves x² = 4"
exists_solution = theorem (exists (
    (two => (eq (square two) four)) | 
    (neg_two => (eq (square neg_two) four))
))

// Equivalent to:
exists_solution = theorem (
    (eq (square two) four) or (eq (square neg_two) four)
)
```

```logi
// "Both 2 and -2 solve x² = 4"
all_solutions = theorem (forall (
    (two => (eq (square two) four)) | 
    (neg_two => (eq (square neg_two) four))
))

// Equivalent to:
all_solutions = theorem (
    (eq (square two) four) and (eq (square neg_two) four)
)
```

---

## Why This Is Deep

### Partial Functions as Propositional Connectives!

The **domain** of a partial function represents a **set of witnesses**.

**Quantifying over that domain:**
- `exists` = "at least one witness satisfies" = **DISJUNCTION**
- `forall` = "all witnesses satisfy" = **CONJUNCTION**

### This is a Beautiful Encoding!

**Instead of:**
```logi
theorem ((prop a) or (prop b) or (prop c))
```

**We can write:**
```logi
theorem (exists ((a => (prop a)) | (b => (prop b)) | (c => (prop c))))
```

**Both mean:** "At least one of {a, b, c} satisfies prop"

---

## Proof Relevance

### Multi-case functions encode proof-relevant disjunctions!

**Standard disjunction:**
```logi
P ∨ Q
```
"At least one is true, but we don't know which"

**With partial functions:**
```logi
exists ((a => P) | (c => Q))
```

The function **explicitly lists all potential witnesses**: `{a, c}`

This is **constructive**: we know exactly which values we're checking!

---

## Comparison to Standard Logic

### Existential Quantification

**Standard:**
```
∃x ∈ S. P(x)
```
"There exists x in set S such that P(x) holds"

**In Logi:**
```logi
exists ((a => P) | (c => Q) | (e => R))
```

The set S is **explicitly enumerated** as the domain: `{a, c, e}`

**This is finite existential quantification over an explicit set!**

### Universal Quantification

**Standard:**
```
∀x ∈ S. P(x)
```
"For all x in set S, P(x) holds"

**In Logi:**
```logi
forall ((a => P) | (c => Q) | (e => R))
```

Again, S = `{a, c, e}` is explicit.

**This is finite universal quantification over an explicit set!**

---

## When Is This Useful?

### Use Case 1: Finite Case Analysis

```logi
// Check if ANY of {0, 2, 4, 6, 8} is prime
theorem (exists (
    (zero => (isprime zero)) |
    (two => (isprime two)) |
    (four => (isprime four)) |
    (six => (isprime six)) |
    (eight => (isprime eight))
))

// Result: True (because 2 is prime)
```

### Use Case 2: Bounded Search

```logi
// Search for solution in finite domain {1, 2, 3, 4, 5}
find_solution = exists (
    (one => (eq (square one) four)) |
    (two => (eq (square two) four)) |
    (three => (eq (square three) four)) |
    (four => (eq (square four) four)) |
    (five => (eq (square five) four))
)

// Result: True (because 2 satisfies it)
```

### Use Case 3: Verification of Multiple Witnesses

```logi
// Verify ALL of {2, 3, 5, 7} are prime
all_prime = forall (
    (two => (isprime two)) |
    (three => (isprime three)) |
    (five => (isprime five)) |
    (seven => (isprime seven))
)

// Result: True
```

---

## The Type-Theoretic View

In dependent type theory:
```
∃x ∈ {a, c}. P(x)
```

Is a sum type:
```
P(a) + P(c)
```

In Logi:
```logi
exists ((a => P) | (c => Q))
```

The partial function encodes the **explicit enumeration** of the finite domain!

**This is proof-relevant finite quantification.**

---

## Interaction with Pattern Variables

### What if we mix pattern variables with atom patterns?

```logi
exists ((X => (prop X)) | (a => (prop a)))
```

**Case 1:** Pattern `X` binds ANY value to X, returns `(prop X)`
**Case 2:** Pattern `a` matches ONLY atom `a`, returns `(prop a)`

**But wait!** If `X` matches ANY value, it will always match first!

The second case is **unreachable** (assuming left-to-right case matching).

**So this simplifies to:**
```logi
exists (X => (prop X))
```

"There exists some value satisfying prop"

**Lesson:** Put more specific cases before more general cases!

**Correct order:**
```logi
exists ((a => (special_prop a)) | (X => (general_prop X)))
```

- First try specific case: if input is `a`, check special property
- Otherwise, bind to `X` and check general property

---

## Semantics of Case Selection

### Given function:

```logi
f = (a => P) | (c => Q) | (e => R)
```

### Evaluation of `f M`:

**Try cases in order:**
1. Does `M` match pattern `a`? If yes, return `P`
2. Does `M` match pattern `c`? If yes, return `Q`  
3. Does `M` match pattern `e`? If yes, return `R`
4. No match → don't reduce

### Domain: `{a, c, e}`

All values for which at least one case matches.

---

## Quantification Semantics (Precise)

### Existential: `exists f`

**Meaning:** "There exists at least one value in domain(f) such that the result is true"

**For** `f = (a => P) | (c => Q) | (e => R)`:

Domain = `{a, c, e}`

**So:** "At least one of {P, Q, R} is true"

**Logical form:** `P ∨ Q ∨ R`

### Universal: `forall f`

**Meaning:** "For all values in domain(f), the result is true"

**For** `f = (a => P) | (c => Q) | (e => R)`:

Domain = `{a, c, e}`

**So:** "All of {P, Q, R} are true"

**Logical form:** `P ∧ Q ∧ R`

---

## Edge Cases

### Empty Function

```logi
// No cases at all - what does this even mean?
// Can't be written in current syntax!
```

If we allowed it:
```logi
exists (empty_function)
```
Would mean: "There exists a value in the empty domain satisfying the property"

**Result:** False (vacuously)

### Single Case with Variable

```logi
exists (X => (prop X))
```

Domain = all values (X matches anything)

**Meaning:** "There exists SOME value satisfying prop"

**This is true abstract existential!**

### All Atom Cases

```logi
exists ((a => (prop a)) | (b => (prop b)) | (c => (prop c)))
```

Domain = `{a, b, c}` (finite, explicit)

**Meaning:** "At least one of {a, b, c} satisfies prop"

**This is finite existential!**

---

## Summary: The Big Picture

### Three Flavors of Existential

**1. Abstract (infinite domain):**
```logi
exists (X => (prop X))
```
"SOME value exists satisfying prop" (classical existential)

**2. Concrete single witness:**
```logi
exists (a => (prop a))
```
"The value `a` satisfies prop" (constructive witness)

**3. Finite disjunction:**
```logi
exists ((a => (prop a)) | (b => (prop b)) | (c => (prop c)))
```
"At least one of {a, b, c} satisfies prop" (finite search)

**Equivalently:** `(prop a) ∨ (prop b) ∨ (prop c)`

### Three Flavors of Universal

**1. Abstract (infinite domain):**
```logi
forall (X => (prop X))
```
"ALL values satisfy prop" (classical universal)

**2. Concrete single instance:**
```logi
forall (a => (prop a))
```
"The value `a` satisfies prop" (not really universal!)

**3. Finite conjunction:**
```logi
forall ((a => (prop a)) | (b => (prop b)) | (c => (prop c)))
```
"All of {a, b, c} satisfy prop" (finite verification)

**Equivalently:** `(prop a) ∧ (prop b) ∧ (prop c)`

---

## The Answer to Your Question

### `exists (a => b | c => d)` means:

**"There exists a value (either `a` or `c`) such that the function returns something true"**

**Which is:** "`b` is true OR `d` is true"

**Logical form:** `b ∨ d`

### Why?

The function has domain `{a, c}`.

Asking "does there exist an input giving true output?" is asking:

"Is at least one of the outputs {b, d} true?"

**This is a disjunction!**

---

## Philosophical Implications

### Partial Functions as Data Structures

A partial function with multiple cases is like a **finite map**:

```
{a ↦ b, c ↦ d, e ↦ f}
```

Quantifying over this map:
- `exists` = "at least one value in the range satisfies..."
- `forall` = "all values in the range satisfy..."

### This Unifies:

1. **Function definition** (computational)
2. **Finite sets** (data)
3. **Logical connectives** (∨, ∧)
4. **Quantification** (∃, ∀)

**All in one construct!**

---

## Is This a Good Idea?

### Pros

1. **Elegant unification** - One construct does many things
2. **Proof relevance** - Explicit witnesses
3. **Finite reasoning** - Natural for case analysis
4. **Constructive** - Clearly shows what we're checking

### Cons

1. **Potentially confusing** - Mixes computation and logic
2. **Verbose** - Longer than `P or Q`
3. **Redundant** - Can already write `or` and `and` directly

### My Take

**This is a beautiful emergent property of the semantics!**

It's logically sound and reveals deep structure.

**However:** Users should probably just use `or` and `and` directly for clarity.

**But:** This is useful for **meta-reasoning** and understanding the theory!

---

## Recommendation

### For Practical Use

**Prefer direct logical operators:**
```logi
theorem ((prop a) or (prop b))    // Clear
```

**Over quantified partials:**
```logi
theorem (exists ((a => (prop a)) | (b => (prop b))))    // Verbose
```

### But This Is Valuable For:

1. **Finite bounded search**
2. **Explicit case enumeration**
3. **Proof construction with tracked witnesses**
4. **Meta-theoretic understanding**

---

## Final Insight

**Multi-case partial functions under quantification encode finite logical connectives with explicit witness tracking!**

This is:
- Logically sound ✓
- Computationally natural ✓
- Proof-relevant ✓
- Philosophically deep ✓
- Practically verbose ✗

**Allow it, understand it, but don't necessarily encourage it for everyday use!**

---

End of Analysis.
