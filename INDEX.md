# Logi Language Documentation - Complete Index

## 🚨 Read This First!

**Many files in this repository show outdated or rejected syntax from the design exploration process.**

**Only use the files marked ✅ CURRENT below!**

---

## ✅ CURRENT DOCUMENTATION (Correct Syntax)

### Essential Reading (In Order)

1. **[START_HERE.md](START_HERE.md)** - Start point with navigation ⭐
2. **[SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md)** - Quick syntax card (5 min) ⭐⭐⭐
3. **[QUICKSTART.md](QUICKSTART.md)** - 10-minute introduction ⭐⭐
4. **[LANGUAGE_SUMMARY.md](LANGUAGE_SUMMARY.md)** - One-page summary (5 min) ⭐
5. **[FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)** - Complete formal semantics ⭐⭐⭐
6. **[EXAMPLES.md](EXAMPLES.md)** - Worked examples with traces ⭐⭐
7. **[FINAL_SUMMARY.md](FINAL_SUMMARY.md)** - Design evolution and roadmap ⭐

### Specialized Documentation

8. **[IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md)** - Python implementation skeleton
9. **[QUOTATION_CURLY.md](QUOTATION_CURLY.md)** - Quotation deep dive
10. **[USE_CASES.md](USE_CASES.md)** - Use cases beyond logic
11. **[DOCUMENTATION_STATUS.md](DOCUMENTATION_STATUS.md)** - File status guide
12. **[README_THEORY.md](README_THEORY.md)** - Alternative index

---

## 📚 Historical/Design Evolution Files (May Show Rejected Syntax)

**⚠️ Warning:** These files document the design process and may show syntax that was explored and then REJECTED. They are useful for understanding WHY we made certain decisions, but do NOT learn syntax from them!

### Design Exploration (Correct to Read After Understanding Current Design)

- **RECONSIDERING.md** - Why lambda ≠ pattern (critical insight)
- **REVISED_THEORY.md** - Alternative approaches explored
- **ARROW_SEMANTICS.md** - Context-dependent arrow (rejected)
- **CORRECTED_SYNTAX.md** - Why one pattern per arrow
- **CLEAN_SYNTAX.md** - Multi-pattern exploration (rejected)
- **FINAL_DESIGN.md** - Partial functions design (pre-quotation)

**Read these ONLY after understanding the current design from the files above.**

---

## Correct Syntax (The Truth™)

### Arrow Operator

```logi
✅ CORRECT: =>
not = true => false | false => true

❌ WRONG: ->
not = true -> false  ✗
```

### Pipe Operator

```logi
✅ CORRECT: binary infix |
not = true => false | false => true

❌ WRONG: prefix with braces { | ... }
not = { | true => false | false => true }  ✗
```

### Curly Braces

```logi
✅ CORRECT: quotation (prevent reduction)
term = {not true}

❌ WRONG: pattern matching
not = { T => F }  ✗
```

### Multiple Arguments

```logi
✅ CORRECT: curry (one pattern per arrow)
add = X => Y => plus X Y

❌ WRONG: multi-pattern
add = X Y => plus X Y  ✗
```

### Case Convention

```logi
✅ CORRECT:
- Uppercase = variables (P, Q, X, Y)
- lowercase = atoms (true, false, imp)

❌ WRONG: opposite convention
```

---

## The 7 Constructs

1. **Atoms** - `true`, `false` (lowercase)
2. **Variables** - `X`, `Y` (uppercase)
3. **Application** - `f x y`
4. **Function** - `pattern => body`
5. **Choice** - `case1 | case2` (binary infix)
6. **Theorem** - `theorem M`
7. **Quotation** - `{M}` (prevent reduction)

---

## Quick Examples (All Correct Syntax)

```logi
// Boolean NOT
not = true => false | false => true

// Curried AND
and = true => (X => X) | false => (_ => false)

// Modus Ponens
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Quotation
term = {not true}           // stays as {not true}
extract = {X} => X          // pattern match on quote
result = extract term       // → not true → false

// Lists
cons = X => Xs => (cons X Xs)
head = (cons X Xs) => X
length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

---

## Reading Paths

### Path 1: Learn the Language (1 hour)

1. [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md) (5 min) ⭐⭐⭐
2. [QUICKSTART.md](QUICKSTART.md) (15 min)
3. [LANGUAGE_SUMMARY.md](LANGUAGE_SUMMARY.md) (5 min)
4. [EXAMPLES.md](EXAMPLES.md) - Skim examples 1-5 (30 min)

### Path 2: Implement the Language (4 hours)

1. [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md) (5 min)
2. [QUICKSTART.md](QUICKSTART.md) (15 min)
3. [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md) (2 hours)
4. [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md) (1.5 hours)

### Path 3: Deep Understanding (8+ hours)

1. Read all ✅ CURRENT files in order
2. Then read historical files to understand design evolution
3. Work through all examples by hand

---

## What's What

### ✅ Use These Files

**For learning:**
- SYNTAX_REFERENCE.md (authoritative syntax)
- QUICKSTART.md (introduction)
- EXAMPLES.md (see it in action)

**For implementing:**
- FORMAL_SPECIFICATION.md (complete semantics)
- IMPLEMENTATION_GUIDE.md (code skeleton)

**For understanding:**
- FINAL_SUMMARY.md (design evolution - shows old vs. new)
- USE_CASES.md (broader applications)

### 📚 Historical Reference Only

**Design exploration files:**
- Show rejected syntax
- Explain why decisions were made
- Useful AFTER learning the current language
- Do NOT learn syntax from these!

---

## Common Questions

### Q: Which arrow do I use?

**A: Use `=>` (not `->` or `→`)**

```logi
f = X => body  ✓
```

### Q: How do I write multi-argument functions?

**A: Curry with nested arrows (one pattern per arrow)**

```logi
add = X => Y => plus X Y  ✓
```

### Q: What are curly braces for?

**A: Quotation ONLY (preventing reduction)**

```logi
{not true}  // Freezes reduction
```

### Q: How do I write case analysis?

**A: Use binary pipe operator `|`**

```logi
not = true => false | false => true  ✓
```

### Q: What's the difference between `p` and `P`?

**A: Case convention**

- `p` = atom (lowercase)
- `P` = variable (uppercase)

---

## Summary

**Start here:** [START_HERE.md](START_HERE.md)

**Syntax reference:** [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md)

**Full spec:** [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)

**⚠️ Many files show rejected syntax - be careful which ones you read!**

---

End of Index.
