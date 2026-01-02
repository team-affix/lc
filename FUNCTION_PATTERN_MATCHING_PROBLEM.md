# The Function Pattern Matching Problem

## The Issue

You've identified a critical problem with pattern matching on functions.

### Example 1: Quantified Predicates

**Value:**
```logi
exists (A => (iseven A))
```
This is a **total function** - it means "∃x. iseven(x)" (exists some number that is even).

**Pattern:**
```logi
exists (1 => (iseven X))
```

**If we naively pattern match:**

Matching `(A => (iseven A))` against pattern `(1 => (iseven X))`:
1. Match pattern part `1` against value pattern part `A` → substitute A=1
2. Match pattern body `(iseven X)` against value body `(iseven A)` → after A=1 substitution, this becomes `(iseven 1)` → X=1
3. Result: The function becomes `(1 => (iseven 1))`

**This is WRONG!**

We've transformed:
- **Before:** A total function (matches any argument)
- **After:** A partial function (matches only 1)

The **meaning** changed:
- **Before:** "∃x. iseven(x)" - "some number is even" 
- **After:** "∃x=1. iseven(1)" - "1 is even"

**These are NOT equivalent!**

### Example 2: Regular Functions

**Value:**
```logi
A => (square A)
```
Total function: squares any argument.

**Pattern:**
```logi
a => X
```

**Naive matching gives:**
- A → a (substitute A with a)
- (square A) → X, which after substitution is (square a)
- Result: `a => (square a)`

**Transformed:**
- **Before:** Total function (accepts anything)
- **After:** Partial function (accepts only atom `a`)

**Again, semantic change!**

---

## Why This is Catastrophic

### 1. **Semantic Instability**

Pattern matching should be a **query operation** - it should inspect values without modifying them.

But here, matching is CHANGING the value's behavior!

### 2. **Type Instability**

If we had types:
- Before: `∀α. α → α` (polymorphic, total)
- After: `{a} → α` (partial, singleton domain)

The "type" changed!

### 3. **Logical Unsoundness**

```logi
// Start with valid theorem
theorem (exists (A => (iseven A)))    // "Some number is even" - TRUE

// Pattern match against (exists (1 => (iseven X)))
// This produces: exists (1 => (iseven 1))

// Which means: "1 is even" - FALSE
```

**We've turned a true statement into a false one!**

### 4. **Breaks Substitution Principle**

If `f = (A => (iseven A))` and we match f, we shouldn't be able to change what f means.

Functions should have **referential transparency**.

---

## Root Cause Analysis

### What Is Pattern Matching?

Pattern matching traditionally:
1. **Tests** if a value has a certain structure
2. **Extracts** subcomponents and binds pattern variables
3. **Does NOT modify** the value

### What Went Wrong?

When matching functions, we're trying to unify:
- Pattern: `(p1 => M1)` 
- Value: `(p2 => M2)`

Naively, we'd say:
- Match p1 against p2
- Match M1 against M2
- Apply resulting substitutions

**But this applies substitutions to the VALUE, changing it!**

### The Core Problem

**Pattern variables in the pattern part of a function pattern should NOT cause substitution in the value.**

When we match pattern `(1 => ...)` against value `(A => ...)`:
- Pattern says "I expect an atom 1 here"
- Value says "I have a variable A here"
- These are **structurally incompatible**

**They should NOT match at all!**

---

## Possible Solutions

Let me explore several approaches and argue with myself about each.

### Option 1: Functions Are Completely Opaque

**Proposal:** You cannot pattern match on function structure at all.

**Allowed:**
```logi
process = F => (apply F five)    // F binds to any function (opaque)
```

**Not allowed:**
```logi
decompose = (X => BODY) => BODY    // Can't decompose functions
```

**Pros:**
- Simple and safe
- Functions remain black boxes
- No semantic issues
- Aligns with most functional languages (Haskell, ML, etc.)

**Cons:**
- Very restrictive
- Can't inspect quantified predicates
- Can't do meta-programming on functions
- Loses expressiveness

**Can we work around it?**
- Use quotation `{A => M}` to make functions inspectable
- But even then, quoted functions are still data - should we allow decomposing them?

**Verdict:** Safe but limiting. Let me explore alternatives.

---

### Option 2: Structural Compatibility Check

**Proposal:** Functions match only if their pattern parts have the SAME STRUCTURE.

