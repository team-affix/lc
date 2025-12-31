# Logi: Quick Start Guide

## TL;DR

**Logi** is a minimal logic verification language that unifies computation and proof using pattern matching.

**Core idea:** 
- Everything is a pattern matching function
- `theorem M` marks propositions
- Static analysis detects axioms
- No dependent types, no backtracking, no complexity

---

## The Calculus in 60 Seconds

### Syntax (5 constructs)

```
M ::= x                          // variable
    | a                          // atom
    | { | p ⇒ M | ... }         // matching function
    | M M                        // application
    | theorem M                  // theorem marker
```

### Examples

**Boolean logic:**
```
not = { | T ⇒ F | F ⇒ T }
and = { | T ⇒ { | x ⇒ x } | F ⇒ { | _ ⇒ F } }
```

**Axioms:**
```
axiom1 = theorem (imp P Q)
axiom2 = theorem P
```

**Inference rules:**
```
mp = {
  | (theorem (imp p q)) ⇒ {
    | (theorem p) ⇒ theorem q
  }
}
```

**Derivations:**
```
result = mp axiom1 axiom2   // theorem Q
```

---

## Key Insights

### 1. Pattern Matching is Everything

Lambda abstraction is just pattern matching:
```
λx. M   =   { | x ⇒ M }
```

Multi-branch functions use pattern matching:
```
f = {
  | pattern1 ⇒ result1
  | pattern2 ⇒ result2
}
```

**One mechanism for:**
- Function parameters
- Branching
- Destructuring

### 2. Theorems are Tagged Computations

No separation between computation and proof:

```
// Computation
result = and T F   // F

// Proof
result = mp axiom1 axiom2   // theorem Q
```

Same evaluation rules, same syntax, same functions.

### 3. Static Analysis for Soundness

Axioms detected by `theorem` keyword:
```
// Axiom (detected statically)
ax = theorem P

// Derivation (computed)
derived = mp ax1 ax2
```

Can't produce `theorem P` without axiomatizing or deriving it.

### 4. HOAS for Quantifiers

Universal quantification uses higher-order functions:

```
// ∀x. P(x)
universal = theorem (forall (λx. (P x)))

// Instantiate with A
ui universal A   // theorem (P A)
```

Elegant and leverages existing lambda machinery!

---

## Comparison to Alternatives

| Feature | Logi | Coq/Agda | Prolog |
|---------|------|----------|--------|
| Core primitives | Pattern matching | Dependent types | Unification |
| Binding mechanisms | 1 | 2+ | 1 |
| Type system | None/Optional | Dependent | None |
| Backtracking | No | No | Yes |
| Beginner-friendly | ✓ Yes | ✗ No | ✓ Somewhat |
| Theorem-as-types | ✗ No | ✓ Yes | N/A |
| Universe hierarchies | ✗ No | ✓ Yes | N/A |

**Logi = Minimal + Expressive + Beginner-friendly**

---

## Detailed Examples

### Example 1: Modus Ponens

```
// Axioms
ax1 = theorem (imp P Q)   // P → Q
ax2 = theorem P            // P

// Inference rule
mp = {
  | (theorem (imp p q)) ⇒ {
    | (theorem p) ⇒ theorem q
  }
}

// Derive Q
result = mp ax1 ax2   // reduces to: theorem Q
```

**Reduction steps:**
```
mp ax1 ax2
= mp (theorem (imp P Q)) (theorem P)
= { | (theorem p) ⇒ theorem q }[p ↦ P, q ↦ Q] (theorem P)
= { | (theorem P) ⇒ theorem Q } (theorem P)
= theorem Q   ✓
```

### Example 2: Boolean Computation

```
not = { | T ⇒ F | F ⇒ T }

test = not T
```

**Reduction:**
```
not T
= { | T ⇒ F | F ⇒ T } T
= F   ✓
```

### Example 3: Quantifier Instantiation

