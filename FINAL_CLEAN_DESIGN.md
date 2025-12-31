# Logi: Final Clean Design (No End Token!)

## The Key Insight: Pipe as Binary Infix Operator

**Your realization:**
```
a => b           // This is a function
a => b | c => d  // This is ALSO a function (with alternatives)
```

The `|` is a **binary infix operator** that joins two functions:
- LHS: first case
- RHS: second case (or more cases)
- If LHS fails to match, try RHS

**No `end` token needed!**

---

## Beautiful Consequences

### 1. Total Functions Are Super Clean

```
id = X => X                          // Single case, no pipe

add = X Y => plus X Y               // Multi-argument, no pipe

compose = F G X => F (G X)          // Three arguments, no pipe
```

**No syntactic noise when you don't need case-splitting!**

### 2. Currying Is Natural

Your example:
```
curry = A => B => C => f A B C
```

This is a curried ternary function:
- Takes `A`, returns function
- That function takes `B`, returns function  
- That function takes `C`, returns `f A B C`

**Curried and uncurried both work:**
```
uncurried = A B C => f A B C        // All patterns before =>
curried = A => B => C => f A B C    // Nested functions
```

### 3. Partial Functions Use Pipe

```
not = true => false | false => true

and = true X => X | false _ => false

head = (cons X Xs) => X             // No fallback case, partial!
```

**The presence of `|` signals "I have alternatives."**

---

## Grammar

### Syntax

```
term ::= atom                        // lowercase identifier
       | Var                         // uppercase identifier
       | term term                   // application
       | theorem term                // theorem marker
       | pattern+ => term            // single case function
       | term | term                 // choice (alternative cases)
       | (term)                      // grouping

pattern ::= atom                     // atom pattern (lowercase)
          | Var                      // variable pattern (uppercase)
          | _                        // wildcard
          | (pattern pattern)        // application pattern
          | theorem pattern          // theorem pattern
```

### Precedence (highest to lowest)

1. **Application** (left-associative)
   ```
   f a b c  =  ((f a) b) c
   ```

2. **Arrow `=>`** (right-associative)
   ```
   A => B => C  =  A => (B => C)
   ```

3. **Pipe `|`** (right-associative)
   ```
   a => b | c => d | e => f  =  (a => b) | ((c => d) | (e => f))
   ```

**Right-associativity of `|` ensures we try leftmost case first.**

---

## Examples

### Boolean NOT (Two Cases)

```
not = true => false | false => true
```

**Reduction:**
```
not true
= (true => false | false => true) true
= (true => false) true              // try first case
= false                              // matches!
```

```
not false
= (true => false | false => true) false
= (true => false) false              // try first case, no match
= (false => true) false              // try second case
= true                               // matches!
```

### Boolean AND (Two Cases)

```
and = true X => X | false _ => false
```

**Reduction:**
```
and true false
= (true X => X | false _ => false) true false
= (true X => X) true false          // try first case
= (X => X)[X := true] false         // first pattern matches
= (true => true) false              // substitute
= false                              // wait, this is wrong...
```

Actually, let me think about this more carefully. With multi-pattern:

```
and = true X => X | false _ => false
```

This takes TWO arguments at once. So:

```
and true false
= (true X => X | false _ => false) true false
= (match [true, false] against [true, X]) with body X
= X[X := false]
= false  ✓
```

```
and false true
= (match [false, true] against [true, X]) - no match
= (match [false, true] against [false, _]) with body false
= false  ✓
```

**Perfect!**

### Identity (Total, Single Case)

```
id = X => X
```

No pipe needed!

### Composition (Total, Three Arguments)

```
compose = F G X => F (G X)
```

**Usage:**
```
compose not not true
= (F G X => F (G X)) not not true
= (G X => not (G X)) not true
= (X => not (not X)) true
= not (not true)
= not false
= true  ✓
```

### Modus Ponens (Total, Two Arguments)

```
mp = (theorem (imp P Q)) (theorem P) => theorem Q
```

**One line, no pipe, beautiful!**

**Usage:**
```
mp (theorem (imp a b)) (theorem a)
= theorem b  ✓
```

If called with wrong structure:
```
mp (theorem (and a b)) (theorem a)
= (mp (theorem (and a b)) (theorem a))   // no match, doesn't reduce
```

Partial function behavior!

---

## Your Ternary Example

```
f = A => B => C => f A B C
```

**This is curried!** Let's trace it:

```
f x y z
= (A => B => C => f A B C) x y z
= (B => C => f x B C) y z           // A bound to x
= (C => f x y C) z                   // B bound to y
= f x y z                            // C bound to z
```

**Alternatively, uncurried:**
```
f = A B C => f A B C

f x y z
= (A B C => f A B C) x y z
= f x y z                            // All three bound at once
```

**Both styles work!**

---

## Case Splitting Examples

### Option Type

```
// Constructor
some = X => (some X)
none = none

// Pattern match on option
get_or_default = (some X) Default => X | none Default => Default
```

**Usage:**
```
get_or_default (some 42) 0  →  42
get_or_default none 0       →  0
```

### List Operations

```
// Constructors
nil = nil
cons = Head Tail => (cons Head Tail)

// Head (partial)
head = (cons X Xs) => X

// Tail (partial)
tail = (cons X Xs) => Xs

// Length
length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

**Usage:**
```
head (cons 1 nil)  →  1
head nil           →  (head nil)      // partial, doesn't reduce

