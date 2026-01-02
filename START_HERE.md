# 👋 Welcome to Logi!

## ✨ Logi Language: Complete Theory and Specification

**Status:** Design complete and ready for implementation!

---

## 🚀 Quick Start (5 minutes)

### The Language in 30 Seconds

```logi
// Boolean NOT
not = true => false | false => true

// Modus Ponens (inference rule)
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Axioms
ax1 = theorem (imp p q)
ax2 = theorem p

// Derive Q
result = mp ax1 ax2    // → theorem q  ✓
```

**Key syntax:**
- `=>` for arrows (not `->`)
- `|` is binary infix (connects alternatives)
- `{M}` for quotation (prevent reduction)
- Uppercase = variables, lowercase = atoms
- One pattern per arrow (curry for multiple args)

---

## 📚 Documentation (Read in This Order)

### Core Documentation (Start Here!)

1. ⭐⭐⭐ [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md) (5 minutes)
   **Read this first!** Quick reference card with all correct syntax.

2. ⭐⭐ [QUICKSTART.md](QUICKSTART.md) (10 minutes)
   Introduction to core concepts with examples.

3. ⭐ [LANGUAGE_SUMMARY.md](LANGUAGE_SUMMARY.md) (5 minutes)
   One-page language summary.

### Complete Specification

4. ⭐⭐⭐ [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md) (1-2 hours)
   Complete formal semantics - the authoritative reference.

5. ⭐⭐ [EXAMPLES.md](EXAMPLES.md) (1-2 hours)
   Detailed worked examples with full reduction traces.

### Specialized Topics

6. [QUOTATION_CURLY.md](QUOTATION_CURLY.md) (20 minutes)
   Deep dive into quotation mechanism with `{M}`.

7. [CONSTRUCTORS_VS_QUOTES.md](CONSTRUCTORS_VS_QUOTES.md) (10 minutes)
   When to use atom constructors vs. explicit quotation.

8. [USE_CASES.md](USE_CASES.md) (30 minutes)
   Use cases beyond logic: computation, meta-programming, DSLs, etc.

9. [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md) (1 hour)
   Complete Python implementation skeleton for building the interpreter.

10. [FINAL_SUMMARY.md](FINAL_SUMMARY.md) (30 minutes)
    Design rationale, evolution, and roadmap.

### Meta-Documentation

11. [DOCUMENTATION_STATUS.md](DOCUMENTATION_STATUS.md) (5 minutes)
    Which files are current vs. historical reference.

---

## ⚠️ Important Warnings

### Some Files Show Outdated/Rejected Syntax!

**See [DOCUMENTATION_STATUS.md](DOCUMENTATION_STATUS.md)** for complete list of which files are current vs. historical.

**Historical files** (design exploration - may show rejected syntax):
- RECONSIDERING.md
- REVISED_THEORY.md
- ARROW_SEMANTICS.md
- CLEAN_SYNTAX.md
- CORRECTED_SYNTAX.md
- FINAL_DESIGN.md

**These are kept for understanding the design evolution, but DO NOT learn syntax from them!**

---

## 🎯 Correct Syntax Summary

```
Arrow:       =>   (not ->)
Pipe:        |    (binary infix, not prefix)
Quotes:      {M}  (for quotation, not pattern matching)
Multiple args:    X => Y => body  (curry, not X Y => body)
Case:        Uppercase vars, lowercase atoms
```

**Example:**
```logi
// ✅ CORRECT
not = true => false | false => true
add = X => Y => plus X Y
term = {not true}

// ❌ WRONG (old syntax)
not = { | true => false }  ✗
add = X Y => plus X Y  ✗
```

---

## 🎓 Learning Paths

### Path 1: Quick Start (30 min)
1. SYNTAX_REFERENCE.md (5 min)
2. QUICKSTART.md (10 min)
3. LANGUAGE_SUMMARY.md (5 min)
4. Try writing some examples!

### Path 2: Implementation (3 hours)
1. SYNTAX_REFERENCE.md (5 min)
2. QUICKSTART.md (10 min)
3. FORMAL_SPECIFICATION.md (1-2 hours)
4. EXAMPLES.md - skim (30 min)
5. IMPLEMENTATION_GUIDE.md (1 hour)

### Path 3: Deep Understanding (6+ hours)
Read all ⭐ marked files in order.

---

## 🔧 Implementation Status

**Theoretical foundation:** ✅ Complete  
**Formal specification:** ✅ Complete  
**Examples:** ✅ Complete (15+)  
**Implementation guide:** ✅ Complete  
**Interpreter:** ⬜ Not yet implemented  

**Next step:** Implement interpreter using IMPLEMENTATION_GUIDE.md

**Estimated effort:** 2-3 weeks (~1000 LOC)

---

## 📊 What is Logi?

### Design Goals

1. **Minimal** - Only 7 constructs
2. **Logic-oriented** - First-order logic with user-defined inference rules
3. **Unified** - Computation and proof in same framework
4. **Beginner-friendly** - No dependent types, intuitive pattern matching
5. **Expressive** - Full programming language capabilities

### Core Innovation

**Theorem marker for static soundness:**

```logi
// Axiom (detected by static analysis)
axiom = theorem p

// Derivation (computed from axioms)
derived = mp axiom1 axiom2
```

Can't create theorems without axiomatizing or deriving from existing theorems!

### Unique Capabilities

1. ✅ Write normal programs (lists, trees, arithmetic)
2. ✅ Prove properties about programs
3. ✅ Same language for both (no separation!)
4. ✅ Meta-programming with quotation `{M}`
5. ✅ Partial functions natural (no exhaustiveness required)

---

## 🎯 Example Use Cases

### Logic Verification (Primary)
```logi
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
result = mp axiom1 axiom2
```

### Symbolic Computation
```logi
deriv = Var => {Var} => one
  | Var => {plus X Y} => {plus (deriv Var {X}) (deriv Var {Y})}
```

### Interpreters
```logi
eval = {const N} => N
  | {plus E1 E2} => (add (eval {E1}) (eval {E2}))
```

### DSLs
```logi
query = {select Fields From Where} => (run_query Fields From Where)
```

**See [USE_CASES.md](USE_CASES.md) for 13 categories of use cases!**

---

## 🚀 Next Steps

### To Learn Logi
1. Read [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md)
2. Read [QUICKSTART.md](QUICKSTART.md)
3. Try writing examples

### To Implement Logi
1. Read [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md)
2. Follow [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md)
3. Test with [EXAMPLES.md](EXAMPLES.md)

### To Understand Design
1. Read current documentation first
2. Then explore design evolution files (marked as historical)
3. See how we arrived at final design

---

## 📞 Summary

**Logi is a minimal logic verification language with:**
- 7 constructs (minimal!)
- Pattern matching for all functions
- Theorem tracking for proof verification
- Quotation for meta-programming
- No dependent types, no backtracking
- Clean, unambiguous syntax

**Ready for implementation: 2-3 weeks for core system (~1000 LOC)**

---

**Start reading: [SYNTAX_REFERENCE.md](SYNTAX_REFERENCE.md) → [QUICKSTART.md](QUICKSTART.md)** 🎉
