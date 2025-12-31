# The Quotation Problem Revisited

## The Issue

**User's observation:** When writing `(apply not true)`, if `not` is defined, it might reduce in place!

Let me trace through what happens:

```logi
// Definitions
not = true => false | false => true
apply = F => X => (apply F X)

// Usage
statement = apply not true
```

**Evaluation:**
1. `apply` is a function (value)
2. `not` is a function (value)  
3. `true` is an atom (value)
4. Apply `apply` to `not`: `(X => (apply not X))`
5. Apply to `true`: `(apply not true)`

**Result:** `(apply not true)`

**But wait - what IS `not` in this term?**

---

## The Critical Question

In the term `(apply not true)`:
- `apply` in application position is an atom (not the function definition)
- But what about `not` in the second position?
- And could `not true` reduce?

**Two possibilities:**

### Possibility 1: Names Are Always Looked Up

**If the evaluator eagerly looks up names:**

`(apply not true)`
→ Look up `not`: `(true => false | false => true)`
→ Term becomes: `(apply (true => false | false => true) true)`
→ But then we have a function sitting there as data... that's odd

Or worse:
→ Reduce `not true` → `false`
→ Term becomes: `(apply false)`
→ **WRONG! We've lost information**

### Possibility 2: Terms Inside Data Don't Reduce

**If the evaluator recognizes that `(apply ...)` is data (because `apply` in head position is an atom):**

`(apply not true)` is recognized as a value (stuck term with atom head)
→ Don't try to reduce subterms
→ Keep as `(apply not true)` where `not` and `true` are just names/atoms
→ **This could work**

---

## The Problem: It Depends on Evaluation Strategy

### Issue 1: Are Names Values or Reducible?

**If `not` is a name that refers to a definition:**
- Should it be automatically substituted with its definition everywhere?
- Or only when it's in "function position" (head of an application)?

**Example:**
```logi
not = true => false | false => true

term1 = not          // Is this the function, or the atom "not"?
term2 = not true     // This should reduce to false
term3 = (pair not true)   // Should 'not' be substituted here?
```

**In most languages:**
- Names in value position denote their definitions
- `not` by itself is the function value
- `(pair not true)` contains the function and the atom

**So in `(apply not true)`:**
- `not` would be the function value `(true => false | false => true)`
- We'd have `(apply <function> true)` as data

### Issue 2: Functions as Data

**If functions can appear in data positions:**
```logi
(apply not true)
```

Here, `not` is the function value, and we're building data that CONTAINS a function.

**This should be fine!** Functions are first-class values.

**BUT:** The user might want to represent the NAME "not" symbolically, not the actual function value.

---

## What Does The User Actually Want?

**I think the user wants:**
```logi
statement = (apply not true)
```

To represent the **symbolic expression** "apply not to true", NOT:
- The actual function `not` applied to `true` (which would be `false`)
- A data structure containing the function value and the atom

**In other words: reified syntax, not semantic values.**

---

## Solutions

### Solution 1: Keep Quotation

**With quotation:**
```logi
statement = (apply {not} {true})
```

Or:
```logi
statement = {apply not true}    // Quote the whole thing
```

**This explicitly says:** "Don't reduce, this is syntax."

**Pros:** 
- Clear intent
- Can distinguish `not` (the function) from `{not}` (the symbol)

**Cons:**
- Adds complexity
- Need to quote everything you want symbolic

---

### Solution 2: Distinguish Symbols from Definitions

**Idea:** Have a way to refer to symbols without triggering lookup.

**Syntax:**
```logi
// Backtick for symbolic reference?
statement = (apply `not `true)

// Or apostrophe?
statement = (apply 'not 'true)
```

**Pros:**
- Clear distinction
- Can mix symbolic and semantic

**Cons:**
- Still adds syntax
- Similar to quotation

---

### Solution 3: Data Constructors Use Different Namespace

**Idea:** Data constructor atoms are distinct from function names.

```logi
// Function definition
not = true => false | false => true

// Data constructor (capitalized?)
Apply = F => X => (Apply F X)

// Usage
statement = Apply Not True    // Not, True are data symbols, not the functions
```

**Naming convention:**
- Lowercase: computational (functions, reducing atoms)
- Uppercase: data (constructors, non-reducing)

**Pros:**
- No new syntax
- Clear convention
- Common in ML/Haskell

**Cons:**
- Requires discipline
- Need both `not` and `Not` for function vs symbol

---

### Solution 4: Context-Sensitive Reduction

**Idea:** Only reduce in "reducing contexts", not in "data contexts".

**Rule:** If the head of an application is an atom (not a defined function), the entire application is data and subterms don't reduce.

```logi
apply = F => X => (apply F X)

statement = apply not true
// Reduces to: (apply not true)
// Since 'apply' in head position is an atom (not defined as a reducing function there),
// the term is stuck and 'not' inside doesn't reduce
```

**Wait, but `apply` IS defined!**

Let me reconsider...

After beta-reduction:
```logi
statement = apply not true
→ (apply not true)    // Result of substitution
```

In the result `(apply not true)`, the head is the ATOM `apply` (from the body of the function), not the function definition `apply = ...`.

**So the term is stuck!** It won't reduce further, and neither will its subterms.

