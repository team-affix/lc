# Quotation with Curly Braces

## Syntax

**Use curly braces to prevent reduction:**

```logi
{M}    // Quoted term - will not reduce
```

**Curly braces guard the term from reduction, but NOT from substitution.**

---

## Semantics

### 1. Reduction is Prevented

```logi
{not true}    // Does NOT reduce to {false}
```

Inside `{...}`, no beta-reduction occurs.

### 2. Substitution Still Applies

```logi
f = X => {not X}

f true
= {not X}[X := true]
= {not true}  ✓
```

Substitution is capture-avoiding as usual.

### 3. Lifting Applies

When moving quoted terms under binders, lifting still occurs:

```logi
g = {X}    // Free variable X

h = Y => (g Y)
// When substituting g, X needs to be lifted if there are binders
```

---

## Examples

### Example 1: Building Unreduced Terms

```logi
// Without braces - reduces
term1 = (not true)
// → false  ✗

// With braces - doesn't reduce
term2 = {not true}
// → {not true}  ✓
```

### Example 2: Substitution Works

```logi
build_neg = X => {not X}

term = build_neg true
// → {not X}[X := true]
// → {not true}  ✓
```

### Example 3: Pattern Matching on Quoted Terms

```logi
// Extract from quoted term
extract = {X} => X

result = extract {not true}
// → not true  (now can reduce)
// → false  ✓
```

### Example 4: Meta-Programming

```logi
// Get operator of a term
get_op = {(F Arg)} => F

op = get_op {not true}
// → not  ✓
```

### Example 5: Nested Quotes

```logi
nested = {{not true}}
// → {{not true}}  (both levels frozen)

extract_once = {X} => X

once = extract_once nested
// → {not true}  (one level removed)

twice = extract_once once
// → not true  (fully extracted, can now reduce)
// → false
```

---

## Pattern Syntax

**Patterns can also use curly braces:**

```logi
p ::= ...
    | {p}    // quoted pattern
```

**Matches quoted terms:**

```logi
f = {X} => X           // Matches any quoted term
g = {(not X)} => X     // Matches quoted negations
```

---

## Formal Semantics

### Extended Grammar

```
Terms:
  M ::= atom
      | Var
      | M M
      | pattern => M
      | M | M
      | theorem M
      | {M}              // quotation

Patterns:
  p ::= atom
      | Var
      | _
      | (p p)
      | theorem p
      | {p}              // quoted pattern
```

### Values

```
V ::= atom
    | pattern => M
    | M | M
    | theorem V
    | {M}              // quoted term is a value
```

### Reduction Rules

**Quotation never reduces:**

```
{M}  is a value (no reduction rule)
```

**Inside quotation, no reduction:**

```
// This does NOT reduce
{not true}  ⟶  (no reduction)

// NOT even to:
{false}  ✗
```

### Substitution

**Substitution penetrates quotes:**

```
{M}[σ] = {M[σ]}
```

**Example:**

```
{not X}[X := true] = {not true}
```

### Pattern Matching

```
match({M}, {p}) = match(M, p)
match({M}, p)   = FAIL  (if p is not a quoted pattern)
```

---

## Comparison to Other Constructs

### vs. Regular Terms

```logi
not true        // Reduces to: false
{not true}      // Stays as: {not true}
```

### vs. Atoms

```logi
myatom          // An atom (value)
{myatom}        // Quoted atom (also value, but explicitly marked)
```

### vs. Theorem

```logi
theorem M       // Marks M as a proposition (M can reduce inside)
{M}             // Freezes M (M cannot reduce inside)
```

---

## Use Cases

### 1. Building Syntax Trees

```logi
// Build unreduced propositions
prop = {imp (not P) Q}
// → {imp (not P) Q}  ✓
```

### 2. Meta-Theorems

```logi
// "not true reduces to false"
reduction_fact = theorem (reduces {not true} {false})
```

### 3. Term Inspection

```logi
// Check if term is a negation
is_negation = {(not _)} => true | _ => false

is_negation {not p}     // → true
is_negation {imp p q}   // → false
```

### 4. Substitution Control

```logi
// Want to substitute but not reduce
f = X => {imp X (not X)}

result = f p
// → {imp p (not p)}  ✓  (substituted but not reduced)
```

---

## Implementation

### AST Extension

```python
@dataclass
class Quoted:
    inner: Term

Term = Union[Atom, Var, App, Function, Choice, Theorem, Quoted]
```

