# Clean Syntax Design

## Your Three Critiques

1. **Meta-variable ambiguity**: How does system know `p` and `q` are pattern variables?
2. **Syntax bloat**: Nested curly braces for multi-argument functions is horrible
3. **AST terminator**: Need explicit terminator for pattern match lists

All valid. Let me redesign.

---

## Proposed Clean Syntax

### Core Idea: Multi-Pattern Matching

Allow multiple patterns before `=>` for multi-argument functions:

```
| pattern1 pattern2 ... patternN => body
```

This matches N arguments sequentially.

### Terminator

Use `end` as the pattern match terminator (or `()`, `.`, `•` - your choice).

### Full Syntax

```
term ::= 
  | pattern1 ... patternN => body
  | pattern1 ... patternN => body
  | ...
  | end
```

---

## Examples

### Modus Ponens (Clean!)

```
mp = 
  | (theorem (imp p q)) (theorem p) => theorem q
  | end
```

**One line!** Two patterns, one result.

### Boolean NOT

```
not = 
  | T => F
  | F => T
  | end
```

### Boolean AND

```
and = 
  | T T => T
  | T F => F
  | F T => F
  | F F => F
  | end
```

Or with fallthrough:

```
and = 
  | T x => x
  | F _ => F
  | end
```

### Identity

```
id = 
  | x => x
  | end
```

---

## Meta-Variable Question

> "how does the system know that `p` and `q` are metavariables?"

### Option 1: Everything in Patterns is a Pattern Variable

**Rule:** Any identifier in a pattern is a pattern variable (binds).

**For atoms:** Use quoted syntax or uppercase convention.

```
not = 
  | 'true => 'false
  | 'false => 'true
  | end
```

Or uppercase:

```
not = 
  | TRUE => FALSE
  | FALSE => TRUE
  | end
```

### Option 2: Lowercase = Pattern Var, Uppercase = Atom

**Convention:**
- Lowercase: pattern variables (`p`, `q`, `x`)
- Uppercase: atoms (`T`, `F`, `True`, `False`)

```
not = 
  | T => F
  | F => T
  | end

mp = 
  | (theorem (imp p q)) (theorem p) => theorem q
  | end
```

**This is clean and standard (ML/Haskell convention).**

### Option 3: Explicit Binding

Use prefix for pattern variables:

```
mp = 
  | (theorem (imp ?p ?q)) (theorem ?p) => theorem ?q
  | end
```

**More verbose but unambiguous.**

---

## My Recommendation: Option 2 (Convention)

**Use case convention:**
- `T`, `F`, `True`, `False` - atoms (uppercase)
- `p`, `q`, `x`, `y` - pattern variables (lowercase)

**Benefits:**
- Clean syntax (no extra symbols)
- Standard in functional programming
- Easy to read

**Rule:** In patterns, lowercase identifiers bind, uppercase identifiers match atoms.

---

## AST Representation

### Right-Associative Chaining

```
| p1 => M1
| p2 => M2
| p3 => M3
| end
```

**Parsed as:**

```
(case p1 M1 (case p2 M2 (case p3 M3 end)))
```

Right-associative, so we peel off first case first.

### AST Node Structure

```
Case(patterns: [Pattern], body: Term, next: Term)
End
```

**Example:**

```
mp = 
  | (theorem (imp p q)) (theorem p) => theorem q
  | end
```

**AST:**

```
Case(
  patterns: [
    App(Atom("theorem"), App(App(Atom("imp"), Var("p")), Var("q"))),
    App(Atom("theorem"), Var("p"))
  ],
  body: App(Atom("theorem"), Var("q")),
  next: End
)
```

### Terminator Symbol

**Options:**
- `end` - clear, keyword-like
- `()` - empty list/unit type
- `.` - simple dot
- `•` - bullet point
- `_|_` - bottom symbol
- `∅` - empty set

**My recommendation:** `end` (clearest)

---

## Complete Grammar

```
term ::= 
  | caseExpr
  | atom
  | variable
  | (term term)
  | theorem term

caseExpr ::=
  | pattern+ => term caseExpr
  | end

pattern ::=
  | UPPERCASE_ID        // atom pattern
  | lowercase_id        // pattern variable
  | _                   // wildcard
  | (pattern pattern)   // structure pattern
  | theorem pattern     // theorem pattern
```

---

## Full Examples

### Propositional Logic

