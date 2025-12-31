# Logi: Final Design with Partial Functions

## Core Insight: Partial Functions Are Natural

Pattern matching creates partial functions automatically:

```
(abc => def) abc  →  def        // pattern matches
(abc => def) abd  →  (abc => def) abd   // no match, no reduction
```

**If no pattern matches, the term doesn't reduce. It's just undefined for that input.**

This is exactly the behavior of partial functions! Elegant.

---

## Convention: UPPERCASE = Variables, lowercase = atoms

**Your proposal:**
- **lowercase letters** = atoms (match exactly)
- **UPPERCASE letters** = variables (pattern variables, bind anything)

### Examples

**Atoms:**
```
true, false, zero, succ, nil, cons
```

**Variables:**
```
X, Y, Z, P, Q, Head, Tail
```

**This is the OPPOSITE of ML/Haskell, but that's fine! It's your language.**

---

## Why This Makes Sense

### 1. Atoms are Common, Variables Less So

In logic programming, you often work with specific atoms:
```
not = 
  | true => false
  | false => true
  | end
```

Lowercase for the common case (atoms) is convenient.

### 2. Variables Stand Out

Uppercase variables are visually distinct:
```
mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end
```

Easy to see that `P` and `Q` are being bound.

### 3. Prolog-like

Prolog uses uppercase for variables, lowercase for atoms. Familiar to logic programmers!

---

## Complete Syntax

### Terms

```
term ::= 
  | lowercase_id             // atom
  | UPPERCASE_ID             // variable (in term position, refers to bound var)
  | (term term)              // application
  | theorem term             // theorem marker
  | caseExpr                 // pattern matching function

caseExpr ::=
  | pattern+ => term caseExpr
  | end
```

### Patterns

```
pattern ::=
  | lowercase_id             // atom pattern (match exactly)
  | UPPERCASE_ID             // variable pattern (bind anything)
  | _                        // wildcard (match anything, don't bind)
  | (pattern pattern)        // application pattern
  | theorem pattern          // theorem pattern
```

---

## Examples Revisited

### Boolean NOT

```
not = 
  | true => false
  | false => true
  | end
```

- `true` and `false` are atoms (lowercase)
- Pattern matches exactly

**Usage:**
```
not true   →  false
not false  →  true
not xyz    →  (not xyz)   // no match, doesn't reduce (partial function!)
```

### Identity Function

```
id = 
  | X => X
  | end
```

- `X` is a variable (uppercase)
- Binds to anything

**Usage:**
```
id true   →  true
id false  →  false
id xyz    →  xyz
```

### Boolean AND

```
and = 
  | true X => X
  | false _ => false
  | end
```

**Usage:**
```
and true true   →  true
and true false  →  false
and false true  →  false
and false false →  false
and xyz true    →  (and xyz true)   // no match, doesn't reduce
```

### Modus Ponens

```
mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end
```

- `P` and `Q` are variables (uppercase)
- `imp` and `theorem` are atoms (lowercase)

**Usage:**
```
ax1 = theorem (imp p q)
ax2 = theorem p

mp ax1 ax2  →  theorem q   ✓
```

---

## Partial Functions in Action

### Type-Safe-ish Operations

```
// Head of list (partial function)
head = 
  | (cons X Xs) => X
  | end

head (cons 1 nil)  →  1
head nil           →  (head nil)   // undefined, doesn't reduce
```

### Pattern Matching on Structures

```
// Extract premise from implication
get_premise = 
  | (imp P Q) => P
  | end

get_premise (imp a b)  →  a
get_premise (and a b)  →  (get_premise (and a b))   // wrong structure, undefined
```

---

## Reduction Semantics

### Pattern Match Application

```
(| P1 ... PN => M | rest) V1 ... VN
```

1. Try to match V1 against P1, V2 against P2, ..., VN against PN
2. If all match with bindings σ, reduce to M[σ]
3. If any fail, try `rest` with the same arguments
4. If all cases fail (reach `end`), **don't reduce** (partial function)

### Key Point: No Reduction ≠ Stuck

**Traditional "stuck" term:** Runtime error, undefined behavior

**Logi "no reduction":** Partial function, simply not defined for this input

The term `(head nil)` is a perfectly valid term that just happens to not reduce further. It's not an error.

---

## Theoretical Elegance

### Normal Form Includes Partial Applications

A term is in **normal form** if:
1. It's a value (atom, case expression, theorem value), OR
2. It's an application where no reduction is possible

**Examples of normal forms:**
```
true                    // atom
false                   // atom
| X => X | end         // case expression (value)
theorem p               // theorem value (if p is in normal form)
(head nil)             // partial application (head undefined on nil)
(not xyz)              // partial application (xyz not a boolean)
```

### Composability

Partial functions compose naturally:

```
// These are all valid terms, just not fully reduced
f (g x)                // if g x doesn't reduce, f applied to unreduced term
not (head nil)         // head nil doesn't reduce, not applied to it
mp ax1 (mt ax2 ax3)    // if mt doesn't reduce, mp applied to unreduced term
```

---

## Comparison to Other Approaches

### vs. "Stuck" Semantics (My Original Proposal)

**Old:** Pattern match failure = stuck = error

**New:** Pattern match failure = no reduction = partial function (not an error!)

**Better for logic:** Many operations are naturally partial.

### vs. Total Functions (Coq/Agda)

**Total function languages:** Must prove all functions terminate and are defined everywhere

**Logi:** Functions are partial by default, termination not required