**This might actually work!**

---

## Let Me Trace Through More Carefully

### Setup
```logi
not = true => false | false => true
apply = F => X => (apply F X)
```

### Evaluation of `apply not true`

**Step 1:** Evaluate head `apply`
- `apply` is defined as a function
- Result: function value `(F => X => (apply F X))`

**Step 2:** Evaluate first argument `not`
- `not` is defined as a function
- Result: function value `(true => false | false => true)`

**Step 3:** Evaluate second argument `true`
- `true` is an atom (no definition)
- Result: `true`

**Step 4:** Apply function to first argument
```logi
(F => X => (apply F X)) <not-function>
→ (X => (apply <not-function> X))
```

In the body, `apply` is just an atom (not the function definition), and `<not-function>` is the bound value.

**Step 5:** Apply to second argument
```logi
(X => (apply <not-function> X)) true
→ (apply <not-function> true)
```

**Result:** `(apply <not-function> true)` where `<not-function>` is the function value.

---

## The Key Issue

**In the result `(apply <not-function> true)`:**
- `apply` is an atom in head position
- `<not-function>` is a function VALUE
- `true` is an atom

**The term is stuck (can't reduce) because the head is an atom, not a defined function.**

**BUT:** The function value is "baked in" to the data structure.

**Is this what the user wants?**

---

## Two Different Use Cases

### Use Case 1: Reify The Computation

**Want:** Represent "apply not to true" with the actual function objects

```logi
term = (apply <not-function> true)
```

**This is what atom constructors give us!**

Later, can pattern match and extract:
```logi
extract = (apply F X) => (F, X)
(f, x) = extract term
// f is the not function, x is true
// Can then apply: f x → false
```

### Use Case 2: Reify The Syntax

**Want:** Represent "apply not to true" as symbolic expression, without semantic content

```logi
term = (apply 'not 'true)
```

Where `'not` is a SYMBOL, not the function.

**This requires quotation or similar!**

---

## Which Does The User Need?

**Re-reading the user's example:**

> "suppose we want to say something propositionally like: 'f applied to true is false', I could just express the application in a way that doesn't reduce (apply f true)"

**I think the user wants symbolic representation!**

Not the function value, but the NAME of the function as a symbol.

**So they might actually need quotation after all.**

---

## The Fundamental Tension

**Without quotation:**
- Names denote their definitions
- `not` means the function, not the symbol "not"
- Can't distinguish symbolic `not` from semantic `not`

**With quotation:**
- `not` means the function
- `'not` or `{not}` means the symbol
- Clear distinction

---

## Possible Compromise: Quotation Only for Names

**Minimal quotation:** Just for preventing name lookup

**Syntax:**
```logi
// Backtick for symbolic names?
statement = (apply `not `true)

// Or quote operator?
statement = (apply (quote not) (quote true))
```

**Semantics:**
- `not` → look up definition (the function)
- `(quote not)` → the symbol "not" (doesn't look up definition)

**This is lighter than full quotation of terms!**

---

## Or: Use Atoms More Carefully

**Idea:** Use undefined atoms for symbolic representation

```logi
// Don't define 'not_symbol' anywhere
statement = (apply not_symbol true)
```

Since `not_symbol` has no definition, it remains an atom.

**But this is fragile!** If you accidentally define it later, things break.

---

## My Recommendation

**After this analysis, I think we DO need quotation, but a minimal form.**

### Option A: Name Quotation

**Just quote names to prevent lookup:**
```logi
statement = (apply 'not 'true)
```

**Semantics:**
- `'not` is the atom/symbol "not" (doesn't look up definition)
- `not` is the function value (looks up definition)

### Option B: Term Quotation (Original)

**Quote entire terms:**
```logi
statement = {apply not true}
```

**Semantics:**
- `{M}` freezes the term, preventing any reduction
- Substitution still penetrates

### Option C: Both

**Combine them:**
```logi
// Name quotation for symbols
sym = 'not

// Term quotation for complex expressions
expr = {plus two three}
```

---

## The User's Example Revisited

**User:** "I could just express the application in a way that doesn't reduce (apply f true)"

**Issue:** If `f` is defined, it WILL be looked up and substituted!

**Solution:** Quote it!
```logi
statement = (apply 'f 'true)    // Symbolic
```

Or:
```logi
statement = {apply f true}    // Quoted term
```

**Without quotation:** Can't distinguish symbolic reference from semantic reference.

---

## Conclusion

**The user found a real problem!**

**Without quotation:**
- Can't represent symbolic expressions distinctly from their semantics
- Names always denote their definitions
- Can build data containing function VALUES, but not symbolic references to functions

**We need SOME form of quotation to:**
1. Prevent name lookup when desired
2. Distinguish syntax from semantics
3. Support meta-programming and symbolic reasoning

**Minimal solution:** Name quotation (quote individual names)

**More powerful solution:** Term quotation (quote entire expressions)

**I think term quotation `{M}` is the right choice:**
- Simple syntax
- Clear semantics
- Sufficient for all use cases

---

**Verdict: We need quotation after all!**

The earlier analysis was incomplete. You can't distinguish symbolic references from semantic ones without SOME form of quotation.

End of Document.
