# Totality Proofs and Induction in Logi

## The Problem

**How do we prove universal properties like:**

"For all booleans x, `(and x (not x))` is false"

**In Logi:**
```logi
theorem (forall (X => (eq (and X (not X)) false)))
```

**But how do we actually PROVE this?**

We need to check it for ALL possible values of X. But there are infinitely many terms! Or are there?

---

## The User's Insight

**The key observation:**

For booleans, there are only TWO constructors: `true` and `false`.

If we can prove the property for both constructors, we've covered all cases!

**Proof by case analysis:**
- Case X = true: (and true (not true)) = (and true false) = false ✓
- Case X = false: (and false (not false)) = (and false true) = false ✓
- Therefore: for all booleans, the property holds ✓

**This is exhaustive case analysis / totality proof.**

---

## Approach 1: Manual Case Checking (Current Capability)

### For Finite Types (Like Bool)

```logi
// Define the property we want to prove
prop = X => (eq (and X (not X)) false)

// Check all cases manually
case_true = prop true      // → (eq (and true false) false) → true ✓
case_false = prop false    // → (eq (and false true) false) → true ✓

// Claim: we've checked all cases
// Therefore: forall X, prop(X) holds
```

**But this doesn't give us a proof term for `forall (X => prop X)`**

We've just checked two instances. How do we conclude the universal?

---

## Approach 2: Explicit Exhaustiveness Witness

### Encode the Proof Structure

```logi
// A totality proof for Bool is a pair of proofs for true and false
bool_cases = P => (pair (P true) (P false))

// Now we can construct a totality proof:
prop = X => (eq (and X (not X)) false)

totality_proof = bool_cases prop
// → (pair (prop true) (prop false))
// → (pair true true)

// This witness demonstrates we've checked all cases
```

**Interpretation:** `bool_cases P` proves that P holds for all booleans by checking both cases.

---

## Approach 3: Induction Principle as a Combinator

### For Bool

```logi
// The induction principle for Bool
bool_ind = P => case_true => case_false => X => 
    (X matches true ? case_true : (X matches false ? case_false : stuck))
```

Wait, but we said we can't pattern match on X in the body if X is from outside scope...

Actually, let me reconsider. The user said:

> "I think we might also allow some sort of pattern matching on internals of functions so long as the internal which we are pattern matching on is externally defined such that it is in scope of the function doing the pattern matching."

**What does this mean?**

I think the user is suggesting: if X is a parameter, we should be able to case-split on it in the body, even though X is a variable.

**Example:**
```logi
// Case split on X (which is a variable)
bool_cases = X => 
    X true  => result_for_true |    // If X is true
    X false => result_for_false     // If X is false
```

Hmm, but this doesn't quite make sense either because X isn't a function here...

Let me re-read: "so long as the internal which we are pattern matching on is externally defined such that it is in scope"

I think the user means: allow pattern matching on a variable IF we know its type/constructors.

So:
```logi
bool_ind = X => 
    true => result_for_true |
    false => result_for_false
```

Wait, that's just a regular pattern matching function!

Let me think differently...

---

## Approach 4: The Function IS the Proof

### Key Insight: Functions That Handle All Cases ARE Totality Proofs

```logi
// This function is ITSELF a proof of totality
proof = true => (eq (and true (not true)) false) |
        false => (eq (and false (not false)) false)
```

This function:
1. Accepts true → computes to true (the proof for true case)
2. Accepts false → computes to true (the proof for false case)
3. Accepts nothing else (partial function with domain {true, false})

**Claim:** If a function's domain is exactly the set of all constructors for a type, AND it returns true for each, THEN it's a proof of the universal property.

**In other words:**
```logi
proof = true => true | false => true

// This is equivalent to:
theorem (forall (X => (eq (and X (not X)) false)))
```

**But how do we KNOW that {true, false} is exhaustive for booleans?**

---

## The Core Challenge: Type Information

**The problem:** In a minimalist untyped language, we don't have type information!

We don't know:
- What type X has
- What constructors exist for that type
- Whether we've covered all cases

**Possible solutions:**

### Solution A: Explicit Type Declaration (Not Minimal)

```logi
type Bool = true | false

theorem (forall X : Bool => (eq (and X (not X)) false))
```

