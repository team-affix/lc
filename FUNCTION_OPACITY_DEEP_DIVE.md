# Deep Dive: Function Opacity and Unification Directionality

## The Central Questions

1. **Should we allow ANY pattern matching on functions?** Even with variable patterns?
2. **What exactly does one-directional unification mean?**
3. **What behavior do we lose if functions are completely opaque?**
4. **Can we find edge cases that break the system?**

Let me argue with myself systematically.

---

## Question 1: Can Functions Match Variable Patterns?

### Position A: "Yes - Variable Patterns Should Match Anything"

**Argument:**

A variable pattern like `F` should match ANY value, including functions.

```logi
apply_to_five = F => (F five)

my_square = X => (square X)
result = apply_to_five my_square
// F binds to (X => (square X))
// Result: ((X => (square X)) five) → (square five) ✓
```

This is essential for higher-order programming! If F can't bind to functions, we lose:
- Passing functions as arguments
- Returning functions from functions
- Function composition
- Combinators

**This seems non-negotiable for a functional language.**

**Counterargument to myself:**

But what IS the value of F after binding? Is it the function (X => (square X)) as-is? Or is it some opaque reference?

If we allow `F` to bind to a function, can we then inspect F? If not, what can we do with it?

**Answer:** We can APPLY it, but not DECOMPOSE it.

```logi
apply_to_five = F => (F five)    // ✓ Can apply F
decompose = F => (F matches (X => BODY) ? BODY : error)    // ✗ Can't decompose
```

**Conclusion:** Variable patterns CAN match functions, but only as opaque values.

---

### Position B: "No - Functions Should Be Completely Opaque"

**Argument:**

Maybe even binding with a variable pattern is too permissive. Perhaps functions should only be applicable, never passed around as values.

```logi
// Not allowed:
apply_to_five = F => (F five)    // ✗ Can't bind functions to variables

// Only allowed:
(X => (square X)) five    // ✓ Direct application
```

This is similar to some restricted lambda calculi where functions are not first-class.

**Counterargument to myself:**

This is WAY too restrictive! We'd lose:
- Higher-order functions
- Quantification (exists, forall take predicates as arguments!)
- Function composition
- Most of the expressiveness

**Example that breaks:**
```logi
exists (X => (iseven X))
```

Here, `exists` takes the predicate `(X => (iseven X))` as an argument. If functions can't be passed as values, this is impossible!

**Conclusion:** This is unworkable. Functions MUST be first-class values.

---

### Resolution: Variable Patterns Match Functions Opaquely

**Rule:** Variable patterns CAN bind to functions, but the function remains opaque (non-decomposable).

```logi
// ✓ Allowed: opaque binding
process = F => (F arg)

// ✓ Allowed: apply bound function
apply_to_five = F => (F five)

// ✗ Not allowed: decompose function
extract_body = (X => BODY) => BODY
```

**This gives us:**
- First-class functions ✓
- Higher-order programming ✓
- Quantification ✓
- Semantic stability ✓

---

## Question 2: What Does One-Directional Unification Mean?

Let me construct some challenging examples to test the boundaries.

### Example 1: The Basic Case

```logi
f = a => result
f X    // Does this reduce?
```

**Two-way unification (Prolog-style):**
- Unify atom `a` with variable `X`
- Solution: X = a
- Both terms get modified: pattern becomes `X` and argument becomes `a`
- Result: reduces

**One-way unification (Pattern-matching-style):**
- Try to match pattern `a` against value `X`
- Pattern is concrete (atom), value is variable
- To match, we'd need to MODIFY the pattern to become `X`
- This violates one-directionality!
- Result: does NOT reduce

**Which is right for Logi?**

**Argument for two-way:** More expressive, enables logic programming features

**Argument for one-way:** Simpler semantics, preserves semantic stability, aligns with functional programming

**My position:** One-way is correct for Logi because:
1. Functions are values with fixed definitions
2. Applying a function shouldn't change the function
3. Semantic stability is paramount

---

### Example 2: Both Sides Have Variables

```logi
f = A => (plus A B)    // B is free - error by our rules
// Let's fix it:
f = A => (plus A a)    // a is an atom

f X    // Does this reduce?
```

