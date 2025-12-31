# Exploring Arrow Semantics: Context-Dependent Binding

## ⚠️ HISTORICAL DOCUMENT - REJECTED APPROACH

**This document explores an approach that was ultimately rejected in favor of the simpler design.**

**For current correct syntax, see:**
- [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md)
- [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)

---

# Original Document (Preserved Below)

## A Different Approach: Smart Arrow Operator

### Hypothesis

What if the arrow operator examines its LHS to determine behavior?

```
P -> M
```

**Semantics:**
- If P contains **free variables**, they become pattern variables (bind from input)
- If P is **closed** (no free variables), check equality with input

---

## Examples

### Case 1: Closed LHS (Equality Check)

```
not = (T -> F) ; (F -> T)
```

- `T` and `F` are atoms (closed terms)
- Arrow checks if input ≡ T or input ≡ F
- No binding happens

**Reduction:**
```
not T
= ((T -> F) ; (F -> T)) T
= (T -> F) T           // T is closed, check equality
= F                     // T ≡ T, return F
```

### Case 2: Open LHS (Pattern Binding)

```
id = x -> x
```

- `x` is a free variable in LHS
- Arrow binds input to `x`
- Returns `x` (now bound to input)

**Reduction:**
```
id T
= (x -> x) T
[x is free in LHS, bind: x ↦ T]
= x[x ↦ T]
= T
```

**Now `λx. x  =  x -> x` because both bind!**

### Case 3: Mixed (Structure with Variables)

```
mp = (theorem (imp p q)) -> ((theorem p) -> theorem q)
```

- `p` and `q` are free variables in LHS
- Arrow matches structure and binds `p` and `q`

**Reduction:**
```
mp (theorem (imp A B))
= ((theorem (imp p q)) -> ...) (theorem (imp A B))
[Match structure, bind: p ↦ A, q ↦ B]
= ((theorem p) -> theorem q)[p ↦ A, q ↦ B]
= ((theorem A) -> theorem B)

(mp (theorem (imp A B))) (theorem A)
= ((theorem A) -> theorem B) (theorem A)
[A is closed, check equality: theorem A ≡ theorem A ✓]
= theorem B
```

---

## The Key Insight

**Free variables in arrow LHS become pattern variables automatically!**

No explicit syntax needed:
- Atoms are atoms
- Variables are pattern variables (if free in LHS)
- Bound variables are values (check equality)

---

## Formal Semantics

### Pattern Matching with Free Variables

Given arrow `P -> M` applied to value `V`:

1. Find all free variables in P: `FV(P) = {x₁, x₂, ...}`
2. Try to match V against P, treating free variables as pattern variables
3. If match succeeds with bindings `σ = {x₁ ↦ V₁, x₂ ↦ V₂, ...}`
4. Return `M[σ]`

### Match Algorithm

```
match(V, P, FV):
  if P is atom:
    if V ≡ P: return {}
    else: fail
  
  if P is variable x:
    if x ∈ FV:  return {x ↦ V}      // pattern variable
    else:       if V ≡ P: return {} else fail   // bound variable, check equality
  
  if P is (P₁ P₂):
    σ₁ = match(V.lhs, P₁, FV)
    σ₂ = match(V.rhs, P₂, FV)
    return σ₁ ∪ σ₂
  
  if P is (theorem P'):
    if V is (theorem V'):
      return match(V', P', FV)
    else: fail
```

---

## Examples Revisited

### Boolean NOT (closed patterns)

```
not = (T -> F) ; (F -> T)

FV(T) = {}    // no free variables
FV(F) = {}    // no free variables
```

Equality checking only. ✓

### Identity (open pattern)

```
id = x -> x

FV(x) = {x}   // x is free
```

Binds x to input. ✓

### Modus Ponens (mixed)

```
mp = (theorem (imp p q)) -> ((theorem p) -> theorem q)

FV((theorem (imp p q))) = {p, q}   // p and q are free
```

Matches structure, binds p and q. ✓

### With Lambda Context

```
f = λx. (x -> F ; y -> T)

```

In the context of `λx`, the variable `x` is bound.

```
FV(x) = {}    // x is bound by lambda
FV(y) = {y}   // y is free
```

First arrow: checks equality with bound x
Second arrow: binds y

**Example:**
```
f T
= (λx. (x -> F ; y -> T)) T
= (T -> F ; y -> T)           // x substituted with T
= (T -> F) T ; ...            // first arrow: T closed, check equality
= F                            // T ≡ T, return F
```

```
f A   // A is some other atom
= (A -> F ; y -> T)
= (A -> F) A ; ...            // A closed, check equality
= F                            // A ≡ A, return F
```

**This works!**

---

## Advantages

1. **No explicit syntax** for pattern variables (`$x`, `?x`, etc.)
2. **Natural semantics** - free variables bind, closed terms check equality
3. **Unifies lambda and arrow** - `λx. M ≈ x -> M` when x is free
4. **Matches your intuition** from DOC 1

---

## Potential Issues

### Issue 1: Accidental Pattern Variables

```
// Intended: check equality with global constant
global_true = T

test = global_true -> F
```

If `global_true` is a free variable (not yet substituted), it becomes a pattern variable!

**Solution:** Use substitution eagerly, or require definitions to be closed.

### Issue 2: Shadowing Confusion

```
outer = λx. (x -> F)
```

The inner `x` is bound by outer lambda, so:
```
FV(x) = {}    // x is bound
```

Arrow checks equality, not binding. This is probably desired behavior.

### Issue 3: Partial Application Complications

