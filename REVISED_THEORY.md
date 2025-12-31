# Revised Theory: Lambda + Arrow Operators

## Starting Fresh: Your Original Insight

From your DOC 1, you had this key idea:

> "The arrow operator denotes pattern matching. When it is applied to an input, the input is checked to be exactly beta-equivalent to the lhs of the arrow, and if so, the rhs of the arrow will be produced."

**This is CHECKING EQUALITY, not binding variables!**

---

## Hypothesis: Three Distinct Constructs

### 1. Lambda (Variable Binding)

```
λx. M
```

- Takes ANY argument
- Binds it to name `x`
- Returns `M` with `x` substituted
- **This is BINDING**

### 2. Arrow (Equality Checking)

```
P -> R
```

- Takes an argument
- Checks if argument ≡ P (beta-equivalent)
- If yes, returns R
- If no, fails/stuck
- **This is PATTERN MATCHING on equality**

### 3. Choice (Alternatives)

```
M₁ ; M₂
```

- Try M₁ first
- If M₁ gets stuck, try M₂
- **This is FALLBACK**

---

## Examples with This Model

### Boolean NOT

```
not = (T -> F) ; (F -> T)
```

**Reduction:**
```
not T
= ((T -> F) ; (F -> T)) T
= (T -> F) T               // try first branch
= F                         // T ≡ T, return F
```

**This works! No variable binding needed.**

### Identity Function

```
id = λx. x
```

**NOT the same as:**
```
wrong_id = (x -> x)   // This checks if input ≡ x (some specific value)
```

**Lambda is needed for identity!**

### Boolean AND

Your example:
```
and = λa. λb. (
  (T -> b) ; (F -> F)
) a
```

Wait, this applies the pattern matcher to `a`. Let me think through reduction:

```
and T F
= (λa. λb. ((T -> b) ; (F -> F)) a) T F
= (λb. ((T -> b) ; (F -> F)) T) F
= ((T -> F) ; (F -> F)) T
= (T -> F) T
= F   ✓
```

```
and T T  
= ((T -> T) ; (F -> F)) T
= (T -> T) T
= T   ✓
```

```
and F F
= ((T -> F) ; (F -> F)) F
= (T -> F) F         // first branch: F ≢ T, stuck
= ; (F -> F) F       // fallback to second branch
= (F -> F) F
= F   ✓
```

**This works!**

---

## The Key Insight: Arrow LHS Can Contain Variables

From your DOC 1:
```
mt: \x.\y. (theorem x -> theorem y) -> (theorem (not y)) -> (theorem (not x))
```

Wait, the LHS `(theorem x -> theorem y)` contains variables `x` and `y`.

**Question:** What does this mean?

**Interpretation:** After lambda binds `x` and `y`, the term `(theorem x -> theorem y)` becomes a specific value (with `x` and `y` substituted). Then the arrow checks if the input equals that value.

**Example:**
```
mt_instance = mt P Q
= \x.\y. (theorem x -> theorem y) -> ... [x ↦ P, y ↦ Q]
= (theorem P -> theorem Q) -> (theorem (not Q)) -> (theorem (not P))
```

Now the arrow checks if input equals `(theorem P -> theorem Q)`.

**But wait...** This doesn't help us EXTRACT components from a structure!

If we have `theorem (imp A B)` and want to extract `A` and `B`, how do we do it?

---

## The Missing Piece: Destructuring Patterns

**We need a way to bind variables from matched structures.**

### Option 1: Patterns Can Have Variables

Let arrow LHS be a PATTERN, not just a value:

```
(theorem (imp ?x ?y)) -> M
```

**Semantics:**
- If input has form `(theorem (imp V₁ V₂))` for any values V₁, V₂
- Bind `x ↦ V₁` and `y ↦ V₂`  
- Return `M` with `x` and `y` substituted

**This introduces pattern variables!**

### Option 2: Pattern Variables Are Explicit

