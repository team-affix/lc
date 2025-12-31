# Logi Documentation Status

## Correct Syntax Reference

### Arrows
**Use `=>` (not `->`):**
```logi
not = true => false | false => true
```

### Pipe Operator
**Binary infix operator `|` (not prefix with braces):**
```logi
// CORRECT: pipe is binary infix
not = true => false | false => true

// WRONG: old syntax with braces and prefix pipe
not = { | true => false | false => true }  ✗
```

### Curly Braces
**For quotation only (preventing reduction):**
```logi
// CORRECT: quotation
term = {not true}        // Prevents reduction

// WRONG: old syntax for pattern matching
not = { | T => F }  ✗
```

### One Pattern Per Arrow
**Must curry for multiple arguments:**
```logi
// CORRECT: curried
add = X => Y => plus X Y

// WRONG: multi-pattern (old rejected syntax)
add = X Y => plus X Y  ✗
```

---

## Current Documentation Files (✅ Use These)

### Core Documentation (Correct Syntax)

1. **QUICKSTART.md** ✅
   - 10-minute introduction
   - Correct syntax throughout
   - Use this for learning

2. **FORMAL_SPECIFICATION.md** ✅
   - Complete formal semantics
   - Authoritative reference
   - Correct syntax

3. **EXAMPLES.md** ✅
   - Worked examples with full reduction traces
   - All syntax correct

4. **LANGUAGE_SUMMARY.md** ✅
   - One-page quick reference
   - Correct syntax

5. **IMPLEMENTATION_GUIDE.md** ✅
   - Complete implementation skeleton
   - Python code examples
   - Correct syntax

6. **FINAL_SUMMARY.md** ✅ (with caveats)
   - Design evolution
   - Shows old syntax when explaining what was rejected
   - Final recommendations use correct syntax

7. **README.md** ✅
   - Main project readme
   - Correct syntax

8. **README_THEORY.md** ✅
   - Documentation index
   - Correct syntax

9. **QUOTATION_CURLY.md** ✅
   - Complete quotation specification
   - Correct `{M}` syntax

10. **USE_CASES.md** ✅
    - Broader use cases
    - Correct syntax

---

## Design Exploration Files (Historical Reference)

These files document the design process and show syntax that was REJECTED. They are kept for historical understanding but should NOT be used for learning the language.

### Mark as "Historical - Outdated Syntax"

1. **RECONSIDERING.md** 📚
   - Shows old `{ | p => M }` syntax (REJECTED)
   - Explains why equivalence was questioned
   - Keep for understanding design evolution

2. **REVISED_THEORY.md** 📚
   - Explores alternative approaches
   - May use old/experimental syntax
   - Keep for reference

3. **CORRECTED_SYNTAX.md** 📚
   - Explains why one pattern per arrow
   - Mostly correct but transitional

4. **CLEAN_SYNTAX.md** 📚
   - Multi-pattern exploration (REJECTED)
   - Keep for design rationale

5. **FINAL_DESIGN.md** 📚
   - Partial functions discussion
   - Check for syntax errors

6. **ARROW_SEMANTICS.md** 📚
   - Context-dependent arrow exploration (REJECTED)
   - Keep for design rationale

---

## Deleted Files (Outdated)

✅ QUICK_START.md - Deleted (old syntax)  
✅ FORMAL_SPEC.md - Deleted (superseded by FORMAL_SPECIFICATION.md)  
✅ PROOF_EXAMPLES.md - Deleted (superseded by EXAMPLES.md)  
✅ INDEX.md - Deleted (superseded by README_THEORY.md)  
✅ RECOMMENDATIONS.md - Deleted (superseded by FINAL_SUMMARY.md)  
✅ SUMMARY_FOR_USER.md - Deleted (outdated)  
✅ THEORY.md - Deleted (superseded by current docs)  
✅ CRITICAL_ANALYSIS.md - Deleted (outdated)  
✅ QUOTATION.md - Deleted (superseded by QUOTATION_CURLY.md)  
✅ FINAL_CLEAN_DESIGN.md - Deleted (had multi-pattern syntax errors)  

---

## Reading Order for New Users

1. **QUICKSTART.md** (10 min)
2. **LANGUAGE_SUMMARY.md** (5 min)
3. **FORMAL_SPECIFICATION.md** (1-2 hours)
4. **EXAMPLES.md** (1-2 hours)
5. **IMPLEMENTATION_GUIDE.md** (if implementing)

**Do NOT read the "Historical Reference" files first** - they show rejected syntax!

---

## Syntax Quick Reference

```logi
// Atoms (lowercase)
true, false, nil, zero

// Variables (uppercase)
X, Y, P, Q

// Function (one pattern per arrow)
not = true => false | false => true

// Currying (multiple arguments)
add = X => Y => plus X Y

// Application
f x y  =  ((f x) y)

// Quotation (prevent reduction)
term = {not true}

// Extract from quote
extract = {X} => X

// Theorem marker
axiom = theorem (imp p q)

// Structured patterns
head = (cons X Xs) => X
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
```

---

## Common Mistakes to Avoid

### ❌ Wrong: Using `->` instead of `=>`
```logi
not = true -> false  ✗
```
**✅ Correct:**
```logi
not = true => false  ✓
```

### ❌ Wrong: Pipe as prefix with braces
```logi
not = { | true => false | false => true }  ✗
```
**✅ Correct:**
```logi
not = true => false | false => true  ✓
```

### ❌ Wrong: Curly braces for pattern matching
```logi
id = { X => X }  ✗
```
**✅ Correct:**
```logi
id = X => X  ✓
```

### ❌ Wrong: Curly braces for quotation are for preventing reduction
**Curly braces are ONLY for quotation:**
```logi
// Correct: prevents reduction
term = {not true}  ✓
```

### ❌ Wrong: Multiple patterns before arrow
```logi
add = X Y => plus X Y  ✗
```
**✅ Correct: curry**
```logi
add = X => Y => plus X Y  ✓
```

---

## If You Find Syntax Errors

If you find syntax errors in the current documentation:
1. Report them
2. Reference this document for correct syntax
3. Check FORMAL_SPECIFICATION.md as the authoritative source

---

## Summary

**For learning Logi:**
- Use: QUICKSTART.md, FORMAL_SPECIFICATION.md, EXAMPLES.md
- Avoid: Design exploration files (historical only)

**Correct syntax:**
- Arrow: `=>`
- Pipe: binary infix `|`
- Curly braces: quotation `{M}` only
- One pattern per arrow (curry for multiple args)

**The language has 7 constructs:**
1. Atoms: `true`
2. Variables: `X`
3. Application: `M N`
4. Function: `pattern => M`
5. Choice: `M | M`
6. Theorem: `theorem M`
7. Quotation: `{M}`

