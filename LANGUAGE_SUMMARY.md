# Logi Language: Complete Syntax Summary

## The Language in One Page

### Core Syntax (7 Constructs)

```
M ::= atom           // lowercase: true, false, imp
    | Var            // uppercase: X, Y, P, Q
    | M M            // application (left-assoc)
    | pattern => M   // function (right-assoc, one pattern per arrow)
    | M | M          // choice (right-assoc, binary infix)
    | theorem M      // theorem marker
    | {M}            // quotation (prevent reduction)
```

### Patterns

```
p ::= atom           // match atom exactly
    | Var            // bind anything to Var
    | _              // wildcard (match, don't bind)
    | (p p)          // application pattern
    | theorem p      // theorem pattern
    | {p}            // quoted pattern
```

---

## Key Design Principles

1. **One pattern per arrow** - Curry for multiple args: `X => Y => body`
2. **Uppercase = variables, lowercase = atoms** - Case convention
3. **Binary pipe `|`** - No end token needed
4. **Partial functions natural** - No match = no reduction (not error)
5. **Quotation `{M}`** - Prevent reduction, allow substitution

---

## Examples

### Boolean Logic

```logi
not = true => false | false => true
and = true => (X => X) | false => (_ => false)
```

### Inference Rules

```logi
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)
```

### Quotation

```logi
// Build unreduced term
term = {not true}        // → {not true} (doesn't reduce)

// Extract via pattern matching
extract = {X} => X
result = extract {not true}  // → not true → false
```

### Lists

```logi
nil = nil
cons = X => Xs => (cons X Xs)
head = (cons X Xs) => X
length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

### Quantifiers (HOAS)

```logi
forall = P => (forall P)
ui = (theorem (forall P)) => T => theorem (P T)
```

---

## Precedence (Low to High)

```
|      (choice)
=>     (function)
app    (application)
```

---

## Semantics Summary

### Values

```
V ::= atom | (pattern => M) | (M | M) | theorem V | {M}
```

### Reduction

- **Application:** `(p => M) V → M[σ]` if `match(p, V) = σ`
- **Choice:** Try left, if fails try right
- **Quotation:** `{M}` is a value, never reduces
- **Partial functions:** No match = no reduction

### Substitution

- Penetrates all constructs including `{M}`
- Capture-avoiding
- `{M}[σ] = {M[σ]}`

### Pattern Matching

- Variable patterns bind
- Atom patterns match exactly
- Wildcard matches anything
- Application patterns recurse
- `{p}` matches `{V}` and extracts `V`

---

## Complete Example

```logi
// Constructors
imp = P => Q => {imp P Q}
neg = P => {neg P}

// Axioms
ax1 = theorem {imp p q}
ax2 = theorem {neg q}

// Modus Tollens
mt = (theorem {imp P Q}) => (theorem {neg Q}) => theorem {neg P}

// Derive ¬P
result = mt ax1 ax2
// → theorem {neg p}  ✓
```

---

## Implementation Checklist

### Core (Required)

- [x] Lexer (atoms, vars, `=>`, `|`, `{}`, `()`, `theorem`)
- [x] Parser (precedence: app > => > |)
- [x] Pattern matcher
- [x] Evaluator (small-step reduction)
- [x] Substitution (capture-avoiding)
- [x] REPL

### Standard Library

- [x] Boolean logic (not, and, or)
- [x] Propositional rules (mp, mt, hs, etc.)
- [x] Quantifiers (forall, exists with HOAS)
- [x] Lists (cons, nil, head, tail, length)
- [x] Natural numbers (zero, succ, add, mul)

---

## Quick Reference

```logi
// Atoms and variables
true, false, nil         // atoms (lowercase)
X, Y, P, Q              // variables (uppercase)

// Functions
X => body               // one argument
X => Y => body          // two arguments (curried)

// Pattern alternatives
true => a | false => b  // two cases

// Application
f x y  =  (f x) y       // left-associative

// Theorem
theorem M               // mark as proposition

// Quotation
{M}                     // prevent reduction
{X} => body            // pattern match on quote

// Structured patterns
(cons X Xs) => body    // list pattern
(theorem P) => body    // theorem pattern
```

---

## Status

- **Theory:** ✅ Complete
- **Specification:** ✅ Complete
- **Examples:** ✅ Complete
- **Implementation guide:** ✅ Complete
- **Ready for implementation:** ✅ YES

**Estimated implementation time:** 2-3 weeks for core system (~1000 LOC)

---

End of Summary.
