# User-Defined Totality Checking (Not Built-In!)

## Key Correction

**Totality checking is NOT a built-in language feature.**

**Users define totality checkers themselves in Logi.**

---

## For Finite Types: Users Write Explicit Enumerators

### Bool Example

**User writes:**
```logi
// User-defined totality checker for Bool
forall_bool = P => (and (P true) (P false))
```

**Usage:**
```logi
// Property: for all X, (and X (not X)) = false
prop = X => (eq (and X (not X)) false)

// Check totality by calling forall_bool
theorem (forall_bool prop)
```

**Execution:**
```logi
forall_bool prop
→ (and (prop true) (prop false))
→ (and (eq (and true (not true)) false) (eq (and false (not false)) false))
→ (and (eq (and true false) false) (eq (and false true) false))
→ (and (eq false false) (eq false false))
→ (and true true)
→ true ✓
```

**Key insight:** User explicitly enumerates all constructors in their checker!

---

## For Infinite Types: Users Write Structural Recursive Proofs

### Nat Example

**Property:** For all N, (plus N zero) = N

**User writes proof directly:**
```logi
proof = zero => true
      | (succ N) => (proof N)
```

**This IS the totality proof!** The structure encodes induction:
- Base case: `zero => true`
- Inductive case: `(succ N) => (proof N)` (recursive call is inductive hypothesis)

**No external checker needed - the proof is self-contained.**

---

## What The Language Provides (Built-In)

**Only the primitives:**
1. Pattern matching
2. Recursion
3. `and`, `or`, `eq` (basic operations)

**Everything else is user-defined!**

---

## More Examples: User-Defined Checkers

### Option/Maybe Type

```logi
// User-defined totality checker
forall_maybe = P => inner_forall =>
    (and 
        (P none)
        (inner_forall (X => (P (some X)))))
```

**Usage:**
```logi
// Property: all maybes are either none or some
prop = M => (or (is_none M) (is_some M))

// Check with user-defined forall_maybe
theorem (forall_maybe prop forall_any_value)
```

### Result Type

```logi
// User-defined checker for Result
forall_result = P => left_forall => right_forall =>
    (and
        (left_forall (E => (P (error E))))
        (right_forall (V => (P (ok V)))))
```

---

## Comparison: What's Built-In vs User-Defined

| Feature | Built-In? | User-Defined? |
|---------|-----------|---------------|
| Pattern matching | ✓ | |
| Recursion | ✓ | |
| `and`, `or`, `eq` | ✓ | |
| `forall_bool` | | ✓ |
| `forall_nat` | | ✓ (through structural recursion) |
| Totality "checking" | | ✓ (users write proofs) |

---

## Why This Is Better

**Advantages of user-defined:**
1. **Minimal language** - no special totality checking logic in core
2. **Flexible** - users can define checkers for their own types
3. **Explicit** - clear what's being checked
4. **Compositional** - checkers are just functions

**The language provides:**
- Pattern matching (to handle different constructors)
- Recursion (for infinite types)
- Basic logic operators

**Users build totality checking on top of these primitives!**

---

## Concrete Example: Full Proof

**Goal:** Prove that for all booleans B, (or B (not B)) = true

**Step 1: User defines the property**
```logi
prop = B => (eq (or B (not B)) true)
```

**Step 2: User uses built-in forall_bool (or defines their own)**
```logi
// Standard library might provide this, but it's just a function:
forall_bool = P => (and (P true) (P false))
```

**Step 3: User constructs the theorem**
```logi
theorem (forall_bool prop)
```

**Step 4: Reduction (happens in the language)**
```logi
forall_bool prop
→ (and (prop true) (prop false))
→ (and 
    (eq (or true (not true)) true)
    (eq (or false (not false)) true))
→ (and
    (eq (or true false) true)
    (eq (or false true) true))
→ (and
    (eq true true)
    (eq true true))
→ (and true true)
→ true ✓
```

**The language just reduces the term - no special totality checking!**

---

## Summary

**What the language provides:**
- Pattern matching
- Recursion  
- Basic operators

**What users write:**
- Totality checkers like `forall_bool`
- Proofs as functions
- Structural recursive proofs for infinite types

**No built-in totality checking - it's all user-defined!** ✓

---

End of Correction.