But we wanted to avoid a type system!

### Solution B: Convention-Based Exhaustiveness

**Rule:** A partial function with specific atom patterns is considered exhaustive if we say so.

```logi
// We claim this is exhaustive for booleans
proof = true => true | false => true

// The system trusts us
theorem (forall proof)
```

**Problem:** No static checking, easy to make mistakes.

### Solution C: Induction Principles as Primitives

**Provide built-in induction principles for each "type":**

```logi
// Built-in: bool_ind
bool_ind : (true : P true) -> (false : P false) -> (forall X, P X)

// Usage:
proof_true = eq (and true (not true)) false    // true
proof_false = eq (and false (not false)) false  // true

totality = bool_ind proof_true proof_false
// Now totality proves: forall X, (eq (and X (not X)) false)
```

**This works but requires built-ins.**

---

## Let Me Reconsider the User's Proposal

> "I think we might also allow some sort of pattern matching on internals of functions so long as the internal which we are pattern matching on is externally defined such that it is in scope of the function doing the pattern matching."

**Interpretation:** Allow inspecting/matching on function bodies if the variables in those bodies are accessible from the outside.

**Example:**
```logi
// Function to prove
f = X => (eq (and X (not X)) false)

// Can we inspect f's body?
// The body is: (eq (and X (not X)) false)
// X is bound by f's pattern

// From outside, can we say:
// "For all instantiations of X, f X is true"?
```

**Idea:** Allow a proof tactic that:
1. Takes a function f = X => Body
2. Enumerates all possible values for X
3. Checks that Body[X → value] holds for each
4. Concludes: forall X, f X holds

**This requires:**
- Knowing what "all possible values" are (type information)
- Being able to inspect function bodies (breaks opacity?)

Hmm, this is tricky...

---

## Alternative Interpretation: Pattern Matching on Functions with External Binders

**Maybe the user means:**

```logi
// Outside scope
X = some_value

// Function that uses X from outer scope
f = Y => (plus X Y)

// Can we pattern match on f to extract that it uses X?
inspect = (Y => (plus X Y)) => X    // Extract the outer X?
```

**But this still has the opacity problem...**

---

## Let Me Try a Different Angle: Totality Through Evaluation

### Idea: Prove Totality by Explicit Case Enumeration

**For Bool:**
```logi
// Define what it means for a predicate to hold for all bools
forall_bool = P => (and (P true) (P false))

// Our property
prop = X => (eq (and X (not X)) false)

// Proof:
theorem (forall_bool prop)
// Expands to: (and (prop true) (prop false))
// Evaluates to: (and true true)
// Result: true ✓
```

**This gives us totality checking for finite types!**

---

## For Nat: The Induction Problem

**Nat has two constructors:**
- zero
- succ N (for any Nat N)

**Nat is infinite!** We can't enumerate all cases.

**We need induction:**

**Induction principle for Nat:**
```
To prove: forall N : Nat, P(N)
Prove:
  1. Base case: P(zero)
  2. Inductive case: forall N, P(N) -> P(succ N)
Then conclude: forall N, P(N)
```

**In Logi:**
```logi
nat_ind = P => base => step => N =>
    N matches zero ? base :
    N matches (succ N') ? (step N' (nat_ind P base step N')) :
    stuck
```

**Wait, this is just structural recursion!**

---

## Key Insight: Structural Recursion IS Induction

**Example: Prove that for all N, (plus N zero) = N**

```logi
// Define the property
prop = N => (eq (plus N zero) N)

// Prove by structural recursion
proof = zero => true |    // Base case: (plus zero zero) = zero ✓
        (succ N) => (proof N)    // Inductive case: if holds for N, holds for (succ N)

// This function IS the proof!
```

**The structure of the recursive function corresponds to the induction principle!**

---

## Back to the User's Question

**Q: "If we don't do this, how could we create totality proofs?"**

**A: Use structural recursion for infinite types, explicit case enumeration for finite types.**

### For Finite Types (Bool, Enum)

```logi
// Explicit case split
proof = true => check_true | false => check_false

// Or, use a helper:
forall_bool = P => (and (P true) (P false))
```

### For Recursive Types (Nat, List)

```logi
// Structural recursion
proof = zero => base_case |
        (succ N) => (combine (proof N) (step N))
```

