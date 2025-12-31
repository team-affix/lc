# Constructors vs. Quotation in Logi

## Two Ways to Build Non-Reducing Terms

### Method 1: Atom Constructors (No Quotation Needed)

**Pattern:** Use atoms as constructor tags

```logi
// Define constructor function
imp = P => Q => (imp P Q)
```

Here:
- `imp` on LHS is the function name
- `imp` on RHS inside `(imp P Q)` is an **atom** (not a function!)
- `(imp P Q)` is an application of atom `imp` to arguments
- Since atom `imp` is not a function, **it doesn't reduce**

**This works because atoms applied to arguments just stay as-is:**

```logi
imp p q
= (P => Q => (imp P Q)) p q
= (Q => (imp p Q)) q
= (imp p q)  ✓  (atom 'imp' applied to two args - doesn't reduce further)
```

**Use for:** Propositions, data constructors, any time you want simple tagged structures

**Examples:**
```logi
// Propositional constructors
imp = P => Q => (imp P Q)
neg = P => (neg P)
and = P => Q => (and P Q)

// Data constructors
cons = X => Xs => (cons X Xs)
pair = X => Y => (pair X Y)
some = X => (some X)

// Usage
prop = imp (neg p) q     // → (imp (neg p) q) ✓
list = cons 1 (cons 2 nil)  // → (cons 1 (cons 2 nil)) ✓
```

### Method 2: Quotation (Explicit Freezing)

**Pattern:** Use curly braces to explicitly prevent reduction

```logi
// Define constructor with quotation
imp = P => Q => {imp P Q}
```

Now:
- `{imp P Q}` is **explicitly quoted**
- Nothing inside the braces will reduce
- Even if `P` or `Q` themselves would reduce

**Use for:** 
- Meta-programming (building syntax trees)
- Preventing unwanted reduction
- Symbolic computation

**Examples:**
```logi
// Symbolic constructors
plus_sym = X => Y => {plus X Y}
mul_sym = X => Y => {mul X Y}

// Build symbolic expression (nothing evaluates)
expr = plus_sym (mul_sym x two) x
// → {plus {mul x two} x}  ✓

// Even if x is defined, it won't evaluate inside quotes
```

---

## Key Difference

### Without Quotation: Arguments Reduce Before Passing

```logi
imp = P => Q => (imp P Q)

ax = imp (not true) q
```

**Reduction:**
```
imp (not true) q
= (P => Q => (imp P Q)) (not true) q
// First, reduce (not true) → false
= (P => Q => (imp P Q)) false q
= (Q => (imp false Q)) q
= (imp false q)  ✓
```

**The argument `(not true)` reduced to `false` before being passed to `imp`!**

### With Quotation: Arguments Don't Reduce Inside

```logi
imp = P => Q => {imp P Q}

ax = imp (not true) q
```

**Reduction:**
```
imp (not true) q
= (P => Q => {imp P Q}) (not true) q
// First, reduce (not true) → false
= (P => Q => {imp P Q}) false q
= (Q => {imp false Q}) q
= {imp false q}  ✓
```

Wait, the argument still reduced before being passed!

To prevent reduction of the argument, we need to quote it at the call site:

```logi
ax = imp {not true} q
```

**Reduction:**
```
imp {not true} q
= (P => Q => {imp P Q}) {not true} q
// {not true} is a value, doesn't reduce
= (Q => {imp {not true} Q}) q
= {imp {not true} q}  ✓
```

**Now the `not true` is preserved unreduced!**

---

## When to Use Which

### Use Atom Constructors When:

✅ Building simple tagged data structures
✅ Arguments will be values (atoms, already reduced)
✅ You want normal evaluation before construction
✅ Propositions with atomic variables

**Example:**
```logi
imp = P => Q => (imp P Q)

// If p and q are atoms, this works fine
ax1 = theorem (imp p q)  // → theorem (imp p q) ✓
```

### Use Quotation When:

✅ Building syntax trees with unreduced subterms
✅ Meta-programming (term manipulation)
✅ Symbolic computation
✅ Need to prevent reduction explicitly

**Example:**
```logi
imp = P => Q => {imp P Q}

// Even if P or Q are complex expressions, they stay unreduced
ax1 = theorem (imp {not (not p)} q)
// → theorem {imp {not (not p)} q} ✓
```

Or quote at call site:
```logi
imp = P => Q => (imp P Q)
ax1 = theorem (imp {not (not p)} q)
// → theorem (imp {not (not p)} q) ✓
```

---

## Recommended Convention

### For Propositional Logic: Atom Constructors

```logi
// Constructors (no quotation)
imp = P => Q => (imp P Q)
neg = P => (neg P)
and = P => Q => (and P Q)

// Axioms with atoms
ax1 = theorem (imp p q)
ax2 = theorem (neg q)

// If you need to preserve structure, quote at call site
ax3 = theorem (imp {not (not p)} q)
```

**Why:** Simpler, less quotation noise for common case (atomic props)

### For Symbolic Computation: Explicit Quotation

```logi
// Symbolic constructors
plus = X => Y => {plus X Y}
mul = X => Y => {mul X Y}

// Build expressions
expr = plus (mul {x} two) {x}
// → {plus {mul {x} two} {x}}
```

**Why:** Explicit control, clear that nothing evaluates

---

## Summary

**Two approaches:**

1. **Atom constructors:** `imp = P => Q => (imp P Q)`
   - Simple, less syntax
   - Arguments evaluate before construction
   - Good for propositions with atomic arguments

2. **Quoted constructors:** `imp = P => Q => {imp P Q}`
   - Explicit, prevents all reduction
   - Arguments stay unreduced
   - Good for symbolic/meta-programming

**Or mix:** Atom constructors + quote at call site when needed:
```logi
imp = P => Q => (imp P Q)
ax = theorem (imp {complex expr} q)
```

**Choose based on your use case!** Both are valid.

---

## In the Standard Library

**Recommendation:** Use atom constructors for simplicity:

```logi
// Standard library (EXAMPLES.md, FORMAL_SPECIFICATION.md)
imp = P => Q => (imp P Q)
neg = P => (neg P)
and = P => Q => (and P Q)

// Users quote when needed
ax = theorem (imp {not (not p)} q)
```

This keeps the common case (atomic props) clean and allows flexibility when needed.

---

End of Constructor Conventions.