**Analysis:**
- Pattern: `A` (variable)
- Argument: `X` (variable)
- Match: A binds to X
- Body: (plus X a)
- Result: reduces to (plus X a) ✓

This works! A is a pattern variable, X is a value variable. A binds to X.

**But wait - what if:**
```logi
g = X => result
g A    // Does this reduce?
```

- Pattern: `X` (variable in the function's pattern)
- Argument: `A` (variable in the calling context)
- Match: X binds to A
- Body: result
- Result: reduces to result ✓

**Both work! Because both patterns are variables, they unify (one-directionally) by binding pattern variable to value variable.**

---

### Example 3: The Tricky Case - Nested Variables

```logi
f = (cons A B) => (plus A B)
f (cons X Y)    // Does this reduce?
```

**Analysis:**
- Pattern: (cons A B) where A, B are pattern variables
- Value: (cons X Y) where X, Y are value variables
- Match cons against cons: ✓
- Match A against X: A binds to X ✓
- Match B against Y: B binds to Y ✓
- Body: (plus A B) becomes (plus X Y)
- Result: reduces to (plus X Y) ✓

**This works!**

**But consider:**
```logi
f = (cons a b) => result
f (cons X Y)    // Does this reduce?
```

**Analysis:**
- Pattern: (cons a b) where a, b are atoms
- Value: (cons X Y) where X, Y are value variables
- Match cons against cons: ✓
- Match a against X: Would need to bind atom `a` to variable `X` - backwards! ✗
- Result: does NOT reduce ✗

**Wait, or should it?**

**Argument FOR reduction:**
In logic programming (Prolog), this would unify by binding X=a, Y=b. The variables in the value get bound.

**Argument AGAINST reduction:**
In functional pattern matching, the function's pattern is fixed. We're checking "does the argument match this pattern?" not "can we make them match?"

**Which is right?**

Let me think about the semantics...

**If we say it reduces:**
```logi
f = (cons a b) => result
f (cons X Y)  →  result (with substitution X=a, Y=b in the calling context)
```

But what does "substitution in the calling context" mean? 

```logi
g = (cons X Y) => (f (cons X Y))
// When evaluating f (cons X Y):
// Does X get bound to a and Y get bound to b?
// Then the body of g becomes... what?
```

This is getting weird. We'd be MODIFYING the argument in the calling context.

**If we say it doesn't reduce:**
```logi
f = (cons a b) => result
f (cons X Y)  →  (f (cons X Y))  // stuck
```

The function expects concrete atoms `a` and `b`, but got variables. No match.

**I think "doesn't reduce" is correct.** Here's why:

**The function is saying:** "I only accept the specific value (cons a b)."

**The argument is:** "(cons X Y)" where X and Y are unknowns.

**Until we know what X and Y are, we can't determine if the argument matches the pattern.**

**This aligns with one-directional matching:** We're testing if the value matches the pattern, not making them match by modifying the value.

---

### Example 4: The Quantifier Case

```logi
theorem (exists (X => (iseven X)))    // "Some number is even"

// Later, we want to match this against a pattern:
process = (exists P) => (check_predicate P)

process (theorem (exists (X => (iseven X))))
```

**Analysis:**
- Pattern: (exists P)
- Value: (exists (X => (iseven X)))
- Match: P binds to (X => (iseven X)) ✓
- P is now bound to the predicate function (opaquely)

**Can we inspect P? No.**
**Can we apply P? Yes!**

```logi
check_predicate = P => (P test_value)
// We can apply P to test_value
```

This is exactly what we want!

---

### Example 5: The Problematic Case (From User's Original Question)

```logi
exists (A => (iseven A))    // Value: total function

// Try to match against:
exists (1 => (iseven X))    // Pattern

// Matching functions:
// Pattern function: (1 => (iseven X))
// Value function: (A => (iseven A))

// Try to match:
// Pattern's pattern: 1 (atom)
// Value's pattern: A (variable)
```

**To match pattern `1` against value pattern `A`:**

**Option 1: Two-way unification**
- Bind A=1
- Modify the value function to become (1 => (iseven 1))
- Result: MATCH, but value changed ✗

**Option 2: One-way matching**
- Check if value matches pattern
- Value pattern is `A` (variable), pattern is `1` (atom)
- These have different STRUCTURE (variable vs atom)
- Result: NO MATCH ✓

**One-way matching is correct!** The value says "I'm a total function (pattern variable)", the pattern says "I expect a partial function (atom pattern)". They're incompatible.

---

## Question 3: What If We Allow Structural Matching on Functions?

Let me explore what we could do if we allowed decomposing functions, and see what breaks.

### Scenario: Allow Pattern (A => BODY) to Match Functions

```logi
extract_body = (A => BODY) => BODY

f = X => (square X)
result = extract_body f
// Pattern: (A => BODY)
// Value: (X => (square X))
// A matches X (both variables)
// BODY matches (square X)
// Result: BODY = (square X)
```

**But wait, what about the binding of X?**

The value is `(X => (square X))`. The X in `(square X)` is BOUND by the lambda abstraction.

When we extract BODY = `(square X)`, is X still bound? Or is it now free?

**Interpretation 1: X becomes free**
```logi
result = (square X)    // X is free (undefined)
```

This breaks things! We've created a free variable that was previously bound.

**Interpretation 2: X remains bound somehow**
```logi
result = (square X)    // X is... bound to what?
```

But the binding context is gone. X can't remain bound.

**Interpretation 3: We perform capture-avoiding substitution**

Maybe we rename X to some fresh variable?

```logi
result = (square X')    // X' is a fresh name
```

But this is arbitrary and doesn't preserve semantics.

**Conclusion: Extracting function bodies breaks binding structure.** This is a serious problem!

---

### The α-Equivalence Issue

```logi
f1 = X => (square X)
f2 = Y => (square Y)

// These are α-equivalent (same function)
// But if we extract bodies:
extract_body f1  →  (square X)
extract_body f2  →  (square Y)

// Are these equal? Only up to renaming...
```

If we allow structural decomposition, we break α-equivalence.

---

### The Partial Function Problem (User's Original Example)

```logi
// Total function
total = A => (iseven A)

// Try to match against partial pattern
match_result = total matches (1 => X) ? X : fail
```

If we allow this match:
- Pattern part: 1 (atom)
- Value part: A (variable)
- We'd need to substitute A→1 in the VALUE
- This changes total function to partial function
- **Semantic instability!**

**This is unacceptable.**

---

## Question 4: What Behavior Do We Lose?

If functions are opaque (can only bind with variable patterns, not decompose), what can't we do?

### Lost Capability 1: Extracting Function Parts

```logi
// Can't do:
get_pattern = (p => body) => p        // Extract pattern part
get_body = (p => body) => body        // Extract body part
```

**Workaround:** Don't need this. Functions are abstract computational objects, not data structures.

**If you really need it:** Use quotation to turn functions into data.

```logi
quoted_func = {X => (square X)}
// Now it's data, not a function
// Could define decomposition on quoted terms (carefully)
```

---

### Lost Capability 2: Inspecting Quantified Predicates

```logi
// Can't do:
check_shape = (exists (X => BODY)) => BODY    // Extract predicate body

// Can do:
check_behavior = (exists P) => (P test_val)   // Test predicate by applying it
```

**Workaround:** Use behavioral testing instead of structural inspection.

---

### Lost Capability 3: Distinguishing Total vs Partial by Pattern Matching

```logi
// Can't do:
classify = (A => body) => total
         | (a => body) => partial

f = X => result
classify f  →  can't determine!
```

**Workaround:** You don't need to distinguish them. Just apply the function and see if it reduces.

```logi
test = F => (F test_arg)
// If it reduces, argument matched
// If it doesn't, argument didn't match
```

---

### Lost Capability 4: Function Transformation by Pattern Matching

```logi
// Can't do:
curry = (p => (q => body)) => (pair p q) => body
```

**Workaround:** This kind of metaprogramming should use quotation or explicit combinators.

---

## The Core Insight: Behavior vs Structure

**Functional programming paradigm:** Work with functions BEHAVIORALLY (apply them), not STRUCTURALLY (decompose them).

**Data structures:** Work with them STRUCTURALLY (pattern match), not BEHAVIORALLY.

**Functions are not data structures.**

---

## Argument With Myself: What About Logic Programming?

### Position: "But Prolog lets you inspect terms, including compound terms with variables!"

In Prolog:
```prolog
decompose(f(X, Y), X, Y).
```

This extracts X and Y from f(X, Y). Why can't we do this with functions?

**Counterargument:**

Prolog doesn't have lambda abstractions! Prolog terms are data structures, not functions.

```prolog
f(X, Y)  % This is a term (data), not a function
```

In contrast, Logi functions are computational abstractions:
```logi
X => Y => (f X Y)    % This is a function, not data
```

**If you want to treat functions as data in Logi, use quotation:**
```logi
{X => Y => (f X Y)}    % Now it's data, not a function
```

---

## Argument With Myself: What About Dependent Types?

### Position: "In dependent type theory, you can inspect function types!"

In languages like Agda/Coq:
```agda
inspect : (A → B) → ...
```

You can pattern match on types and do type-level computation.

**Counterargument:**

Logi doesn't have dependent types! We explicitly decided against them (per user's initial requirements).

