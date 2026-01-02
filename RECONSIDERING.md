# Reconsidering the Lambda-Pattern Equivalence

## ⚠️ HISTORICAL DOCUMENT - OUTDATED SYNTAX

**This document is kept for historical reference to understand the design evolution.**

**This document shows REJECTED syntax that was explored and discarded.**

**For current syntax, see:**
- [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md)
- [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)

---

# Original Document (Preserved Below)

## The User's Critical Observation

**My claim:**
```
λx. M  =  { | x ⇒ M }
```

**The problem:**
- `λx. M` - takes ANY argument, binds it to name `x`, returns `M[x]`
- `{ | x ⇒ M }` - returns `M` only if supplied something that is EXACTLY `x`

**These are NOT the same unless `x` has special meaning in patterns!**

---

## Challenging My Own Belief

### My Original Reasoning (Now Questioned)

I assumed pattern variables work like ML/Haskell:
- In pattern position, `x` is a pattern variable (binds to anything)
- In term position, `x` refers to a bound variable
- Context determines interpretation

**Example I had in mind:**
```
f = { | x ⇒ x }    // pattern variable x binds, term variable x refers
f T → T             // x bound to T, return T
```

### The User's Counterargument

If `x` is an identifier (atom/variable), then:
```
{ | x ⇒ M }    // "match exactly x"
```

means: only accept argument that equals `x`, not "bind anything to x".

**This interpretation:**
```
let x = T
f = { | x ⇒ F }
f T   → F    // matches!
f F   → stuck  // doesn't match
```

Here `x` in the pattern means "the value T", not "bind anything".

---

## The Fundamental Question

**Are pattern variables distinct from value variables?**

### Option A: Same Namespace, Context-Dependent (My Original Proposal)

**Convention:**
- Pattern position: `x` is pattern variable (binds)
- Term position: `x` is value variable (refers)

**Example:**
```
f = λy. { | x ⇒ (x, y) }

f T
= { | x ⇒ (x, T) } T     // y→T substituted
= (T, T)                  // x bound by pattern
```

**Problem:** What if `x` is already bound?
```
outer = λx. { | x ⇒ M }
```

Does inner `x` in pattern:
- Shadow outer `x` (bind new variable)?
- Match value of outer `x`?

**In ML/Haskell:** It shadows (pattern variables always bind).

**User's concern:** This is confusing! Hard to tell what's binding vs. matching.

---

### Option B: Explicit Binding Syntax

Use different syntax for "bind" vs. "match":

**Strawman syntax:**
```
{ | ?x ⇒ M }    // ?x is pattern variable (binds anything)
{ | x ⇒ M }     // x is atom/value (must match exactly)
{ | T ⇒ M }     // T is atom (must match exactly)
```

**Then:**
```
λx. M  =  { | ?x ⇒ M }    // NOW they're equivalent!
```

**Example:**
```
not = { | T ⇒ F | F ⇒ T }        // match atoms T and F
id = { | ?x ⇒ x }                 // bind anything to x

parameterized = λy. { | y ⇒ F | ?z ⇒ z }
// matches exactly the value y, else binds to z
```

**Advantage:** Explicit, no confusion
**Disadvantage:** Extra syntax, more complex

---

### Option C: Uppercase/Lowercase Convention

**Convention (like ML/Haskell):**
- Uppercase: atoms/constructors (match exactly)
- Lowercase: pattern variables (bind)

**Then:**
```
not = { | T ⇒ F | F ⇒ T }    // T, F are atoms (uppercase)
id = { | x ⇒ x }              // x is pattern variable (lowercase)

λx. M  =  { | x ⇒ M }         // equivalent!
```

**But what about lowercase atoms?**

If we want atoms like `true`, `false`, `zero`, etc., we'd need:
- Uppercase for atoms: `True`, `False`, `Zero`
- Lowercase for variables: `x`, `y`, `z`

Or:
- All atoms must be capitalized
- All variables must be lowercase

**This is the ML/Haskell approach.**

---

### Option D: No Variable Binding in Patterns

**Radical alternative:** Pattern matching ONLY matches atoms/structures, NO variable binding.

**Then:**
- Lambda does ALL variable binding
- Patterns only check values

