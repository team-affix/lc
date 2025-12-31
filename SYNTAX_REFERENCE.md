# Logi: Syntax Reference Card

## Core Syntax

```logi
// 1. Atoms (lowercase)
true, false, nil, zero, imp

// 2. Variables (uppercase)  
X, Y, P, Q, Head, Tail

// 3. Application (left-associative)
f x y  =  ((f x) y)

// 4. Function with =>
not = true => false | false => true

// 5. Pipe | is binary infix
f = case1 => body1 | case2 => body2

// 6. Theorem marker
axiom = theorem (imp p q)

// 7. Quotation (curly braces prevent reduction)
term = {not true}
```

---

## The 7 Constructs

```
M ::= atom           // lowercase: true, false
    | Var            // uppercase: X, Y
    | M M            // application
    | pattern => M   // function (=> not ->)
    | M | M          // choice (binary infix)
    | theorem M      // theorem marker
    | {M}            // quotation
```

---

## Examples

### Boolean Logic

```logi
not = true => false | false => true

and = true => (X => X) | false => (_ => false)

or = true => (_ => true) | false => (X => X)
```

### Currying (One Pattern Per Arrow)

```logi
// CORRECT: curried
add = X => Y => plus X Y

// WRONG: multi-pattern (old rejected syntax)
add = X Y => plus X Y  ✗
```

### Lists

```logi
nil = nil
cons = X => Xs => (cons X Xs)

head = (cons X Xs) => X

length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

### Inference Rules

```logi
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)
```

### Quotation

```logi
// Build unreduced term
term = {not true}        // stays as {not true}

// Extract via pattern matching
extract = {X} => X

result = extract {not true}
// → not true → false
```

---

## Precedence (Low to High)

```
|      (choice - binary infix, right-associative)
=>     (function - right-associative)
app    (application - left-associative)
```

**Examples:**
```logi
a => b | c => d    =  (a => b) | (c => d)
X => Y => Z        =  X => (Y => Z)
f x y              =  (f x) y
```

---

## Common Mistakes

### ❌ Using `->` instead of `=>`

```logi
not = true -> false  ✗
```

✅ **Correct:**
```logi
not = true => false  ✓
```

### ❌ Old brace-pipe syntax

```logi
not = { | true => false | false => true }  ✗
```

✅ **Correct: pipe is binary infix**
```logi
not = true => false | false => true  ✓
```

### ❌ Curly braces for pattern matching

```logi
id = { X => X }  ✗
```

✅ **Correct: curly braces are ONLY for quotation**
```logi
id = X => X                ✓  (function)
term = {not true}          ✓  (quotation)
```

### ❌ Multiple patterns before arrow

```logi
add = X Y => plus X Y  ✗
```

✅ **Correct: curry**
```logi
add = X => Y => plus X Y  ✓
```

---

## Pattern Syntax

```logi
p ::= atom           // match atom exactly: true, false
    | Var            // bind to variable: X, Y
    | _              // wildcard (match, don't bind)
    | (p p)          // application pattern: (cons X Xs)
    | theorem p      // theorem pattern: (theorem P)
    | {p}            // quoted pattern: {X}
```

**Examples:**
```logi
// Match atom
true => false

// Bind variable
X => X

// Wildcard
_ => false

// Structure pattern
(cons X Xs) => X

// Theorem pattern
(theorem P) => P

// Quoted pattern
{X} => X
```

---

## Complete Example

```logi
// Implication constructor
imp = P => Q => {imp P Q}

// Negation constructor
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

## Summary

**Arrow:** Use `=>` (not `->`)  
**Pipe:** Binary infix `|` (not prefix with braces)  
**Curly braces:** Quotation `{M}` only (not for pattern matching)  
**Multiple args:** Curry with `X => Y => body` (not `X Y => body`)

**For more details:** See [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)