Type-level computation is different from value-level computation.

Also, even in Agda, you can't actually decompose the *implementation* of a function - you can only inspect its *type*.

---

## Existential Examples: Finding Edge Cases

Let me try to break the system with edge cases...

### Edge Case 1: Self-Application

```logi
self = X => (X X)
self self  →  (self self)  →  ...
```

Does opacity help or hurt here?

**Analysis:**
- Pattern: X (variable)
- Argument: self (function)
- Match: X binds to self (opaquely) ✓
- Body: (X X) becomes (self self)
- Result: infinite loop (as expected)

Opacity doesn't affect this. It's the standard self-application issue.

---

### Edge Case 2: Function Equality

```logi
f = X => (plus X zero)
g = Y => Y

// Are f and g equal?
```

With opaque functions, we can't inspect to determine equality. We can only test extensionally:

```logi
equal_at = F => G => V => (eq (F V) (G V))
equal_at f g five  →  (eq (plus five zero) five)  →  true (if we have reduction)
```

**This is fine.** Extensional equality is the right notion for functions anyway.

---

### Edge Case 3: Higher-Order Pattern Matching

```logi
apply_twice = F => X => (F (F X))

// Can we match this?
is_apply_twice = (F => X => (F (F X))) => true
               | _ => false

is_apply_twice apply_twice  →  ???
```