```
not = { | T ⇒ F | F ⇒ T }    // OK, matches atoms

id = λx. { | _ ⇒ x }          // lambda binds x, pattern matches anything

and = λa. λb. {
  | T ⇒ b      // if a is T, return b
  | F ⇒ F      // if a is F, return F
} a            // apply to a
```

**Wait, but how do we apply the pattern matcher to `a`?**

We'd need:
```
and = λa. λb. (
  ({ | T ⇒ b | F ⇒ F }) a
)
```

But `b` is free in the pattern branches - it's captured from outer lambda.

**This could work!**

**Example:**
```
f = λx. λy. (
  ({ | T ⇒ x | F ⇒ y }) x
)

f T F
= (λy. ({ | T ⇒ T | F ⇒ y }) T) F
= ({ | T ⇒ T | F ⇒ F }) T
= T
```

**In this model:**
- Lambda: `λx. M` binds x
- Pattern: `{ | T ⇒ M }` matches T (no binding!)
- No pattern variables at all!

**Then the equivalence BREAKS:**
```
λx. M  ≠  { | x ⇒ M }
```

Because `{ | x ⇒ M }` means "match atom x", not "bind anything".

---

## Re-examining Your Original Vision

### From DOC 1:

```
not:
  true -> false ;
  false -> true
```

Here, `true` and `false` are clearly atoms (specific values).

**This is pattern matching on atoms, not variable binding!**

---

```
and:
  false -> \x. false ;
  true  -> \x. x
```

**First branch:** If input is `false` (atom), return function `\x. false`
**Second branch:** If input is `true` (atom), return function `\x. x`

Again, pattern matching on atoms!

---

```
mt: \x.\y. (theorem x -> theorem y) -> (theorem (not y)) -> (theorem (not x))
```

Here, `\x.\y.` introduces variables `x` and `y`.

Then `(theorem x -> theorem y)` seems to be... a pattern that matches `(theorem x)` structure?

But `x` and `y` are already bound by lambda, so in the pattern, they'd be matching values of `x` and `y`.

Hmm, but that doesn't make sense either because we want to extract `x` and `y` from the pattern...

**Actually, I think your notation is:**
```
(theorem x -> theorem y)
```

is a pattern that:
- Matches structure `(theorem A -> theorem B)` 
- Binds `A` to `x` and `B` to `y`

So patterns DO bind variables in your vision!

---

## The Real Issue: Two Kinds of Pattern Matching?

Maybe there are TWO uses:

**1. Matching atoms (no binding):**
```
not = { | true ⇒ false | false ⇒ true }
```

**2. Matching structures (with binding):**
```
mp = { | (theorem (imp x y)) ⇒ ... }
```

In (2), `x` and `y` are being BOUND by the pattern, not matched!

**This suggests patterns CAN bind variables.**

---

## The Scoping Question

**Scenario:**
```
outer = λx. { | x ⇒ M }
```

**Interpretation A:** Inner `x` shadows outer `x` (pattern variable)
```
outer T
= { | x ⇒ M } T    // outer x→T
= M[x ↦ T]          // pattern x binds to T
```

**Interpretation B:** Inner `x` matches value of outer `x`
```
outer T
= { | T ⇒ M } T    // x is T, pattern becomes { | T ⇒ M }
= M                 // matches!

outer F
= { | T ⇒ M } F    // doesn't match, stuck
```

**Which do you intend?**

In ML/Haskell, it's (A) - pattern variables always shadow.

But maybe you want (B)?

---

## Proposed Resolution

### Hypothesis: Your Arrow Operator Works Differently

Your notation:
```
x -> M
```

Maybe `->` has special semantics:

**If LHS is a variable in scope:** Match value
**If LHS is an atom:** Match atom
**If LHS is a structure with variables:** Bind variables from structure

**Examples:**
```
not = true -> false ; false -> true
// 'true' and 'false' are atoms, match exactly

mp = (theorem (imp x y)) -> (theorem x) -> theorem y
// x and y are NOT in scope, so they're pattern variables
```

But this is getting complicated...

---

## Alternative: Explicit Pattern Variables

**Use special syntax for pattern variables:**