length nil                →  zero
length (cons 1 nil)       →  (succ zero)
length (cons 1 (cons 2 nil))  →  (succ (succ zero))
```

---

## Inference Rules (All Clean!)

```
// Modus ponens (total)
mp = (theorem (imp P Q)) (theorem P) => theorem Q

// Modus tollens (total)
mt = (theorem (imp P Q)) (theorem (not Q)) => theorem (not P)

// Hypothetical syllogism (total)
hs = (theorem (imp P Q)) (theorem (imp Q R)) => theorem (imp P R)

// Conjunction introduction (total)
conj_intro = (theorem P) (theorem Q) => theorem (and P Q)

// Conjunction elimination left (total)
conj_elim_l = (theorem (and P Q)) => theorem P

// Conjunction elimination right (total)
conj_elim_r = (theorem (and P Q)) => theorem Q

// Disjunction introduction left (total over its signature)
disj_intro_l = (theorem P) Q => theorem (or P Q)

// Disjunction introduction right (total over its signature)
disj_intro_r = P (theorem Q) => theorem (or P Q)
```

**Every single one is one line!**

**No pipes needed because they're total over their expected patterns.**

---

## Parsing Examples

### Precedence Demonstration

```
f X Y => g X | h => Y
```

**Parse tree:**
```
(f X Y => g X) | (h => Y)
```

Application binds tighter than `=>`, which binds tighter than `|`.

### Currying vs. Multi-Pattern

```
A => B => C        // Curried: A => (B => C)
A B => C           // Multi-pattern: patterns [A, B] => C
```

Both valid, different behaviors!

### Complex Example

```
process = 
    (some X) Y => process_value X Y 
  | none Y => default_value Y
```

**Parse tree:**
```
((some X) Y => process_value X Y) | (none Y => default_value Y)
```

Clean!

---

## Semantics

### Single Case Function

```
pattern+ => body
```

When applied to arguments:
1. Try to match arguments against patterns
2. If all match with bindings σ, reduce to body[σ]
3. If any fail, no reduction (partial function)

### Choice Function

```
f | g
```

When applied to arguments:
1. Try `f` with arguments
2. If `f` doesn't reduce (no pattern match), try `g` with arguments
3. If neither reduces, no reduction

### Right-Associativity of Pipe

```
f | g | h  =  f | (g | h)
```

Try `f` first. If it fails, try `(g | h)`, which tries `g`, then `h`.

---

## Minimal Core

### Only 4 Constructs!

```
M ::= atom                  // lowercase
    | Var                   // uppercase
    | M M                   // application
    | pattern+ => M         // function
    | M | M                 // choice
    | theorem M             // theorem marker
```

Wait, that's 6. But if we consider `=>` and `|` as just operators:

**Core terms:**
```
M ::= atom
    | Var
    | M M
    | theorem M
```

**Operators:**
- `=>` (function creation)
- `|` (choice)

**Patterns:**
```
P ::= atom | Var | _ | P P | theorem P
```

---

## Comparison: Before vs. Now

### Before (With End Token)

```
not = 
  | true => false
  | false => true
  | end

id = 
  | X => X
  | end

mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end
```

### After (No End Token!)

```
not = true => false | false => true

id = X => X

mp = (theorem (imp P Q)) (theorem P) => theorem Q
```

**Dramatically cleaner for simple cases!**

---

## When Do You Use Pipe?

**Simple rule:**
- **No pipe:** Function is total over its signature (or intentionally partial with one case)
- **With pipe:** Function has multiple cases/alternatives

**Examples:**

**No pipe (total):**
```
id = X => X
add = X Y => plus X Y
mp = (theorem (imp P Q)) (theorem P) => theorem Q
```

**With pipe (alternatives):**
```
not = true => false | false => true
and = true X => X | false _ => false
length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

---

## Final Grammar (Complete)

```
program ::= definition*

definition ::= identifier = term

term ::= atom                       // lowercase identifier
       | Var                        // uppercase identifier  
       | term term                  // application (left-assoc)
       | theorem term               // theorem marker
       | function                   // function definition
       | (term)                     // grouping

function ::= pattern+ => term       // single case
           | function | function    // choice (right-assoc)

pattern ::= atom                    // atom pattern
          | Var                     // variable pattern
          | _                       // wildcard
          | (pattern pattern)       // application pattern
          | theorem pattern         // theorem pattern
```

**Precedence (highest to lowest):**
1. Application (left-assoc)
2. `=>` (right-assoc)
3. `|` (right-assoc)

---

## Summary of Final Design

1. ✅ **Pattern matching for all functions**
2. ✅ **Uppercase = variables, lowercase = atoms**
3. ✅ **Partial functions natural** (no match = no reduction)
4. ✅ **Multi-pattern per case** (clean syntax)
5. ✅ **No `end` token!** (pipe is binary infix)
6. ✅ **Total functions are minimal** (no syntactic noise)

**This is beautifully minimal and elegant!**

---

## Should I Rewrite Formal Spec?

This design is much cleaner than anything I proposed before. Should I:

1. Rewrite the formal specification (FORMAL_SPEC.md)?
2. Rewrite the proof examples (PROOF_EXAMPLES.md)?
3. Update all other documents to match?

**This is the cleanest design yet!** 🎉