Use special syntax:

```
(theorem (imp $x $y)) -> M     // $x and $y are pattern variables
```

Or:

```
(theorem (imp _x _y)) -> M     // _x and _y bind
```

### Option 3: Matching + Projection

Provide projection functions to extract components:

```
fst = λp. <extract first element of p>
snd = λp. <extract second element of p>
```

Then:
```
mp = λimpl. λante.
  (match impl with (theorem (imp _ _)) ->
    let p = (get_premise impl) in
    let q = (get_conclusion impl) in
    (match ante with (theorem p2) ->
      (if p ≡ p2 then theorem q else stuck)))
```

**Too complicated!**

---

## Reconsidering: Pattern Variables Are Necessary

**For logic programming, we NEED to destructure terms.**

```
mp = {
  | (theorem (imp ?p ?q)) ⇒ {
    | (theorem ?p) ⇒ theorem ?q
  }
}
```

We need to:
1. Match structure `(theorem (imp _ _))`
2. Bind the components to variables `p` and `q`
3. Use them in the result

**This requires pattern variables.**

---

## Revised Model: Lambda + Pattern Arrow

### Constructs

```
M ::= x                          // variable
    | a                          // atom
    | λx. M                      // lambda (binding)
    | M -> M                     // arrow (pattern matching)
    | M ; M                      // choice
    | M M                        // application
    | theorem M                  // theorem marker
```

### Pattern Syntax in Arrow LHS

Arrow LHS can be:
```
P ::= x                          // if x is atom: match x
    | $x                         // pattern variable (binds)
    | (P P)                      // structure pattern
    | theorem P                  // theorem pattern
```

**Key distinction:**
- `x` in arrow LHS: matches atom `x`
- `$x` in arrow LHS: binds anything to `x`
- `x` in lambda: `λx` binds anything to `x`

---

## Examples with Revised Model

### Boolean NOT (no binding needed)

```
not = (T -> F) ; (F -> T)
```

### Identity (needs lambda)

```
id = λx. x
```

### Boolean AND (lambda + arrow)

```
and = λa. λb. ((T -> b) ; (F -> F)) a
```

### Modus Ponens (lambda + pattern arrow)

```
mp = λimpl. λante. (
  ((theorem (imp $p $q)) -> (
    ((theorem $p) -> theorem $q) ; fail
  ) ante) ; fail
) impl
```

**Wait, this is getting messy...**

Actually, with multiple arguments, we need nested patterns:

```
mp = (
  (theorem (imp $p $q)) -> (
    (theorem $p2) -> (
      if ($p ≡ $p2) then theorem $q else fail
    )
  )
)
```

**No, we need to be able to refer to the same pattern variable...**

---

## The Problem: Pattern Variables Need Scope

If we write:
```
(theorem (imp $p $q)) -> ((theorem $p) -> theorem $q)
```

The inner `$p` needs to refer to the SAME `$p` bound by the outer pattern.

**This means pattern variables have scope across nested arrows!**

But then the semantics gets complicated...

---

## Alternative: Back to Matching Functions

Maybe the issue is that arrow is too limited. We need full pattern matching functions:

```
mp = match {
  | (theorem (imp $p $q)), (theorem $p) ⇒ theorem $q
}
```

But this is just what I had before with `{ | ... ⇒ ... }`!

---

## Core Realization

**You're right that lambda and pattern matching are different:**

- **Lambda:** `λx. M` - parameter binding, takes any argument
- **Pattern matching:** Checks structure, may extract components

**But for pattern matching to be useful, it MUST support variable binding (extracting components).**

So we need BOTH:
1. Lambda for parameters
2. Pattern matching WITH pattern variables for destructuring

**The question is notation.**

---

## Proposed Syntax Options

### Option A: Explicit Pattern Variables

```
λx. M                    // lambda binds x

(T -> M)                 // arrow matches atom T
($x -> M)                // arrow binds anything to x
((theorem $p) -> M)      // arrow matches structure, binds p
```