**With opaque functions:**
- Pattern: (F => X => (F (F X)))
- Value: (F => X => (F (F X)))
- Can they match?

If we allow α-equivalent exact matching:
- Pattern's outer pattern: F (variable)
- Value's outer pattern: F (variable)
- Match structure: both variables ✓
- Pattern's inner function: X => (F (F X))
- Value's inner function: X => (F (F X))
- These are α-equivalent ✓
- Result: MATCH ✓

**But this requires checking α-equivalence, which is complex!**

**Alternative:** Disallow this entirely. Functions only match with variable patterns (opaque binding).

```logi
is_apply_twice = F => true    // Just checks that argument IS a function
```

**I'm inclined toward full opacity.** α-equivalence checking is complex and rarely needed.

---

### Edge Case 4: Partial Application in Patterns

```logi
add = X => Y => (plus X Y)

// Can we pattern match on partially applied functions?
is_add_five = (add five) => true
            | _ => false
```

**Analysis:**
- (add five) is a value (a function waiting for second argument)
- Is (add five) a pattern or a value in the pattern position?

If it's a pattern, we're matching against a specific partial application. But (add five) is a function, which should be opaque...

**This is getting murky.**

**Better interpretation:** (add five) in pattern position is an APPLICATION, which should be evaluated first:
```logi
(add five)  →  (Y => (plus five Y))
```

So the pattern is actually:
```logi
is_add_five = (Y => (plus five Y)) => true
```

If we require exact α-equivalence, this could match. But this is getting very complex.