```
{ | $x ⇒ M }     // $x is pattern variable (binds)
{ | x ⇒ M }      // x is value (matches atom x)
```

Or:
```
{ | ?x ⇒ M }     // ?x binds
{ | x ⇒ M }      // x matches value
```

**Then:**
```
λx. M  =  { | ?x ⇒ M }    // NOW equivalent
```

---

## What Actually Needs to Be True

For your use case (logic programming), you need:

1. **Match atoms:** `{ | T ⇒ F | F ⇒ T }`
2. **Bind variables:** `{ | ?x ⇒ M }` or context-dependent
3. **Match structures with binding:** `{ | (theorem (imp ?x ?y)) ⇒ ... }`

**The question is notation and semantics.**

---

## My Error: I Assumed ML-Style Pattern Variables

I assumed lowercase identifiers in patterns automatically bind (like ML/Haskell).

But you might want:
- Lowercase identifiers are atoms
- Need explicit syntax for binding

Or:
- Context determines (pattern vs. term position)
- But need to be very clear about this

---

## Recommendation: Let's Be Explicit

**I suggest we have TWO distinct syntactic forms:**

**1. Lambda (explicit binding):**
```
λx. M    // binds x
```

**2. Pattern matching (may or may not bind):**
```
{ | pattern ⇒ M | ... }
```

**Pattern syntax:**
```
pattern ::= _                  // wildcard (matches anything, no binding)
          | atom               // matches specific atom
          | $var               // pattern variable (binds anything)
          | (pattern pattern)  // matches structure
```

**Then:**
```
λx. M  ≠  { | x ⇒ M }       // NOT equivalent!
λx. M  =  { | $x ⇒ M }      // equivalent!
```

**Examples:**
```
not = { | T ⇒ F | F ⇒ T }              // match atoms

id = λx. x  =  { | $x ⇒ $x }           // bind variable

mp = { | (theorem (imp $p $q)) ⇒ { | (theorem $p) ⇒ theorem $q } }
```

**Advantages:**
- Explicit and unambiguous
- No confusion about binding vs. matching
- Clear semantics

**Disadvantages:**
- Extra syntax (`$` or `?` prefix)
- More verbose

---

## Or: Adopt ML Convention with Clear Documentation

**Convention:**
- Uppercase: atoms (must match exactly) - `T`, `F`, `Zero`
- Lowercase in patterns: pattern variables (bind) - `x`, `y`, `p`
- Lowercase in terms: value variables (refer) - `x`, `y`, `p`

**Then:**
```
not = { | T ⇒ F | F ⇒ T }     // atoms

id = { | x ⇒ x }               // pattern var x binds

λx. M  =  { | x ⇒ M }          // equivalent
```

**The key:** Be very explicit in documentation about this convention!

---

## Questions for You

1. **Do you want patterns to bind variables?**
   - YES: We need pattern variables
   - NO: Lambda is sole binding mechanism, patterns only match atoms/structures

2. **Should `x` in a pattern bind or match?**
   - BIND: `{ | x ⇒ M }` equivalent to `λx. M`
   - MATCH: `{ | x ⇒ M }` matches atom `x`

3. **Should we use explicit syntax for pattern variables?**
   - YES: `{ | $x ⇒ M }` or `{ | ?x ⇒ M }`
   - NO: Use convention (uppercase/lowercase) or context

4. **What should happen with shadowing?**
   ```
   λx. { | x ⇒ M }
   ```
   - Inner `x` shadows (pattern variable)?
   - Inner `x` matches value of outer `x`?

---

## My Revised Understanding

**I think I was WRONG about the equivalence.**

Lambda and pattern matching are potentially DIFFERENT mechanisms, depending on your answers above.

**If patterns only match atoms (no binding):**
Then we NEED both lambda (for binding) and pattern matching (for branching).

**If patterns can bind variables:**
Then we need to distinguish pattern variables from value variables somehow.

---

## Next Steps

Please tell me:
1. Your answers to the questions above
2. Whether you want explicit syntax for pattern variables
3. Examples of how you envision patterns working in ambiguous cases

Then I'll reformulate the theory accordingly.

---

**Bottom line: You're right to question this. The lambda-pattern equivalence is NOT obvious and depends on design choices we need to make explicit.**