**Pros:** Explicit, unambiguous
**Cons:** Extra syntax

### Option B: Context-Dependent

```
λx. M                    // lambda binds x (context: lambda parameter)

(T -> M)                 // arrow matches atom T (T is uppercase)
(x -> M)                 // arrow matches atom x (x is lowercase, treated as atom)
((theorem x) -> M)       // x in pattern position binds (destructuring)
```

**Pros:** Less syntax
**Cons:** Confusing, context-dependent

### Option C: Unified Matching Function Syntax

Go back to:
```
{ | pattern ⇒ M | ... }
```

Where patterns can have variables (by convention or explicit syntax).

---

## My Error Was Claiming Equivalence

**I claimed:**
```
λx. M  =  { | x ⇒ M }
```

**This is only true IF:**
- `x` in pattern position means "bind anything" (pattern variable)
- NOT "match atom x"

**The confusion arises from:** How do we distinguish pattern variables from atoms?

---

## Your Original Notation Revisited

```
not = true -> false ; false -> true
```

Here, `true` and `false` are atoms.

```
and = false -> \x. false ; true -> \x. x
```

Same - `false` and `true` are atoms.

**So far, no pattern variables needed!**

---

But then:

```
mp = \impl. \ante. ...
```

How do you extract `P` and `Q` from `(theorem (imp P Q))`?

**You need SOME way to destructure!**

Options:
1. Built-in pattern matching with variables
2. Projection functions (accessor functions)
3. Something else?

---

## Question for You

**How do you envision extracting components from structures?**

**Example:** Given `(theorem (imp A B))`, how do you get `A` and `B`?

**Option 1: Pattern matching with variables**
```
mp = ((theorem (imp ?p ?q)) -> ...) impl
```

**Option 2: Accessor functions**
```
get_premise = λimpl. <extract P from (imp P Q)>
get_conclusion = λimpl. <extract Q from (imp P Q)>

mp = λimpl. 
  let p = get_premise impl in
  let q = get_conclusion impl in
  ...
```

**Option 3: Something else?**

---

## Tentative Revised Proposal

**Accept that we need TWO distinct mechanisms:**

1. **Lambda** - parameter binding
   ```
   λx. M
   ```

2. **Pattern matching** - structure checking + destructuring
   ```
   (pattern -> M) ; ...
   ```

**Patterns can contain:**
- Atoms (match exactly)
- Pattern variables (bind components)
- Structure (recursive matching)

**Notation for pattern variables:**

**Option A:** Explicit prefix
```
($x -> M)           // pattern variable
(T -> M)            // atom
```

**Option B:** Convention
```
(x -> M)            // if x is free: pattern variable; if x is bound: match value
(T -> M)            // uppercase: atom
```

**Option C:** Always atoms unless in destructuring position
```
(T -> M)            // atom T
((f x y) -> M)      // match structure, x and y are pattern variables (by position)
```

---

## Evaluation Semantics

### Lambda Application (unchanged)

```
(λx. M) V  ⟹  M[x := V]
```

### Arrow Application

```
(P -> R) V  ⟹  R[σ]    if match(V, P) = σ
(P -> R) V  ⟹  stuck   if match(V, P) = fail
```

Where `match(V, P)` returns bindings for pattern variables in P, or fail.

### Choice

```
(M₁ ; M₂) V  ⟹  M₁ V               if M₁ V doesn't get stuck
(M₁ ; M₂) V  ⟹  M₂ V               if M₁ V gets stuck
```

---

## Concrete Example: Modus Ponens

**With explicit pattern variable syntax:**

```
mp = 
  ((theorem (imp $p $q)) -> 
    ((theorem $p) -> 
      theorem $q))
```

**Wait, how do pattern variables scope?**

When we match `(theorem (imp $p $q))`, we bind `$p` and `$q`.
Then the result `((theorem $p) -> ...)` uses those bindings.