**Recommendation:** Don't allow function patterns except variable patterns. Applications in patterns should be evaluated.

---

### Edge Case 5: Matching Functions Inside Data

```logi
data = (pair f g)    // where f and g are functions

// Can we match this?
extract = (pair F G) => (F, G)

extract data  →  ???
```

**Analysis:**
- Pattern: (pair F G)
- Value: (pair f g)
- Match pair constructor: ✓
- Match F against f: F binds to function f (opaquely) ✓
- Match G against g: G binds to function g (opaquely) ✓
- Result: (f, g) ✓

**This works!** We're not decomposing the functions, just binding them opaquely.

---

## My Final Position

After arguing with myself extensively, here's my conclusion:

### Rule 1: Functions Can Be Bound Opaquely

**Variable patterns CAN match functions:**
```logi
process = F => (F arg)    ✓
```

Functions are first-class values. Variable patterns bind them opaquely.

### Rule 2: Functions Cannot Be Decomposed

**Structural patterns on functions are NOT allowed:**
```logi
extract = (X => BODY) => BODY    ✗
check = (a => b) => something    ✗
```

Functions are abstract computational objects, not data structures.

### Rule 3: One-Directional Matching in Application

**When applying functions:**
```logi
(a => result) applied to X    →  does NOT reduce (atom pattern vs variable value)
(A => result) applied to x    →  DOES reduce (variable pattern vs atom value)
```

Pattern matching flows one direction: value information flows into pattern variables.

### Rule 4: Matching Checks Structural Compatibility

**When matching functions against function patterns:**

Since we don't allow function patterns except variables, this is moot. But if we DID allow exact matching:

```logi
(A => body) matches (a => body)    ✗  (variable vs atom structure)
(A => body) matches (B => body)    could match if α-equivalent
```

But I recommend against even this. **Full opacity is simpler.**

---

## What We Lose vs What We Gain

### We Lose:
1. Structural inspection of functions
2. Distinguishing total vs partial by pattern matching
3. Extracting function parts
4. Function metaprogramming via pattern matching

### We Gain:
1. **Semantic stability** - functions never change meaning
2. **Simplicity** - clear, simple rules
3. **Soundness** - no unwanted transformations
4. **Consistency** - aligns with lambda calculus tradition
5. **Implementation ease** - no complex α-equivalence checking

### We Can Still Do:
1. Higher-order programming (passing functions)
2. Behavioral testing (applying functions)
3. Quantification (exists/forall)
4. Combinators and function composition
5. Everything important!

---

## Final Recommendation: Full Opacity

**Functions are completely opaque to structural pattern matching.**

**Allowed:**
```logi
// Bind function to variable (opaque)
process = F => (F arg)    ✓

// Apply function
result = f arg    ✓

// Functions in data structures
data = (pair f g)    ✓
extract = (pair F G) => F    ✓  (binds F opaquely)
```

**Not allowed:**
```logi
// Decompose function structure
extract_body = (X => BODY) => BODY    ✗

// Pattern match on function patterns
check = (a => b) => ...    ✗

// Any structural inspection
inspect = f => (is f a function? what's its pattern? what's its body?)    ✗
```

**For metaprogramming needs:**
Use quotation to reify functions as data (outside core language).

---

## The One-Directional Unification Principle

**Precise formulation:**

When matching pattern `p` against value `v`:
- Pattern variables in `p` can be bound to parts of `v`
- Atoms/constructors in `p` must match exactly with `v`
- **`v` is never modified**
- **Information flows: v → pattern variables in p**

**Consequences:**

```logi
// ✓ Variable pattern binds to concrete value
(A => result) applied to five    // A binds to five → result

// ✗ Atom pattern doesn't match variable value  
(five => result) applied to X    // Would need to modify pattern to X - not allowed

// ✓ Variable pattern binds to variable value
(A => result) applied to X    // A binds to X → result (with X still a variable)

// ✗ Atom pattern doesn't match different atom
(five => result) applied to six    // five ≠ six - doesn't match

// ✓ Atom pattern matches same atom
(five => result) applied to five    // five = five → result
```

**This is clean, consistent, and enforceable!**

---

End of Deep Dive.