**Matching rules:**
- `(A => M)` matches pattern `(B => N)` → YES (both have variable patterns)
- `(1 => M)` matches pattern `(1 => N)` → YES (both have atom pattern 1)
- `(1 => M)` matches pattern `(A => N)` → NO (atom ≠ variable)
- `(A => M)` matches pattern `(1 => N)` → NO (variable ≠ atom)

**Pros:**
- Preserves total/partial distinction
- Prevents unwanted substitution
- Still allows some pattern matching on functions
- Philosophically sound: respects function structure

**Cons:**
- More complex semantics
- Need to define "structural compatibility" precisely
- What about multi-case functions?

**Can we work around edge cases?**

**Case 1:** Variable pattern in pattern position
```logi
f = (X => M)  // pattern
(A => N)      // value
```
Should X bind to A? But that's problematic - we're binding a pattern variable to another pattern variable.

**Better interpretation:** X is a META-pattern-variable that can match any pattern structure.

```logi
extract_pattern = (X => BODY) => X
f = (1 => five)
result = extract_pattern f    // X should bind to the pattern `1`, not cause substitution
```

This is getting complex...

**Verdict:** Promising but needs careful specification. Let me think more.

---

### Option 3: No Pattern Matching on Binders

**Proposal:** The pattern part of a function is a **binder context** and cannot be matched against.