**Trade-off:** More expressive, but user responsible for totality if needed

---

## Practical Implications

### 1. Error Handling

No special error handling needed - partial functions just don't reduce:

```
divide = 
  | X zero => divide_by_zero_undefined
  | X Y => (divide_computed X Y)
  | end

divide 10 0  →  divide_by_zero_undefined   // or doesn't match, stays as is
```

### 2. Pattern Matching Exhaustiveness

Not required! Functions can be intentionally partial:

```
// Only defined for true
if_true = 
  | true => success
  | end

if_true false  →  (if_true false)   // just doesn't reduce
```

### 3. Proving Theorems

When proving theorems, you often have partial functions:

```
// Only proves for implications
mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end

// Undefined for other structures - that's fine!
mp (theorem (and P Q)) ...  →  doesn't reduce
```

---

## Examples with New Convention

### Propositional Logic

```
// Boolean values
true = true
false = false

// Negation
not = 
  | true => false
  | false => true
  | end

// Conjunction
and = 
  | true X => X
  | false _ => false
  | end

// Disjunction
or = 
  | true _ => true
  | false X => X
  | end

// Implication constructor
imp = 
  | P Q => (imp P Q)
  | end
```

### Inference Rules

```
// Modus ponens
mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end

// Modus tollens
mt = 
  | (theorem (imp P Q)) (theorem (not Q)) => theorem (not P)
  | end

// Hypothetical syllogism
hs = 
  | (theorem (imp P Q)) (theorem (imp Q R)) => theorem (imp P R)
  | end

// Conjunction introduction
conj_intro = 
  | (theorem P) (theorem Q) => theorem (and P Q)
  | end

// Conjunction elimination
conj_elim_l = 
  | (theorem (and P Q)) => theorem P
  | end

conj_elim_r = 
  | (theorem (and P Q)) => theorem Q
  | end
```

### Usage

```
// Axioms
ax1 = theorem (imp p q)
ax2 = theorem p

// Derive Q
result = mp ax1 ax2
// → theorem q   ✓

// Try to misuse mp
bad = mp (theorem p) (theorem q)
// → (mp (theorem p) (theorem q))   // doesn't reduce, no match
```

---

## Terminator Options

You mentioned needing a terminator. Options:

1. **`end`** - keyword, clear
2. **`()`** - empty/unit, standard in some languages
3. **`.`** - simple dot
4. **`nil`** - but you said don't call it nil!
5. **`_|_`** - bottom symbol
6. **`∅`** - empty set
7. **`done`** - alternative keyword
8. **`stop`** - another keyword

**My recommendation:** `end` or `()` (most conventional)

For now I'll use `end` in examples.

---

## Arrow Syntax Sugar

Your original notation:
```
not = true -> false ; false -> true
```

**Can be sugar for:**
```
not = 
  | true => false
  | false => true
  | end
```

**Grammar:**
```
p1 -> M1 ; p2 -> M2 ; ... ; pN -> MN
```

**Desugars to:**
```
| p1 => M1
| p2 => M2
| ...
| pN => MN
| end
```

**Both syntaxes allowed!**

---

## Final Core Calculus

### Syntax

```
M ::= atom                   // lowercase identifier
    | Var                    // uppercase identifier (in term position)
    | M M                    // application
    | theorem M              // theorem marker
    | | P+ => M | M         // pattern case with continuation
    | end                    // pattern terminator

P ::= atom                   // atom pattern (lowercase, match exactly)
    | Var                    // variable pattern (uppercase, bind anything)
    | _                      // wildcard (match, don't bind)
    | P P                    // application pattern
    | theorem P              // theorem pattern
```

### Conventions

- **Lowercase:** atoms (`true`, `false`, `imp`, `theorem`)
- **Uppercase:** variables (`X`, `Y`, `P`, `Q`)
- **Wildcard:** `_` (matches anything, doesn't bind)

### Semantics

**Pattern matching application:**
```
(| P1 ... PN => M | rest) V1 ... VN
```

- Try to match V1 against P1, ..., VN against PN
- If all match with bindings σ, reduce to M[σ]
- If any fail, try `rest`
- If no cases match (reached `end`), **no reduction occurs**

**Partial functions:** No match means the function is undefined for that input (not an error).

---

## Comparison: Before vs. After

### Before (My Original)

**Nested braces:**
```
mp = {
  | (theorem (imp p q)) ⇒ {
    | (theorem p) ⇒ theorem q
  }
}
```

**Stuck on failure:**
- Pattern match failure = error/stuck

**Lowercase = variables:**
- Opposite of your preference

### After (Your Design)

**Multi-pattern:**
```
mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end
```

**Partial functions:**
- No match = just doesn't reduce (not an error!)

**Uppercase = variables:**
- Clear, stands out, Prolog-like

---

## Summary of Final Design

1. **Pattern matching for all functions** ✓
2. **Uppercase = variables, lowercase = atoms** ✓
3. **Partial functions natural** (no match = no reduction) ✓
4. **Multi-pattern per case** (clean syntax) ✓
5. **Explicit terminator** (`end`) ✓

**Example:**
```
mp = 
  | (theorem (imp P Q)) (theorem P) => theorem Q
  | end
```

**One line, crystal clear, elegant!**

---

## Questions

1. **Terminator preference?** (I'm using `end` for now)
2. **Do you want arrow syntax as sugar?** (`p -> M ; ...`)
3. **Any other syntax preferences?**

This design is much cleaner! Should I rewrite the formal specification with these conventions?