```
curry = λx. (x -> λy. y)

curry T
= (T -> λy. y)   // x substituted with T, now T is closed
= (T -> λy. y) T // apply to T
= λy. y           // T ≡ T, return λy. y
```

This works, but:

```
curry x   // x is free
= (λx. (x -> λy. y)) x
= (x -> λy. y)    // x is still free!

FV(x) = {x}
```

If we then apply:
```
(x -> λy. y) T
[x is free, bind: x ↦ T]
= (λy. y)[x ↦ T]
= λy. y
```

**This is correct!**

---

## Comparison to Lambda

### Lambda

```
λx. M
```

- Always binds x (parameter)
- x scoped to M

### Arrow with Free Variable

```
x -> M
```

- If x is free, binds x (pattern variable)
- x scoped to M

**They're semantically very similar!**

### When Are They Different?

When LHS of arrow is not just a variable:

```
(T -> M)       // checks equality with T
λx. M          // binds anything to x
```

These are different.

```
(f x) -> M     // matches structure (f ...)
λy. M          // binds anything to y
```

These are different.

**So arrow is MORE general than lambda.**

---

## Can We Eliminate Lambda?

If arrow subsumes lambda when LHS is a free variable...

**Can we just use arrow for everything?**

```
not = (T -> F) ; (F -> T)        // ok

id = x -> x                       // ok (x free, binds)

and = a -> (b -> ((T -> b) ; (F -> F)) a)   // ok?
```

**Wait, but:** The semantics gets confusing when we nest arrows.

In `a -> (b -> ...)`, both `a` and `b` are free at different levels.

Actually, let me think:
```
and = a -> (b -> ((T -> b) ; (F -> F)) a)

FV(entire term) = {}              // if this is a top-level definition

and T F
= (a -> (b -> ((T -> b) ; (F -> F)) a)) T F
= (b -> ((T -> b) ; (F -> F)) T) F    [a ↦ T]
= ((T -> F) ; (F -> F)) T             [b ↦ F]
= (T -> F) T
= F   ✓
```

**This works!**

So yes, arrow can replace lambda if we interpret free variables as pattern variables!

---

## Revised Minimal Calculus

### Syntax

```
M ::= x              // variable (or atom)
    | M -> M         // arrow (binding or matching, depending on LHS freeness)
    | M ; M          // choice
    | M M            // application
    | theorem M      // theorem marker
```

**Only 5 constructs!**

No separate lambda needed - arrow with free variable does the same thing.

---

## Syntactic Sugar

For familiarity, we can offer:

```
λx. M   ≡   x -> M    // sugar for arrow with free variable

f = λx. λy. M
  ≡ x -> (y -> M)     // nested arrows
```

---

## Examples with Revised Calculus

### Identity

```
id = x -> x          // x free, binds to input
```

### Boolean NOT

```
not = (T -> F) ; (F -> T)    // T and F closed (atoms), equality check
```

### Boolean AND

```
and = a -> (b -> ((T -> b) ; (F -> F)) a)
```

### Modus Ponens

```
mp = (theorem (imp p q)) -> ((theorem p) -> theorem q)
```

**All work with just arrows!**

---

## Addressing Your Original Concern

You said:
> "the lhs declares a function that takes in something, calls it `x` and returns body `M`"
> "the rhs declares a pattern-matcher which will return `M` only if supplied something that is EXACTLY x"

**My answer:**

It depends on whether `x` is free or bound!

- **If `x` is free:** `x -> M` binds anything to x (like lambda)
- **If `x` is bound:** `x -> M` checks equality with x (pattern matching)

**Both are useful, and the context determines which!**

---

## Key Realization

**The arrow operator is MORE GENERAL than lambda.**

- Lambda: always binds
- Arrow: binds (if LHS has free vars) OR checks equality (if LHS closed)

**This is actually quite elegant!**

---

## Does This Match Your Vision?

Your original notation:
```
not = true -> false ; false -> true

and = false -> \x. false ; true -> \x. x

mp = ...pattern matching...
```

**Interpretation:**
- `true` and `false` are atoms (closed)
- Arrows check equality
- When free variables appear in LHS, they bind

**This is exactly what the smart arrow semantics gives you!**

---

## Revised Recommendation

**Use the context-dependent arrow semantics:**

```
P -> M
```

**Where:**
- Free variables in P become pattern variables (bind from input)
- Closed terms in P are checked for equality
- Mixed structures (like `(f x y)`) match structure and bind free vars

**This gives you:**
1. Minimal syntax (5 constructs, no separate lambda)
2. Intuitive semantics (free vars bind, atoms match)
3. Powerful pattern matching with destructuring
4. Lambda-like behavior when needed (`x -> M` when x free)

**Lambda can be syntactic sugar if desired:**
```
λx. M  ≡  x -> M    // when x is free
```

---

## Updated Theory Status

**Core calculus:**

```
M ::= x | M -> M | M ; M | M M | theorem M
```

**Semantics:** Arrow is context-dependent based on free variables in LHS.

**Examples:** Boolean logic, modus ponens, all work.

**Advantage:** Truly minimal - only 5 constructs!

**The equivalence `λx. M = x -> M` is TRUE when x is free in the arrow LHS!**

---

## Does This Resolve Your Concern?

**You were right to question my original claim.**

But with context-dependent arrow semantics:
- `λx. M` and `x -> M` (with x free) DO have the same behavior
- They both bind x to any input
- The difference is: arrow can ALSO do equality checking when LHS is closed

**So arrow subsumes lambda, but they're equivalent when LHS is a single free variable.**

What do you think of this approach?