**The key:** The pattern matching structure of the function encodes the proof structure!

---

## Allowing Pattern Matching on Function Internals?

**User's proposal:** "allow some sort of pattern matching on internals of functions so long as the internal which we are pattern matching on is externally defined such that it is in scope"

**Let me try a concrete example of what this might enable:**

```logi
// Suppose we have a function
f = X => (and X (not X))

// Can we write a "totality checker" that inspects f?
check_totality = F => 
    // Somehow enumerate all inputs
    // Check F input evaluates to expected value
    // Conclude universality
```

**The problem:** How do we enumerate "all inputs" without type information?

**Possible answer:** We can't, in general. But for specific known types, we could provide special checking functions.

```logi
// Totality checker for Bool
forall_bool = P => (and (P true) (P false))

// Totality checker for Nat (requires induction)
forall_nat = P => base => step => 
    (and base (forall (N => (imp (P N) (step N)))))
```

Wait, this still requires the user to provide base and step cases...

---

## My Proposed Solution: Induction Principles + Structural Pattern Matching

### Core Idea

**Don't inspect function internals. Instead, use the function's PATTERN MATCHING STRUCTURE to encode proofs.**

**For finite types:** Use explicit case split

**For infinite types:** Use structural recursion (which IS induction)

### Example: Bool Totality

```logi
// Property: for all X, (and X (not X)) = false
prop = X => (eq (and X (not X)) false)

// Proof by cases (this function IS the proof)
proof = true => (prop true) |    // Reduces to true
        false => (prop false)    // Reduces to true

// To check totality, we check that proof handles all constructors of Bool
// If proof's domain is {true, false}, we've covered all cases
```

**Key insight:** The DOMAIN of the proof function tells us what cases are covered.

**If domain = {true, false} = all bool constructors, proof is total.**

### Example: Nat Induction

```logi
// Property: for all N, (plus N zero) = N
prop = N => (eq (plus N zero) N)

// Proof by induction (structural recursion)
proof = zero => true |    // Base case
        (succ N) => (proof N)    // Inductive step

// This proof recursively covers all naturals
```

**The recursive structure encodes the induction principle.**

---

## The User's Question About More Complicated Types

**User:** "But how would this work for more complicated types and constructors? Think about Nat with 2 constructors"

**Answer:** Use structural pattern matching + recursion!

### Nat Example

**Constructors:** `zero`, `(succ N)`

**Proof pattern:**
```logi
proof = zero => base_case |
        (succ N) => (inductive_step N (proof N))
```

**The recursive call `(proof N)` is the inductive hypothesis!**

### List Example

**Constructors:** `nil`, `(cons H T)`

**Proof pattern:**
```logi
proof = nil => base_case |
        (cons H T) => (inductive_step H T (proof T))
```

**Again, `(proof T)` is the inductive hypothesis for the tail.**

### Tree Example

**Constructors:** `leaf`, `(node L V R)`

**Proof pattern:**
```logi
proof = leaf => base_case |
        (node L V R) => (combine (proof L) V (proof R))
```

**Recursive calls for both subtrees!**

---

## Totality Checking: Can We Automate This?

**Question:** Can the system check that a proof is total?

**For finite types (Bool, Unit):**

Check that the function's domain covers all constructors.

```logi
proof = true => ... | false => ...
// Domain = {true, false}
// Bool constructors = {true, false}
// Coverage: complete ✓
```

**For infinite types (Nat, List):**

Check that the function is structurally recursive.

```logi
proof = zero => ... |
        (succ N) => ... (proof N) ...
// Recursive call on substructure ✓
// Terminates ✓
```

**This is termination checking!**

---

## Do We Need to Inspect Function Internals?

**My answer: No, if we use the right encoding.**

**Key principles:**

1. **Proofs are functions** that pattern match on all constructors
2. **Case coverage** is determined by the function's patterns
3. **Induction** is encoded as structural recursion
4. **Totality checking** is coverage checking + termination checking

**We DON'T need to inspect function bodies.** The pattern structure tells us everything!

---

## But What If We Want Meta-Theoretic Reasoning?

**Example:** "Prove that this function is total"

```logi
f = true => result_true | false => result_false

// Meta-level claim: f is total for Bool
totality_proof = prove_total f bool_type
```