### Pattern Extension

```python
@dataclass
class PatQuoted:
    inner: Pattern

Pattern = Union[PatAtom, PatVar, PatWild, PatApp, PatTheorem, PatQuoted]
```

### Evaluator Update

```python
def reduce_step(term: Term) -> Term | None:
    # ... existing cases ...
    
    if isinstance(term, Quoted):
        # Quoted terms are values, never reduce
        return None
    
    # Don't try to reduce inside quotes
    # (This is automatic - we only reduce at application, which checks for values)
```

### Substitution Update

```python
def substitute(term: Term, bindings: dict) -> Term:
    # ... existing cases ...
    
    elif isinstance(term, Quoted):
        # Substitution DOES penetrate quotes
        return Quoted(substitute(term.inner, bindings))
```

### Pattern Matching Update

```python
def match_pattern(pattern: Pattern, value: Term) -> dict | None:
    # ... existing cases ...
    
    elif isinstance(pattern, PatQuoted):
        if isinstance(value, Quoted):
            return match_pattern(pattern.inner, value.inner)
        else:
            return None
```

### Lexer Update

```python
# Tokenize { and }
if self.current_char() == '{':
    self.advance()
    return Token(TokenType.LBRACE, '{', line, col)

if self.current_char() == '}':
    self.advance()
    return Token(TokenType.RBRACE, '}', line, col)
```

### Parser Update

```python
def parse_atom_term(self) -> Term:
    # ... existing cases ...
    
    elif token.type == TokenType.LBRACE:
        self.advance()
        inner = self.parse_term()
        self.expect(TokenType.RBRACE)
        return Quoted(inner)
```

---

## Precedence Note

**Curly braces have highest precedence** (like parentheses):

```logi
{f X}           = {(f X)}           // Quote the application
{X} Y           = ({X}) Y           // Quoted term applied to Y
theorem {M}     = theorem ({M})     // Theorem of quoted term
```

---

## Pretty Printing

```python
def pretty_print(term: Term) -> str:
    # ... existing cases ...
    
    elif isinstance(term, Quoted):
        return f"{{{pretty_print(term.inner)}}}"
```

**Example output:**
```
{not true}
{imp p q}
{{nested}}
```

---

## Complete Examples

### Example A: Building Propositions

```logi
// Implication constructor
imp = P => Q => {imp P Q}

// Build: p → q
prop = imp {p} {q}
// → {imp {p} {q}}  ✓
```

### Example B: Meta-Level Equality

```logi
// Check if two quoted terms are "the same" syntactically
same_syntax = X => Y => {eq X Y}

check = same_syntax {not p} {not p}
// → {eq {not p} {not p}}  ✓
```

### Example C: Proof About Reduction

```logi
// State that not true reduces to false
reduction_theorem = theorem (reduces {not true} {false})
```

### Example D: Tactic System

```logi
// Apply a tactic to a quoted goal
simplify_double_neg = {not (not P)} => {P}

simplified = simplify_double_neg {not (not (imp p q))}
// → {imp p q}  ✓
```

---

## Edge Cases

### Quoted Variables

```logi
{X}    // Quoted variable (just a quoted variable, not special)

f = {X}
// X is FREE in the quoted term

g = Y => {X}
// X is still FREE (not bound by Y)
```

### Quotes in Patterns

```logi
// Match any quoted term
extract = {X} => X

// Match quoted application
get_func = {(F Arg)} => F

// Match quoted negation
is_neg = {(not _)} => true | _ => false
```

### Multiple Quotes

```logi
{{M}}    // Doubly quoted

extract = {X} => X

extract {{M}}
// → {M}  (one level removed)
```

---

## Summary

### Syntax

```logi
{M}    // Prevent reduction
```

### Semantics

1. **No reduction inside:** `{not true}` stays as `{not true}`
2. **Substitution applies:** `{not X}[X := true]` becomes `{not true}`
3. **Lifting applies:** Standard lambda calculus rules
4. **Pattern matching:** `{p}` matches `{V}` and extracts `V`

### Use Cases

- Building unreduced syntax trees
- Meta-programming (term inspection)
- Proving properties about reduction
- Controlling evaluation in tactics

### Implementation

- Add `Quoted` AST node
- Add `PatQuoted` pattern node
- Treat as value (no reduction)
- Substitute through quotes
- ~50 lines of additional code

**This is minimal, clean, and gives you full control when needed!** ✓

---

End of Quotation Specification.