```
// Atoms
T = T
F = F

// NOT
not = 
  | T => F
  | F => T
  | end

// AND
and = 
  | T x => x
  | F _ => F
  | end

// OR
or = 
  | T _ => T
  | F x => x
  | end

// IMPLIES (as constructor)
imp = 
  | p q => (imp p q)
  | end
```

### Modus Ponens

```
mp = 
  | (theorem (imp p q)) (theorem p) => theorem q
  | end
```

**Usage:**
```
ax1 = theorem (imp P Q)
ax2 = theorem P

result = mp ax1 ax2
// reduces to: theorem Q
```

### Modus Tollens

```
mt = 
  | (theorem (imp p q)) (theorem (not q)) => theorem (not p)
  | end
```

### Hypothetical Syllogism

```
hs = 
  | (theorem (imp p q)) (theorem (imp q r)) => theorem (imp p r)
  | end
```

### Conjunction Introduction

```
conj_intro = 
  | (theorem p) (theorem q) => theorem (and p q)
  | end
```

---

## Comparison to Your Original

**Your notation:**
```
mp = {
  | (theorem (imp p q)) ⇒ {
    | (theorem p) ⇒ theorem q
  }
}
```

**Clean syntax:**
```
mp = 
  | (theorem (imp p q)) (theorem p) => theorem q
  | end
```

**Improvements:**
- ✅ No nested braces
- ✅ One line
- ✅ Clear terminator
- ✅ Multiple patterns per case
- ✅ Meta-variables via convention

---

## Semantics

### Evaluation

```
(| p1 p2 ... pN => M | rest) V1 V2 ... VN
```

1. Try to match V1 against p1, V2 against p2, etc.
2. If all match with bindings σ, return M[σ]
3. If any fail, try `rest` with same arguments

### With Terminator

```
(| p1 ... pN => M | end) V1 ... VN
```

If match fails, STUCK (no more cases).

---

## Alternative: No Terminator?

**Could we infer the end?**

If we require pattern matches to be saturated (all arguments provided), we could detect when to stop:

```
mp = 
  | (theorem (imp p q)) (theorem p) => theorem q
```

No explicit `end` needed - we know this is a 2-argument function.

**But:** For type-checking/static analysis, explicit terminator is clearer.

**Also:** For zero-argument constants, terminator distinguishes:

```
true = 
  | () => T
  | end

// vs.

true = T   // just an alias
```

**Recommendation:** Keep explicit terminator for clarity.

---

## Syntactic Sugar

### Lambda Notation

If you want `λ` syntax as sugar:

```
λp1 p2 ... pN. M   ≡   | p1 p2 ... pN => M | end
```

**Example:**
```
add = λx y. x + y
```

Desugars to:
```
add = 
  | x y => x + y
  | end
```

### Single-Pattern Short Form

For single-pattern functions:

```
id = | x => x | end
```

Could be written:
```
id x = x
```

With pattern before `=` as sugar.

---

## Your Original Arrow Notation

From DOC 1:
```
not = true -> false ; false -> true
```

**This could be sugar for:**

```
not = 
  | true => false
  | false => true
  | end
```

**Arrow + semicolon syntax:**
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

**This gives you your preferred notation!**

---

## Putting It All Together

### Core Syntax

```
not = 
  | T => F
  | F => T
  | end
```

### With Arrow Sugar

```
not = T -> F ; F -> T
```

**Both allowed, same semantics!**

---

## Final Recommendation

### Core Calculus

```
term ::= 
  | variable
  | ATOM
  | (term term)
  | theorem term
  | | pattern+ => term caseExpr

caseExpr ::=
  | | pattern+ => term caseExpr
  | end
```

### Conventions

- **Uppercase:** atoms (`T`, `F`, `TRUE`, `FALSE`)
- **Lowercase:** pattern variables (`p`, `q`, `x`, `y`)
- **Terminator:** `end` (or your preferred symbol)

### Syntax Sugar

```
λx. M          ≡  | x => M | end
p -> M ; rest  ≡  | p => M | rest
```

---

## Addressing Your Concerns

1. **Meta-variables:** Convention (lowercase binds, uppercase matches)
2. **Syntax bloat:** Multi-pattern per case (one line!)
3. **Terminator:** Explicit `end` (or your preferred symbol)

**Result:** Clean, minimal, unambiguous syntax.

---

## What Terminator Symbol Do You Prefer?

- `end` - keyword style
- `()` - unit/empty
- `.` - simple dot
- `•` - bullet
- `_|_` - bottom
- `∅` - empty set
- Something else?

**Let me know and I'll update the formal specification with this clean syntax!**
