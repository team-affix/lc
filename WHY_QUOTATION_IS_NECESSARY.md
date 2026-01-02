# Why Quotation IS Necessary

## The Problem

```logi
not = true => false | false => true
apply = F => X => (apply F X)

statement = apply not true
```

**What happens?**

### Step-by-step evaluation:
1. `apply not true`
2. Beta-reduce: `(X => (apply not X)) true`
3. Beta-reduce: `(apply not true)`

**Now we have the term: `(apply not true)`**

**But `not` is a defined name!**

---

## The Critical Issue

In the term `(apply not true)`:
- `apply` in the head position is just an atom (from the function body)
- But `not` in the second position... is it:
  - **A)** The atom/symbol "not" 
  - **B)** A reference to the defined function `not = ...`

**In most languages, it's B!** Names denote their definitions.

---

## What Happens Next?

### If names are eagerly looked up:

`(apply not true)`

When the evaluator sees `not`, it looks up the definition:

`(apply (true => false | false => true) true)`

Now we have a **function value** embedded in the data, not a symbolic reference!

### Even worse - if the evaluator tries to reduce inside:

`(apply not true)`
→ See `not true` sitting there
→ Reduce it: `not true` → `false`  
→ Result: `(apply false)`

**We've lost the structure!** We wanted to represent "apply not to true", not "apply false".

---

## The Fundamental Problem

**Without quotation, you cannot distinguish:**
- **Symbolic reference:** The name "not" as data
- **Semantic reference:** The function that `not` denotes

**Example:**
```logi
not = true => false | false => true

symbolic = ???    // Want: the symbol "not" (not the function)
semantic = not    // The actual function
```

**How do you express `symbolic` without quotation?**

You can't! Any reference to `not` will look up its definition.

---

## What Atom Constructors Can't Do

**Atom constructors work fine when:**
```logi
apply = F => X => (apply F X)
result = apply not true    // → (apply <function-value> true)
```

**But this gives you:**
- The function VALUE embedded in data
- NOT the symbolic name "not"

**If you later pattern match:**
```logi
extract = (apply F X) => F
f = extract result
// f is the FUNCTION (true => false | false => true)
// NOT the symbol "not"
```

**You've lost the symbolic information!**

---

## Use Cases That Need Quotation

### 1. Symbolic Computation

**Want:** "The expression (plus x y), not evaluated"

**Without quotation:**
```logi
expr = (plus x y)
```

If `plus`, `x`, `y` are defined, they'll be looked up/substituted!

**With quotation:**
```logi
expr = {plus x y}    // Frozen, symbolic
```

### 2. Meta-Programming

**Want:** "The function definition (X => (square X)) as data"

**Without quotation:**
```logi
code = X => (square X)    // This is a function, not data!
```

**With quotation:**
```logi
code = {X => (square X)}    // This is data representing a function
```

### 3. Proof Statements

**Want:** "f applied to true equals false"

**Without quotation:**
```logi
not = true => false | false => true
statement = eq (apply not true) false
```

But `not` here denotes the function value, not the name!

**With quotation:**
```logi
statement = eq {apply not true} false
```

Or even:
```logi
statement = eq (apply {not} true) false
```

---

## The Two Namespaces Problem

**One might suggest:** "Use different names for data vs functions"

```logi
// Function
not = true => false | false => true

// Symbol (undefined atom)
not_symbol = not_symbol

// Usage
statement = (apply not_symbol true)
```

**Problems:**
1. Verbose - need two names for everything
2. Fragile - if you define `not_symbol`, it breaks
3. Confusing - which name to use when?

**Quotation solves this cleanly:**
```logi
not = true => false | false => true    // Function
{not}    // Symbol
```

---

## Minimal Quotation Syntax

**Proposal:** Keep `{M}` quotation

**Semantics:**
1. `{M}` doesn't reduce (frozen)
2. Substitution penetrates: `{M}[x := v]` = `{M[x := v]}`
3. Names inside `{M}` are NOT looked up

**Example:**
```logi
not = true => false | false => true

unquoted = not             // The function value
quoted = {not}             // The symbol "not"

expr1 = not true           // Reduces to false
expr2 = {not true}         // Frozen expression (not true)
```

---

## Alternative: Name-Only Quotation

**Lighter syntax:** Only quote names, not full terms

```logi
statement = (apply 'not 'true)
```

**Semantics:**
- `'name` prevents lookup of `name`
- Rest of term evaluates normally

**Pro:** Lighter weight
**Con:** Can't freeze complex expressions

---

## My Final Recommendation

**Use full term quotation `{M}`:**

**Reasons:**
1. **Necessary:** Can't distinguish symbolic from semantic without it
2. **Sufficient:** Handles all meta-programming needs
3. **Simple:** One construct for all quotation needs
4. **Clear:** Obvious when something is frozen vs reducible

**Examples of necessity:**

```logi
// Symbolic expression
expr = {plus two three}    // NEED THIS - can't express otherwise

// Function as data
code = {X => (square X)}   // NEED THIS - otherwise it's a function, not data

// Proof statement  
claim = {f x equals y}     // NEED THIS - otherwise names get looked up
```

---

## Updating The Language

**Restore quotation as primitive:**

**Syntax:**
```
term ::= ... | {M}
```

**Values:**
```
value ::= ... | {M}
```

**Substitution (penetrating):**
```
{M}[x := v] = {M[x := v]}
```

**Reduction:**
```
{M} is a value (doesn't reduce)
```

**Pattern matching:**
```
match({M}, {p}) = match(M, p)
```

---

## The Corrected Story

**Previously I claimed:** "Atom constructors are sufficient for quotation"

**Reality:** Atom constructors can build non-reducing STRUCTURES, but they can't prevent NAME LOOKUP.

**Key distinction:**
- **Atom constructor:** `(apply f x)` where `apply` is an atom - STRUCTURE doesn't reduce
- **But:** `f` and `x` still denote their definitions (if any)
- **Quotation:** `{apply f x}` - NAMES inside aren't looked up

**Both are needed:**
- Atom constructors: for building data structures
- Quotation: for preventing name lookup / freezing expressions

---

## Concrete Example

```logi
// Function definitions
not = true => false | false => true
id = X => X

// Atom constructor approach
apply = F => X => (apply F X)
term1 = apply not true
// Result: (apply <not-function> true)
// 'not' was looked up, function value embedded

// Quotation approach  
term2 = {apply not true}
// Result: {apply not true}
// 'not' was NOT looked up, remains symbolic

// Can we pattern match on term2 to extract the name?
extract = {apply F X} => F
name = extract term2
// name = {not} (the symbol, not the function!)
```

**See the difference?**

---

## Summary

**The user is correct!** We need quotation because:

1. **Names denote definitions** - can't prevent lookup without quotation
2. **Symbolic vs semantic** - need to distinguish the symbol "not" from the function `not`
3. **Meta-programming** - need to freeze expressions and inspect syntax
4. **Proof statements** - need to talk about expressions without evaluating them

**Atom constructors are NOT sufficient** - they prevent structural reduction but not name lookup.

**Solution:** Keep `{M}` quotation as a primitive.

---

**I was wrong earlier. Quotation IS necessary!**

End of Document.