```
// ∀x. IsEven(x) → IsEven(x+2)
ax = theorem (forall (λx. (imp (IsEven x) (IsEven (plus x 2)))))

// Universal instantiation
ui = {
  | (theorem (forall pred)) ⇒ {
    | t ⇒ theorem (pred t)
  }
}

// Instantiate with 5
instance = ui ax 5
```

**Result:**
```
theorem (imp (IsEven 5) (IsEven 7))   ✓
```

---

## Why This Design?

### Achieves Your Goals

✓ **Minimal** - 5 constructs, 1 binding mechanism  
✓ **Expressive** - Encodes first-order logic  
✓ **Integrated** - Computation and proof unified  
✓ **Beginner-friendly** - Pattern matching is intuitive  
✓ **Sound** - Static axiom detection  
✓ **No dependent types** - Simpler theory  
✓ **No backtracking** - Deterministic evaluation  

### Realizes Your Vision

From your notes:
> "We can use this system to simply declare lambda calculus programs, and they don't have to be propositional so long as we never use the keyword `theorem`."

**Exactly what this design does!**

### Cleaner Than Alternatives

**vs. Lambda + Pattern Matching (2 mechanisms):**
- Logi has 1 mechanism (patterns subsume lambda)

**vs. Dependent Types:**
- No universe hierarchies
- No level primitives
- No propositions-as-types encoding

**vs. Prolog:**
- No backtracking (simpler)
- First-class functions
- Explicit computation

---

## Implementation Status

### What's Done

✓ Complete formal specification (FORMAL_SPEC.md)  
✓ Operational semantics defined  
✓ Pattern matching algorithm specified  
✓ Example proofs worked out (PROOF_EXAMPLES.md)  
✓ Critical analysis completed (CRITICAL_ANALYSIS.md)  

### What's Next

**Phase 1: Core Interpreter**
- Lexer, parser, evaluator
- ~500-800 lines of code
- 1-2 weeks

**Phase 2: Standard Library**
- Propositional logic
- First-order logic
- Basic inference rules
- ~200-300 lines of Logi

**Phase 3: Tooling**
- REPL
- Error messages
- Static analyzer

---

## Notation Guide

### Core Syntax

```
// Variables (lowercase)
x, y, z

// Atoms (uppercase)
T, F, P, Q

// Matching function
{
  | pattern ⇒ body
  | pattern ⇒ body
}

// Application
f x y   // left-associative: (f x) y

// Theorem
theorem P

// Grouping
(expr)
```

### Syntactic Sugar

**Lambda:**
```
λx. M        // full lambda
\x. M        // ASCII alternative
{ | x ⇒ M }  // desugared form
```

**Arrow:**
```
p → M ; q → N      // arrow style
p -> M ; q -> N    // ASCII
{ | p ⇒ M | q ⇒ N }  // desugared
```

### Patterns

```
x           // variable (binds to anything)
_           // wildcard (matches, doesn't bind)
T           // atom (matches literal)
(p q)       // application (matches structure)
theorem p   // theorem (matches theorem with pattern)
```

---

## Common Patterns

### Multi-Argument Functions

```
// Using lambda sugar
f = λx y. M

// Using pattern matching
f = { | x ⇒ { | y ⇒ M } }

// Inline patterns (if supported)
f = { | x ⇒ { | y ⇒ M } }
```

### Case Analysis

```
f = {
  | case1 ⇒ result1
  | case2 ⇒ result2
  | _ ⇒ default
}
```

### Inference Rules

```
rule = {
  | (theorem premise) ⇒ theorem conclusion
}

// Multi-premise
rule = {
  | (theorem p1) ⇒ {
    | (theorem p2) ⇒ theorem conclusion
  }
}
```

### Quantifiers (HOAS)

```
// ∀x. P(x)
universal = theorem (forall (λx. (P x)))

// ∃x. P(x)
existential = theorem (exists (λx. (P x)))

// Instantiation
ui = {
  | (theorem (forall pred)) ⇒ {
    | t ⇒ theorem (pred t)
  }
}
```

---

## FAQ

### Q: Why pattern matching instead of lambda?

**A:** Pattern matching subsumes lambda (lambda is pattern matching with one variable pattern). Pattern matching also provides branching and destructuring in one mechanism.

