# Logi: Corrected Syntax - One Pattern Per Arrow

## ⚠️ HISTORICAL DOCUMENT - DESIGN EVOLUTION

**This document explains why we chose one pattern per arrow. The syntax shown is mostly correct but see formal spec for authoritative reference.**

**For current correct syntax, see:**
- [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md)
- [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)

---

# Original Document (Preserved Below)

## The Issue

```
add = X Y => plus X Y    // CONFUSING! Looks like X applied to Y
```

The patterns `X Y` before `=>` look like application, even though grammatically they're two separate patterns.

**Solution:** Each `=>` takes **exactly ONE pattern**. For multiple arguments, use currying.

---

## Corrected Rule

**Each arrow takes ONE pattern:**

```
pattern => body
```

**For multiple arguments, curry:**

```
X => Y => plus X Y       // Clear! Each arrow takes one pattern
```

---

## What Counts as "One Pattern"?

### Simple Patterns (Atomic)

**Variables:**
```
X => body
```

**Atoms:**
```
true => body
```

**Wildcards:**
```
_ => body
```

### Complex Patterns (Still One Pattern!)

**Structured patterns:**
```
(cons X Xs) => body              // ONE pattern (structure with sub-patterns)
(theorem (imp P Q)) => body      // ONE pattern (nested structure)
```

**The parentheses make it clear it's a single structured pattern.**

---

## Examples Corrected

### Boolean AND (Curried)

```
and = X => Y => (X => Y | _ => false) | _ => _ => false
```

Wait, that's getting messy. Let me think...

Actually, with currying, we need to be more careful. Let me reconsider.

**With curried style:**
```
and = X => (
    X => true => X | false => false
  | _ => false
)
```

Hmm, this is getting awkward because we need to inspect `X` twice...

Actually, let me think about this differently. With currying:

```
and = X => Y => ((true => Y | false => false) | _ => false)
```

Wait, I need to apply the first case matcher to `X`:

```
and = X => Y => (true => Y | false => false) X
```

No wait, that's not right either...

Let me restart. With currying:

```
and = X => (true => (Y => Y) | false => (Y => false))
```

**Usage:**
```
and true false
= (X => (true => (Y => Y) | false => (Y => false))) true false
= (true => (Y => Y) | false => (Y => false)) true false
= ((Y => Y) | ...) false
= (Y => Y) false
= false  ✓
```

Actually, simpler:

```
and = X => Y => ((true => Y | false => false) X)
```

But this requires applying a case expression to `X`, which should work.

Or even simpler, since we're currying, just nest the cases:

```
and = true => (X => X) | false => (_ => false)
```

**Usage:**
```
and true false
= (true => (X => X) | false => (_ => false)) true false
= (X => X) false
= false  ✓

and false true  
= (true => (X => X) | false => (_ => false)) false true
= (_ => false) true
= false  ✓
```

**This works!**

### Modus Ponens (Curried)

```
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
```

Each `=>` takes one pattern:
1. First pattern: `(theorem (imp P Q))` (one complex pattern)
2. Second pattern: `(theorem P)` (one complex pattern)

**Beautiful!**

### Addition (Curried)

```
add = X => Y => plus X Y
```

Clear and unambiguous!

### Composition (Curried)

```
compose = F => G => X => F (G X)
```

Three arguments, three arrows!

---

## Grammar Update

### Terms

```
term ::= atom                      // lowercase
       | Var                       // uppercase
       | term term                 // application
       | theorem term              // theorem marker
       | pattern => term           // function (ONE pattern!)
       | term | term               // choice
       | (term)                    // grouping
```

### Patterns (ONE of these per arrow)

```
pattern ::= atom                   // atom pattern
          | Var                    // variable pattern
          | _                      // wildcard
          | (pattern pattern)      // structured pattern (ONE pattern!)
          | theorem pattern        // theorem pattern
```

**Key:** `(pattern pattern)` is still ONE pattern - a structured pattern with sub-patterns.

---

## Examples (All Corrected)

### Boolean Logic

```
true = true
false = false

not = true => false | false => true

and = true => (X => X) | false => (_ => false)

or = true => (_ => true) | false => (X => X)
```

### List Operations