So pattern variables in the RHS of an arrow refer to variables bound by the LHS pattern!

**Semantics:**

```
mp (theorem (imp A B))
= ((theorem (imp $p $q)) -> ...) (theorem (imp A B))
[match binds: $p ↦ A, $q ↦ B]
= ((theorem $p) -> theorem $q)[$p ↦ A, $q ↦ B]
= ((theorem A) -> theorem B)
```

**Then:**
```
(mp (theorem (imp A B))) (theorem A)
= ((theorem A) -> theorem B) (theorem A)
= theorem B   ✓
```

**This works!**

---

## Revised Core Calculus

### Syntax

```
Terms (M):
  M ::= x                // variable
      | a                // atom
      | λx. M            // lambda abstraction
      | P -> M           // pattern arrow
      | M ; M            // choice
      | M M              // application
      | theorem M        // theorem marker

Patterns (P):
  P ::= a                // atom (match exactly)
      | $x               // pattern variable (bind anything)
      | P P              // application pattern
      | theorem P        // theorem pattern
```

### Semantics

**Pattern matching:**
```
match(V, a)        = {} if V = a, else fail
match(V, $x)       = {$x ↦ V}
match(V₁ V₂, P₁ P₂) = σ₁ ∪ σ₂ if match(V₁,P₁)=σ₁, match(V₂,P₂)=σ₂
match(theorem V, theorem P) = match(V, P)
```

**Arrow reduction:**
```
(P -> M) V  ⟹  M[σ]    if match(V, P) = σ
```

**Lambda reduction (unchanged):**
```
(λx. M) V  ⟹  M[x := V]
```

**Choice:**
```
(M₁ ; M₂) V  ⟹  M₁ V    if M₁ V ≠ stuck
(M₁ ; M₂) V  ⟹  M₂ V    if M₁ V = stuck
```

---

## Does This Match Your Vision?

**Your examples:**

```
not = (T -> F) ; (F -> T)     ✓

and = λa. λb. ((T -> b) ; (F -> F)) a     ✓

mp = ((theorem (imp $p $q)) -> ((theorem $p) -> theorem $q))     ✓
```

**Key points:**

1. Lambda and pattern arrow are BOTH primitives
2. Lambda binds parameters (takes any argument)
3. Arrow matches patterns (checks structure, may bind pattern variables)
4. Pattern variables use `$` prefix (or your preferred syntax)

---

## What Changed From My Original Theory

**Before:** I claimed pattern matching subsumes lambda
```
λx. M  =  { | x ⇒ M }
```

**Now:** Lambda and pattern matching are distinct
```
λx. M         // binds parameter (any argument)
($x -> M)     // pattern variable (any argument, via pattern matching)
(T -> M)      // pattern matching on atom T
```

**Lambda and pattern arrow with pattern variable are SIMILAR but not identical:**
- Lambda creates a function
- Pattern arrow with pattern variable creates a pattern-matching function

In terms of behavior:
```
λx. M  ≈  ($x -> M)
```

But they're syntactically distinct constructs.

---

## Summary of Revised Theory

**Four core constructs for binding/branching:**

1. **Lambda:** `λx. M` - parameter binding
2. **Pattern arrow:** `P -> M` - pattern matching
3. **Choice:** `M₁ ; M₂` - alternatives
4. **Application:** `M N` - function application

**Plus:**

5. **Variables:** `x` - refer to bound names
6. **Atoms:** `T` - primitive values
7. **Theorem marker:** `theorem M` - mark propositions

**Total: 7 constructs** (more than my original claim of 5!)

**But this correctly captures the distinction between binding and matching.**

---

## Questions

1. Do you want `$x` syntax for pattern variables, or something else?
2. Should we still have the unified `{ | ... ⇒ ... }` syntax as sugar?
3. Any other aspects of your vision I'm still missing?

**I apologize for the confusion in my original theory. Your critique was absolutely correct.**