You can only match:
1. That something IS a function (using variable pattern F)
2. The body of the function (but pattern variables in the function's pattern stay abstract)

**Example:**
```logi
// Pattern
(A => (iseven A))

// Match against
(??? => BODY)
```

We can bind BODY to `(iseven A)`, but we CANNOT inspect or constrain the `???` part.

**Pros:**
- Respects the binding structure
- Prevents substitution into binders
- Philosophically aligned with lambda calculus

**Cons:**
- Can't distinguish total vs partial functions by pattern matching
- Limited usefulness
- Complex to specify

**Verdict:** Interesting but perhaps too restrictive.

---

### Option 4: α-Equivalence Only

**Proposal:** Functions match only if they're α-equivalent (same up to variable renaming).

**Matching rules:**
- `(A => (square A))` matches pattern `(B => (square B))` → YES (α-equivalent)
- `(A => (square A))` matches pattern `(X => X)` → NO (different body structure)
- `(1 => M)` matches pattern `(1 => N)` → YES if M matches N
- `(1 => M)` matches pattern `(A => N)` → NO (not α-equivalent)

**Pros:**
- Very restrictive but safe
- Clear semantics
- Preserves function equality

**Cons:**
- Extremely limited - can only match exact functions
- Not very useful

**Verdict:** Safe but too restrictive to be useful.

---

### Option 5: Quotation Required

**Proposal:** Functions can only be pattern matched if they're quoted.

**Unquoted functions:**
```logi
A => (square A)    // Opaque, can't pattern match internals
```

**Quoted functions:**
```logi
{A => (square A)}    // Frozen, can be inspected
```

**Rules:**
- Unquoted functions: only match with variable patterns (bind opaquely)
- Quoted functions: can pattern match structure, but must respect binding

**Pros:**
- Explicit control over inspectability
- Functions are opaque by default
- Quotation signals intent

**Cons:**
- Adds complexity
- Users must remember to quote
- Still need to solve the substitution problem for quoted functions

**Verdict:** Good principle, but doesn't fully solve the problem.

---

### Option 6: Separate Pattern Variables from Value Variables

**Proposal:** Pattern variables in the pattern part of a function pattern are DIFFERENT from variables in values.

When matching `(1 => M)` against `(A => N)`:
- `1` is a pattern (concrete atom)
- `A` is a value (variable binding)
- These are in different "categories" and don't unify

**Think of it like this:**
- Patterns: describe WHAT to match
- Values: actual computational objects

A pattern `(1 => ...)` says "I expect a partial function that matches exactly 1"
A value `(A => ...)` says "I am a total function that binds any argument to A"

These should NOT unify because they're fundamentally different structures.

**Matching rules:**
- Pattern `(pat1 => body_pat)` matches value `(val_pat => val_body)` if:
  - `pat1` and `val_pat` have the same structure (both atoms/same atom, or both variables)
  - `body_pat` matches `val_body`

**Crucially:** Pattern variables in `body_pat` can bind to subterms of `val_body`, but pattern variables in `pat1` do NOT cause substitution in `val_pat`.

**Example:**
```logi
// Pattern: (1 => (iseven X))
// Value: (A => (iseven A))

// Check: does pattern part `1` match value pattern part `A`?
// 1 is an atom, A is a variable → DIFFERENT STRUCTURES → NO MATCH
```

**Example 2:**
```logi
// Pattern: (A => X)  (A is a pattern variable)
// Value: (B => (square B))

// Check: does pattern part `A` match value pattern part `B`?
// Both are variables → SAME STRUCTURE → MATCH
// Now match body: X matches (square B) → X = (square B)
// But we DON'T substitute into the value!
// Instead, we just recognize that IF the value's argument is called B,
// then X binds to (square B)
```

Wait, this is getting complicated too...

**Verdict:** On the right track but needs more thought.

---

## My Recommendation: Option 2 (Refined)

After thinking through all options, I believe **structural compatibility with careful semantics** is the right approach.

### Core Principle

**Functions match only if their pattern structures are compatible, and matching does NOT modify the value.**

### Precise Matching Rules

**Rule 1: Variable Pattern (Opaque Binding)**
```logi
Pattern: F
Value: any function
Result: F binds to the entire function (opaque)
```

**Rule 2: Atom Pattern Compatibility**
```logi
Pattern: (a => M)
Value: (a => N)
Result: Match if M matches N
```

```logi
Pattern: (a => M)
Value: (b => N)  where a ≠ b
Result: NO MATCH (different atoms)
```

```logi
Pattern: (a => M)
Value: (A => N)  where A is a variable
Result: NO MATCH (atom ≠ variable structure)
```

**Rule 3: Variable Pattern Compatibility**
```logi
Pattern: (X => M)  where X is a pattern variable
Value: (A => N)    where A is a variable
Result: Match if M matches N (with variable correspondence)
```

**Crucially:** When matching bodies, we respect the correspondence between pattern variable X and value variable A.

If pattern body M contains X, and value body N contains A, we check if M[X→A] matches N.

```logi
Pattern: (X => (square X))
Value: (A => (square A))
Result: MATCH (α-equivalent)
```

```logi
Pattern: (X => X)
Value: (A => (square A))
Result: NO MATCH (bodies don't match)
```

**Rule 4: Multi-Case Functions**
```logi
Pattern: (p1 => M1) | (p2 => M2)
Value: (v1 => N1) | (v2 => N2)
Result: Match if p1↔v1 compatible, p2↔v2 compatible, M1↔N1 match, M2↔N2 match
```

### Key Insight

**Pattern matching on functions checks STRUCTURAL COMPATIBILITY without performing substitution into the value's binders.**

### Testing on User's Examples

**Example 1:**
```logi
Pattern: exists (1 => (iseven X))
Value: exists (A => (iseven A))
```

Match inner function `(1 => (iseven X))` against `(A => (iseven A))`:
- Pattern part: `1` (atom)
- Value part: `A` (variable)
- **Different structures → NO MATCH** ✓

**Example 2:**
```logi
Pattern: a => X
Value: A => (square A)
```

Match `(a => X)` against `(A => (square A))`:
- Pattern part: `a` (atom)
- Value part: `A` (variable)
- **Different structures → NO MATCH** ✓

**Example 3: What DOES match?**
```logi
Pattern: A => X
Value: B => (square B)
```

Match `(A => X)` against `(B => (square B))`:
- Pattern part: `A` (variable)
- Value part: `B` (variable)
- **Same structure** ✓
- Now match bodies: `X` against `(square B)`
- **But wait** - we need to respect variable correspondence!

Actually, in pattern matching, pattern variable `A` in the pattern part is saying "match any variable binding here". So we're checking that the value has the form "variable binding => body".

Then X matches against `(square B)`, so X = `(square B)` with the understanding that B is the bound variable.

Hmm, this is subtle. Let me reconsider...

Actually, I think the pattern `(A => X)` should be interpreted as:
- "Match any function with a variable pattern (not atom pattern)"
- "Bind A to the name of that variable" (or just ignore it?)
- "Bind X to the body"

So:
```logi
Pattern: (V => X)  // V is a meta-variable for "any variable name"
Value: (B => (square B))

Result: X = (square B) (where B is the bound variable from the value)
```

But this doesn't give us access to manipulate B...

**This is getting complex.** Let me reconsider with a cleaner approach.

---

## Cleaner Approach: Functions Are Mostly Opaque

### The Simple Rule

**You cannot pattern match on function structure, except:**

1. **Variable pattern:** Matches any function, binds opaquely
   ```logi
   F => (something with F)
   ```

2. **Exact α-equivalent match:** Only matches if function is exactly the same (up to renaming)
   ```logi
   (A => (square A)) matches pattern (X => (square X))  // YES
   (A => (square A)) matches pattern (X => X)           // NO
   (1 => M) matches pattern (1 => N)                    // Only if M=N
   ```

3. **That's it.** No partial matching, no mixing total/partial, no substitution into binders.

### Why This Works

- **Safe:** No unwanted substitution
- **Simple:** Easy to specify and understand
- **Sound:** Preserves function semantics
- **Practical:** You can still match exact functions or use opaque variables

### What You CAN'T Do

- Match a total function against a partial function pattern (or vice versa)
- Extract parts of a function arbitrarily
- Inspect function internals unless you match exactly

### Workarounds

**If you need to inspect functions:**
1. Use quotation to make them data
2. Pass components separately
3. Use auxiliary functions to extract behavior

**Example:**
```logi
// Instead of pattern matching on (A => M) to extract M,
// apply the function and observe results:
test_function = F => (F test_input)
```

---

## Final Recommendation

**Functions should be largely opaque to pattern matching.**

**Allowed:**
- Match with variable pattern (opaque): `F => ...`
- Match exact α-equivalent functions: `(A => (square A))` matches `(X => (square X))`

**Not allowed:**
- Match `(A => M)` against `(1 => N)` (different structures)
- Match `(1 => M)` against `(A => N)` (different structures)  
- Extract parts of functions arbitrarily

**Rationale:**
1. **Preserves semantics** - Functions keep their meaning
2. **Prevents unsoundness** - No unwanted substitution
3. **Simple to specify** - Clear rules
4. **Philosophically sound** - Functions are abstract objects
5. **Practical** - Still allows useful patterns (opaque binding, exact matching)

### Impact on User's Code

**This prevents the problematic examples:**
```logi
exists (A => (iseven A))    // Total function
// Does NOT match pattern:
exists (1 => (iseven X))    // Partial function pattern
```

**Good! Pattern match fails, no unwanted transformation.**

**To work with quantified predicates:**
```logi
// Opaque binding works fine:
process_exists = (exists P) => (test_with P)

// Can apply P, just can't decompose it:
test_with = P => (P some_value)
```

---

## Can This Be Worked Around Correctly?

**Yes!** The workaround is to use the function's BEHAVIOR rather than its STRUCTURE.

**Example: Testing if an existential is about a specific property**

Instead of:
```logi
is_about_iseven = (exists (A => (iseven A))) => true |
                  (exists _) => false
// Doesn't work - can't pattern match function internals
```

Use:
```logi
// Test the predicate behaviorally
test_existential = (exists P) => (check_property P)

check_property = P => (
    // Apply P to test values and see if results suggest it's about iseven
    // This is behavioral testing, not structural inspection
)
```

Or, if you really need structural inspection, use quotation or meta-programming primitives (outside the scope of core Logi).

---

## Philosophical Note

**This aligns with the lambda calculus tradition:**

In pure lambda calculus, you can't inspect the body of a lambda - you can only apply it.

λx. M is an opaque abstraction. You can β-reduce by application, but you can't pattern match to extract M.

**Logi is a pattern matching calculus, but functions should still respect this abstraction boundary.**

---

## Summary

### The Problem

Pattern matching on function structure causes unwanted substitution into binders, changing function semantics (total → partial).

### The Solution

**Functions are opaque to pattern matching except:**
1. Variable patterns (bind whole function)
2. Exact α-equivalent matches

**This prevents:**
- Semantic change (total → partial)
- Unwanted substitution
- Logical unsoundness

**This requires:**
- Users work with functions behaviorally (via application)
- Not structurally (via decomposition)

**This aligns with:**
- Lambda calculus tradition
- Functional programming languages
- Philosophical principles of abstraction

### Implementation

Update pattern matching rules to check structural compatibility:
- Variable patterns: always match functions
- Function patterns: only match α-equivalent functions
- No mixing of total/partial structures

---

**This is the right solution. It's principled, safe, and workable.**

End of Analysis.
