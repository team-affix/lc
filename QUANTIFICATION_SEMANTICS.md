# Quantification of Partial Functions

## The Question

**What does it mean to quantify over partial functions?**

```logi
exists (A => BODY)    // Total function
exists (a => BODY)    // Partial function
```

---

## Understanding Total vs. Partial Functions

### Total Function: `A => BODY`

**Meaning:** Accepts ANY argument, binds it to `A`, returns `BODY`

```logi
f = A => (prop A)

f x       → (prop x)     ✓
f y       → (prop y)     ✓
f true    → (prop true)  ✓
```

**Domain:** All values

### Partial Function: `a => BODY`

**Meaning:** Accepts ONLY atom `a`, returns `BODY`. For anything else, doesn't reduce.

```logi
f = a => (prop a)

f a       → (prop a)     ✓
f x       → (f x)        ✗ (doesn't reduce)
f true    → (f true)     ✗ (doesn't reduce)
```

**Domain:** Just `{a}`

---

## Quantification Semantics

### Universal Quantification: `forall P`

**Informal meaning:** "For all values X, P(X) is true"

**Formal:** `∀x. P(x)` means P(x) holds for every value x in the domain

**With total function:**
```logi
forall (A => (prop A))
```
Means: "For ALL values X, (prop X) is true"

The function `(A => (prop A))` accepts any X, so we're quantifying over all values.

**With partial function:**
```logi
forall (a => (prop a))
```

**Question:** What does this mean?