### Q: Can I use lambda syntax?

**A:** Yes! `λx. M` is sugar for `{ | x ⇒ M }`. Use whichever you prefer.

### Q: How do quantifiers work?

**A:** Via Higher-Order Abstract Syntax (HOAS). Predicates are functions, and quantification is over functions. E.g., `∀x. P(x)` is `(forall (λx. (P x)))`.

### Q: Is this typed?

**A:** No, the core calculus is untyped. Optional type annotations can be added later for documentation.

### Q: Can users axiomatize false?

**A:** Yes. Logi is a verification language, not a proof checker that prevents contradictions. Users are responsible for choosing sound axioms.

### Q: How is soundness achieved without types?

**A:** Static analysis detects axiom declarations (definitions with `theorem` at top level). Theorems can only be produced by axiomatizing or computing from existing theorems.

### Q: What about non-termination?

**A:** Allowed in the calculus. Users responsible for well-formed definitions. Optional static analyzer can warn about potential non-termination.

### Q: Can I do meta-programming?

**A:** Not in the initial version. Quotation/reflection can be added later if needed.

### Q: How fast is it?

**A:** Naive implementation is adequate for proof checking. Pattern compilation and explicit substitutions can optimize further.

---

## Next Steps

### To Learn More

1. **Read THEORY.md** - Exploration of design space
2. **Read FORMAL_SPEC.md** - Complete formal specification
3. **Read PROOF_EXAMPLES.md** - Concrete proof examples
4. **Read RECOMMENDATIONS.md** - Final design decisions

### To Implement

1. Choose language (Python, OCaml, Haskell, Rust)
2. Implement lexer and parser
3. Implement pattern matcher
4. Implement evaluator
5. Build REPL
6. Test with examples from PROOF_EXAMPLES.md

### To Contribute

1. Review formal specification
2. Suggest improvements
3. Write example proofs
4. Build tooling
5. Write documentation

---

## Code Snippet: Complete Minimal Example

```logi
// Boolean values
T = T
F = F

// Negation
not = { | T ⇒ F | F ⇒ T }

// Implication (data constructor)
imp = λp q. (imp p q)

// Propositional variables
P = P
Q = Q

// Axioms
axiom1 = theorem (imp P Q)   // P → Q
axiom2 = theorem P            // P

// Modus ponens
mp = {
  | (theorem (imp p q)) ⇒ {
    | (theorem p) ⇒ theorem q
  }
}

// Derive Q
conclusion = mp axiom1 axiom2   // theorem Q
```

**Run this in the Logi interpreter (once implemented) and it will derive `theorem Q`!**

---

## Visual Summary

```
┌─────────────────────────────────────────────┐
│           The Logi Calculus                 │
├─────────────────────────────────────────────┤
│                                             │
│  Core:    Pattern Matching Functions       │
│  Binding: Single Mechanism (Patterns)       │
│  Proof:   theorem Marker                    │
│  Sound:   Static Axiom Detection            │
│                                             │
├─────────────────────────────────────────────┤
│                                             │
│  ✓ Minimal        (5 constructs)            │
│  ✓ Expressive     (FOL complete)            │
│  ✓ Integrated     (Computation = Proof)     │
│  ✓ Beginner-friendly (Intuitive syntax)     │
│  ✓ Sound          (Static analysis)         │
│                                             │
└─────────────────────────────────────────────┘
```

---

## Resources

- **THEORY.md** - Theoretical exploration
- **FORMAL_SPEC.md** - Complete formal specification
- **PROOF_EXAMPLES.md** - Worked proof examples
- **CRITICAL_ANALYSIS.md** - Design critique and refinements
- **RECOMMENDATIONS.md** - Final design decisions and roadmap

---

**Ready to build Logi? Start with Phase 1: Core Interpreter!**

Estimated effort: 1-2 weeks for experienced developer.

Implementation complexity: ~500-800 lines for core interpreter.

**Let's make logic programming minimal, elegant, and accessible.** 🚀

---

End of Quick Start Guide.