**For this, we might need:**

1. **Type information:** What type does f operate on?
2. **Constructor enumeration:** What are all constructors of that type?
3. **Coverage checking:** Does f handle all constructors?

**This goes beyond the minimal core language.**

**Possible approaches:**

### Approach A: External Checker

Separate tool that analyzes functions and checks totality.

```bash
logi-check totality proof.logi
# Checking function 'proof'...
# Type: Bool -> Bool
# Constructors: {true, false}
# Coverage: {true, false}
# Result: Total ✓
```

### Approach B: Proof Certificates

User provides explicit certificate that function is total.

```logi
proof = true => ... | false => ...

certificate = total_for_bool proof {true, false}
// Checks that proof's domain equals {true, false}
```

### Approach C: Built-in Induction Principles

```logi
// Built-in for each type
bool_elim : P -> P -> (Bool -> P)
bool_elim case_true case_false = 
    true => case_true |
    false => case_false

// Usage
proof = bool_elim proof_true proof_false
// Guaranteed total by construction
```

---

## My Recommendation

**Combine structural pattern matching with explicit induction principles.**

### For Finite Types

**Provide helper functions:**
```logi
// Helper that encodes "for all booleans"
forall_bool = P => (and (P true) (P false))

// Usage
prop = X => (eq (and X (not X)) false)
theorem (forall_bool prop)
// Expands and evaluates to: true
```

**The helper `forall_bool` explicitly enumerates all constructors.**

### For Infinite Types

**Use structural recursion directly:**
```logi
// Proof by structural recursion (this IS induction)
proof = zero => base_case |
        (succ N) => (inductive_step N (proof N))
```

**Optionally, provide induction combinators:**
```logi
nat_ind = base => step => 
    zero => base |
    (succ N) => (step N (nat_ind base step N))

// Usage
proof = nat_ind base_case inductive_step
```

---

## Answering the Specific Questions

### Q: "Should we allow pattern matching on function internals?"

**A: No, but we don't need to!**

The pattern matching STRUCTURE (which constructors the function handles) gives us all the information we need for totality.

### Q: "How could we create totality proofs?"

**A: Use explicit case enumeration or structural recursion.**

**For finite types:**
```logi
forall_bool = P => (and (P true) (P false))
```

**For infinite types:**
```logi
proof = zero => base | (succ N) => (step N (proof N))
```

### Q: "Should we have enumeration totality rule for booleans?"

**A: Yes! As a helper function:**
```logi
forall_bool = P => (and (P true) (P false))
```

### Q: "How would this work for more complicated types like Nat?"

**A: Use structural recursion:**
```logi
forall_nat = P => base => step =>
    (proof where
        proof = zero => base |
                (succ N) => (and base (step N (proof N)))
    )
```

Wait, this doesn't quite work as stated...

Actually, for Nat, we can't enumerate all values. We need the user to provide the proof by structural recursion:

```logi
// User writes this:
proof = zero => base_case |
        (succ N) => (inductive_case N (proof N))

// This IS the induction proof
```

---

## Concrete Proposal

### Add Totality Checkers as Standard Library

```logi
// For Bool
forall_bool = P => (and (P true) (P false))

// For Option
forall_option = P => forall_inner => 
    (and (P none) (forall_inner (V => (P (some V)))))

// For List (requires induction)
forall_list = P => base => step =>
    // Can't enumerate - user must provide structural recursive proof
    // Just a type signature / marker
```

### Examples

**Bool totality:**
```logi
prop = X => (eq (and X (not X)) false)
theorem (forall_bool prop)    // Checks both cases
```

**Nat induction:**
```logi
prop = N => (eq (plus N zero) N)

// User provides inductive proof
proof = zero => true |
        (succ N) => (proof N)

theorem (forall proof)    // Proof covers all nats by induction
```

---

## Summary

1. **Don't need to inspect function internals**
2. **Pattern structure encodes proof structure**
3. **Finite types:** explicit case enumeration with helpers
4. **Infinite types:** structural recursion (IS induction)
5. **Totality checking:** coverage + termination analysis (could be external tool)

**The key insight:** In Logi, proofs ARE functions. The way a function pattern-matches encodes the logical structure of the proof.

---

End of Document.