```
nil = nil
cons = X => Xs => (cons X Xs)

head = (cons X Xs) => X

tail = (cons X Xs) => Xs

length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

### Inference Rules

```
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

mt = (theorem (imp P Q)) => (theorem (not Q)) => (theorem (not P))

hs = (theorem (imp P Q)) => (theorem (imp Q R)) => (theorem (imp P R))

conj_intro = (theorem P) => (theorem Q) => (theorem (and P Q))

conj_elim_l = (theorem (and P Q)) => (theorem P)

conj_elim_r = (theorem (and P Q)) => (theorem Q)
```

**All clean and unambiguous!**

---

## Why This Is Better

### 1. No Visual Ambiguity

**Bad (looks like application):**
```
add = X Y => plus X Y
```

**Good (clearly curried):**
```
add = X => Y => plus X Y
```

### 2. Structured Patterns Still Work

**Complex pattern (still one pattern):**
```
head = (cons X Xs) => X
```

The parentheses make it clear `(cons X Xs)` is a single structured pattern.

### 3. Forces Good Style

Currying is clearer and more composable:

```
add5 = add five     // Partial application
```

This only works naturally with curried functions!

---

## Edge Cases

### Multiple Structured Patterns?

**Not allowed:**
```
swap = (X Y) (A B) => (A B) (X Y)    // NO! Looks like application
```

**Must curry:**
```
swap = (X Y) => (A B) => (A B) (X Y)    // YES! Clear
```

### Nested Arrows

**This is fine:**
```
f = X => (Y => Y)
```

Returns a function.

**Also fine:**
```
g = X => Y => X
```

Constant function (K combinator).

---

## Precedence (Unchanged)

**Lowest to highest:**
1. `|` (choice) - right-associative
2. `=>` (function) - right-associative  
3. Application - left-associative

**Examples:**

```
X => Y => Z    =    X => (Y => Z)                  // => is right-assoc

f X Y          =    (f X) Y                         // application is left-assoc

a => b | c => d    =    (a => b) | (c => d)        // | is right-assoc
```

---

## Complete Examples

### Boolean AND (Fixed)

```
and = true => (X => X) | false => (_ => false)
```

**Trace:**
```
and true false
= (true => (X => X) | false => (_ => false)) true false
= (X => X) false
= false  ✓
```

### Modus Ponens (Perfect)

```
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
```

**Trace:**
```
mp (theorem (imp a b)) (theorem a)
= ((theorem (imp P Q)) => (theorem P) => theorem Q) (theorem (imp a b)) (theorem a)
= ((theorem P) => theorem Q)[P := a, Q := b] (theorem a)
= ((theorem a) => theorem b) (theorem a)
= theorem b  ✓
```

### List Length (Perfect)

```
length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

**Trace:**
```
length (cons 1 (cons 2 nil))
= (nil => zero | (cons _ Xs) => ...) (cons 1 (cons 2 nil))
= (cons _ Xs => (succ (length Xs))) (cons 1 (cons 2 nil))
= (succ (length (cons 2 nil)))[Xs := (cons 2 nil)]
= (succ (length (cons 2 nil)))
= (succ (succ (length nil)))
= (succ (succ zero))  ✓
```

---

## Updated Grammar (Final)

```
program ::= definition*

definition ::= identifier = term

term ::= atom                      // lowercase identifier
       | Var                       // uppercase identifier
       | term term                 // application (left-assoc)
       | theorem term              // theorem marker
       | pattern => term           // function (ONE pattern)
       | term | term               // choice (right-assoc)
       | (term)                    // grouping

pattern ::= atom                   // atom pattern
          | Var                    // variable pattern
          | _                      // wildcard
          | (pattern pattern)      // structured pattern (still ONE pattern)
          | theorem pattern        // theorem pattern
```

**Key rule:** Each `=>` takes **exactly ONE pattern**.

For multiple arguments: **curry** with nested `=>`.

---

## Summary

**Fixed the ambiguity:**
- ❌ `X Y => body` (looks like application)
- ✅ `X => Y => body` (clearly curried)

**Structured patterns still work:**
- ✅ `(cons X Xs) => body` (one complex pattern)
- ✅ `(theorem (imp P Q)) => body` (one complex pattern)

**Forces good style:**
- Currying enables partial application
- Visual clarity
- No ambiguity

**This is the correct design!** 🎯