**Interpretation 1:** "For all X such that `(a => (prop a)) X` reduces, it's true"
- Only `a` makes it reduce
- So this means: "(prop a) is true"
- Vacuously true for all other values (they don't satisfy the precondition)

**Interpretation 2:** "For all X in the domain of the function, it's true"
- Domain = {a}
- So this means: "(prop a) is true"

**Both interpretations converge:** This is just asserting `(prop a)` with extra ceremony.

### Existential Quantification: `exists P`

**Informal meaning:** "There exists some value X such that P(X) is true"

**With total function:**
```logi
exists (A => (prop A))
```
Means: "There exists SOME value X such that (prop X) is true"

The function accepts any X, so we're asking: is there at least one value that satisfies `prop`?

**With partial function:**
```logi
exists (a => (prop a))
```

**Question:** What does this mean?

**Interpretation 1:** "There exists X such that `(a => (prop a)) X` reduces to true"
- Only `a` makes it reduce
- So we're asking: does `(a => (prop a)) a` reduce to true?
- That is: is `(prop a)` true?

**Interpretation 2:** "There exists X in the domain such that applying the function gives true"
- Domain = {a}
- So: does `(prop a)` hold?

**Insight:** `exists (a => (prop a))` is equivalent to just asserting `(prop a)`!

---

## Deep Analysis

### The Partial Function as a Constraint

When we write:
```logi
exists (a => BODY)
```

We're saying: "There exists a value such that when given to this function, we get BODY."

**But the function only accepts `a`!**

So we're really saying: "There exists a value (namely `a`) that this function accepts, and for that value, we get BODY."

**This is logically equivalent to:** "`a` is in the domain AND BODY holds when we pass `a`"

Which simplifies to just: "BODY holds for `a`"

### Example: Even Numbers

**With total function:**
```logi
exists (X => (iseven X))
```
Means: "There exists some number that is even"

This could be 0, 2, 4, 6, etc. The function accepts any X.

**With partial function:**
```logi
exists (two => (iseven two))
```
Means: "There exists a value that makes `(two => (iseven two))` return true"

Only `two` makes the function reduce: `(two => (iseven two)) two → (iseven two)`

So this is asking: "Does `(iseven two)` hold?"

**This is weaker!** We're only checking that TWO is even, not that ANY even number exists.

---

## Philosophical Interpretation

### Partial Functions as Witnesses

**Consider:**
```logi
exists (a => BODY)
```

The partial function `(a => BODY)` is like saying:
- "I claim there's a witness"
- "The witness is `a`"
- "And for that witness, BODY holds"

**This is CONSTRUCTIVE existential quantification!**

The partial function explicitly names the witness (`a`) while the total function leaves it abstract.

### Comparison

**Total function (abstract witness):**
```logi
exists (X => (prop X))
```
"There exists SOME value X such that (prop X) holds"
- We don't specify which X
- Could be many values
- Abstract existential

**Partial function (concrete witness):**
```logi
exists (a => (prop a))
```
"There exists a value (specifically `a`) such that (prop a) holds"
- We specify exactly which value: `a`
- Constructive witness
- Concrete existential

---

## Practical Implications

### Use Case 1: Constructive Proofs

```logi
// Abstract: there exists an even number
theorem1 = theorem (exists (X => (iseven X)))

// Concrete: here's an even number (it's 2)
theorem2 = theorem (exists (two => (iseven two)))
```

`theorem2` is STRONGER - it provides a witness!

### Use Case 2: Specific Instance Proofs

```logi
// "There exists a solution to this equation"
exists_solution = theorem (exists (X => (eq (square X) four)))

// "Two is a solution to this equation"
two_is_solution = theorem (exists (two => (eq (square two) four)))
```

The second is more specific - it says "here's a solution: it's two."

### Use Case 3: Partial Specifications

```logi
// Some number is prime
general = exists (X => (isprime X))

// This specific number is prime
specific = exists (seventeen => (isprime seventeen))
```

---

## Logical Equivalences

### For Universal Quantification

**Total:**
```logi
forall (X => (prop X))
```
≡ "For all X, prop(X) holds"

**Partial:**
```logi
forall (a => (prop a))
```
≡ "For all X in domain {a}, prop(X) holds"  
≡ "prop(a) holds"

**The partial version is just a specific instance, not a true universal!**

### For Existential Quantification

**Total:**
```logi
exists (X => (prop X))
```
≡ "There exists some X such that prop(X) holds"

**Partial:**
```logi
exists (a => (prop a))
```
≡ "There exists X in domain {a} such that prop(X) holds"  
≡ "`a` is a witness such that prop(a) holds"  
≡ "prop(a) holds" (with explicit witness)

**The partial version provides a constructive witness!**

---

## Interesting Consequences

### 1. Partial Functions Can Encode Specific Instances

Instead of saying:
```logi
theorem (prop a)    // "prop holds for a"
```

We could say:
```logi
theorem (exists (a => (prop a)))    // "there exists a value (namely a) satisfying prop"
```

These are logically equivalent, but the second has different flavor (existential with witness).

### 2. Pattern of Generalization

**Start specific:**
```logi
theorem (exists (two => (iseven two)))    // "2 is even"
```

**Generalize:**
```logi
theorem (exists (four => (iseven four)))   // "4 is even"
theorem (exists (six => (iseven six)))     // "6 is even"
```

**Full generalization:**
```logi
theorem (exists (X => (iseven X)))         // "some number is even"
```

Or even:
```logi
theorem (forall (X => (imp (iseven X) (exists (Y => (eq Y X))))))
// "For all even numbers, there exists a witness (itself)"
```

### 3. Partial Functions as Proof Terms

The partial function itself serves as a proof term:

```logi
proof = a => (prop a)
```

This encodes:
- Witness: `a`
- Property: `(prop a)`

Then:
```logi
theorem (exists proof)
```

**The partial function IS the proof!**

---

## Comparison to Standard Logic

### Standard Existential

**Standard notation:**
```
∃x. P(x)
```

**In Logi with total function:**
```logi
exists (X => (P X))
```

**Semantics:** "There exists at least one value satisfying P"

### Constructive Existential (with Witness)

**Standard notation (in constructive logic):**
```
∃x:τ. P(x) with witness w
```

**In Logi with partial function:**
```logi
exists (a => (P a))
```

**Semantics:** "The value `a` satisfies P"

**The partial function NAMES the witness!**

---

## Should This Be Allowed?

### Arguments FOR Allowing It

1. **Constructive proofs** - Provides explicit witnesses
2. **More expressive** - Can encode both abstract and concrete existentials
3. **Natural** - Falls out of the semantics naturally
4. **Useful** - Can track specific instances

### Arguments AGAINST Allowing It

1. **Confusing** - Might blur the line between universal and specific
2. **Redundant** - `exists (a => BODY)` is just `BODY[a]`
3. **Type confusion** - What's the "type" of quantified partial functions?

### My Analysis

**It's logically sound but potentially confusing.**

**Recommendation:**

**Allow it but discourage it by convention.**

**Preferred style:**
```logi
// For abstract existentials, use total functions
theorem (exists (X => (iseven X)))

// For specific instances, just assert directly
theorem (iseven two)
```

**But if someone writes:**
```logi
theorem (exists (two => (iseven two)))
```

It's valid, just redundant (equivalent to `theorem (iseven two)`).

---

## Semantic Analysis

### What Does the Quantifier Do?

**Universal quantification `forall P`:**

**Informally:** "For all X, P(X) is true"

**Operationally:** "The function P returns true for every possible input"

**With partial function:**
```logi
forall (a => BODY)
```

"The function `(a => BODY)` returns true for every value in its domain"

Domain = {a}

So: "For the value `a`, BODY is true"

**This is just a specific instance, not a universal claim!**

### Existential quantification `exists P`:

**Informally:** "There exists at least one X such that P(X) is true"

**Operationally:** "There is at least one input for which P returns true"

**With partial function:**
```logi
exists (a => BODY)
```

"There exists at least one value in the domain of `(a => BODY)` such that the result is true"

Domain = {a}

So: "For the value `a`, BODY is true"

**Again, this is just asserting BODY for the specific value `a`!**

---

## The Real Insight

**Partial functions make quantification CONCRETE.**

### Universal becomes Specific

```logi
forall (X => P X)    // "For ALL X, P(X)"
forall (a => P a)    // "For THE VALUE a, P(a)"  (not really universal!)
```

### Existential becomes Witnessed

```logi
exists (X => P X)    // "SOME X exists where P(X)"
exists (a => P a)    // "THE VALUE a exists where P(a)"  (constructive witness!)
```

---

## Type-Theoretic Perspective

In dependent type theory:
```
∃x:τ. P(x)
```

This is a pair: `(witness, proof that P(witness))`

**In Logi with partial functions:**
```logi
exists (a => (P a))
```

The partial function `(a => (P a))` encodes:
- witness = `a` (the domain of the function)
- proof = `(P a)` (the body)

**The partial function IS the existential proof term!**

---

## Examples with Concrete Semantics

### Example 1: Even Numbers

**Abstract existential:**
```logi
theorem (exists (X => (iseven X)))
```
"There exists some number that is even"

**Reduction semantics:** "There is at least one X such that `(X => (iseven X)) X` reduces to true"

Since `X` is a variable pattern, this applies to any value. We're claiming: "pick any X, there's at least one value such that (iseven X) is true."

**Concrete existential:**
```logi
theorem (exists (two => (iseven two)))
```
"There exists a number (namely two) that is even"

**Reduction semantics:** "There is at least one value such that `(two => (iseven two))` applied to it reduces to true"

The only value that works is `two`:
```logi
(two => (iseven two)) two  → (iseven two)
```

So this is claiming: "two satisfies (iseven two)"

Which is equivalent to:
```logi
theorem (iseven two)
```

### Example 2: Solutions to Equations

**Abstract:**
```logi
theorem (exists (X => (eq (square X) four)))
```
"There exists a solution to x² = 4"

Could be 2 or -2.

**Concrete:**
```logi
theorem (exists (two => (eq (square two) four)))
```
"Two is a solution to x² = 4"

Equivalent to:
```logi
theorem (eq (square two) four)
```

---

## When is This Useful?

### Use Case: Multiple Witnesses

```logi
// Claim: both 2 and -2 are solutions
theorem (exists (two => (eq (square two) four)))
theorem (exists (neg two) => (eq (square (neg two)) four)))
```

vs. just:
```logi
theorem (eq (square two) four)
theorem (eq (square (neg two)) four)
```

**No real difference!**

### Use Case: Distinguishing "Exists" from Direct Assertion

**Direct assertion:**
```logi
theorem (iseven two)
```
"Two is even" (simple fact)

**Existential with witness:**
```logi
theorem (exists (two => (iseven two)))
```
"There exists an even number, and it's two" (existential claim with proof)

**Semantically equivalent, but different logical flavor.**

---

## The Surprising Conclusion

**Quantifying over partial functions doesn't add expressive power!**

**For universal quantification:**
```logi
forall (a => BODY)  ≡  BODY[a]
```

Just a specific instance, not a universal.

**For existential quantification:**
```logi
exists (a => BODY)  ≡  BODY[a]
```

Just asserting BODY for specific value `a`.

**However:**

### This Might Be a Feature, Not a Bug!

**Partial functions provide CONSTRUCTIVE WITNESSES automatically.**

When you write:
```logi
prove_exists = theorem (exists (two => (iseven two)))
```

The partial function `(two => (iseven two))` ENCODES:
1. The witness (`two`)
2. The proof (`(iseven two)`)

**This is proof-relevant existential quantification!**

---

## Recommendation

### Allow It, But Discourage It

**Legal (but redundant):**
```logi
theorem (exists (a => (prop a)))
```

**Preferred:**
```logi
theorem (prop a)
```

**Legal and occasionally useful:**
```logi
// When you want to emphasize witness
witness_proof = (a => (prop a))
claim = theorem (exists witness_proof)
```

### When It's Actually Useful

**Multiple witnesses:**
```logi
witness1 = two => (iseven two)
witness2 = four => (iseven four)

theorem (exists witness1)
theorem (exists witness2)
```

This makes it clear we're providing multiple constructive proofs.

**vs. just:**
```logi
theorem (iseven two)
theorem (iseven four)
```

**No real semantic difference, but stylistic clarity in proof construction.**

---

## Type-Level Interpretation

If we had types, we might write:

```logi
exists : (τ → Prop) → Prop
```

But when given a partial function:
```logi
(a => BODY) : ??? → Prop
```

The domain is `{a}`, a singleton type!

So:
```logi
exists ((a => BODY)) : Prop
```

Is claiming: "There exists a value in {a} such that BODY holds"

Which is trivially equivalent to: "BODY holds for a"

---

## Summary

### The Question

What does `exists (a => BODY)` mean?

### The Answer

**Logically:** "There exists a value (specifically `a`) such that BODY holds"

**Operationally:** "The partial function `(a => BODY)` has at least one input (`a`) that produces true"

**Equivalently:** Just `BODY` with `a` substituted

### Why It's Interesting

1. **Provides constructive witness** - The partial function names the witness
2. **Proof relevance** - The function IS the proof term
3. **Natural encoding** - Falls out of the semantics
4. **Not harmful** - Logically sound, just redundant

### Recommendation

**Allow it (it's sound), but style guide should prefer:**

```logi
// Direct assertion (preferred)
theorem (prop a)

// Over redundant existential
theorem (exists (a => (prop a)))
```

**Unless you specifically want to emphasize the constructive witness in the proof.**

---

## Final Thought

**This reveals something deep about Logi:**

Partial functions naturally encode **proof-relevant existentials** where the function's domain explicitly names the witnesses.

This is actually quite elegant and could be useful for proof construction, even if it's technically redundant!

---

End of Analysis.
