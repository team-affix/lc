# The Logi Language: Formal Specification

## Abstract

Logi is a minimal logic verification language based on pattern matching that unifies computation and proof. Functions are defined via pattern matching, creating partial functions naturally. The language achieves first-order logic expressiveness with only 6 core constructs.

---

## 1. Syntax

### 1.1 Abstract Syntax

```
Terms (M, N, P, Q):
  M ::= atom                    (atom - lowercase identifier)
      | Var                     (variable - uppercase identifier)
      | M M                     (application)
      | pattern => M            (function - one pattern per arrow)
      | M | M                   (choice - alternative cases)
      | theorem M               (theorem marker)

Patterns (p, q):
  p ::= atom                    (atom pattern - matches exactly)
      | Var                     (variable pattern - binds anything)
      | _                       (wildcard - matches, doesn't bind)
      | (p p)                   (application pattern)
      | theorem p               (theorem pattern)

Atoms (a, b, c):
  Lowercase identifiers: true, false, nil, zero, etc.

Variables (X, Y, Z):
  Uppercase identifiers: X, Y, P, Q, Head, Tail, etc.
```

### 1.2 Key Design Principles

1. **One pattern per arrow:** Each `=>` takes exactly one pattern. Multiple arguments require currying.
2. **Case convention:** Lowercase = atoms, Uppercase = variables (pattern variables in patterns, value variables in terms)
3. **Partial functions:** No pattern match = no reduction (not an error)
4. **Binary pipe:** `|` is an infix operator joining alternatives, not a prefix

### 1.3 Precedence (Lowest to Highest)

```
|     (choice - right-associative)
=>    (function - right-associative)
app   (application - left-associative)
```

**Examples:**
```
X => Y => Z             =  X => (Y => Z)
f X Y                   =  (f X) Y
a => b | c => d         =  (a => b) | (c => d)
X => Y => f X | g => h  =  (X => (Y => f X)) | (g => h)
```

---

## 2. Semantics

### 2.1 Values

```
Values (V, W):
  V ::= atom
      | pattern => M
      | M | M                   (choice is a value)
      | theorem V
```

### 2.2 Pattern Matching

Pattern matching is defined by the partial function:

```
match : Pattern × Value → Substitution ∪ {FAIL}
```

**Definition:**

```
match(Var, V)               = [Var ↦ V]
match(_, V)                 = []
match(atom, atom)           = []
match(atom₁, atom₂)         = FAIL              (if atom₁ ≠ atom₂)
match((p₁ p₂), (V₁ V₂))    = σ₁ ∪ σ₂           (if σ₁ = match(p₁, V₁) ≠ FAIL,
                                                      σ₂ = match(p₂, V₂) ≠ FAIL)
match(theorem p, theorem V) = match(p, V)
match(p, V)                 = FAIL              (otherwise)
```

### 2.3 Substitution

Standard capture-avoiding substitution:

```
M[σ]    applies substitution σ to term M
```

**Substitution rules:**
```
atom[σ]              = atom
Var[σ]               = σ(Var) if Var ∈ dom(σ), else Var
(M N)[σ]             = M[σ] N[σ]
(p => M)[σ]          = p => M[σ']     where σ' = σ \ bound(p)
(M | N)[σ]           = M[σ] | N[σ]
(theorem M)[σ]       = theorem M[σ]
```

**Where `bound(p)` returns variables bound by pattern `p`:**
```
bound(atom)          = ∅
bound(Var)           = {Var}
bound(_)             = ∅
bound((p₁ p₂))       = bound(p₁) ∪ bound(p₂)
bound(theorem p)     = bound(p)
```

### 2.4 Small-Step Operational Semantics

#### Evaluation Contexts

```
E ::= □                     (hole)
    | E M                   (application left)
    | V E                   (application right)
    | theorem E             (theorem)
```

#### Reduction Rules

```
──────────────────────────────────────  [MATCH-SUCCESS]
(p => M) V  ⟶  M[σ]
  where match(p, V) = σ ≠ FAIL


──────────────────────────────────────  [MATCH-FAIL-TRY-ALTERNATIVE]
((p => M) | N) V  ⟶  N V
  where match(p, V) = FAIL


──────────────────────────────────────  [MATCH-SUCCESS-SKIP-ALTERNATIVE]
((p => M) | N) V  ⟶  M[σ]
  where match(p, V) = σ ≠ FAIL


──────────  [CTX]
M ⟶ M'
──────────
E[M] ⟶ E[M']
```

**Note:** If a function `(p => M)` is applied to value `V` and `match(p, V) = FAIL`, and there is no alternative (no `|`), then **no reduction occurs**. This is the partial function behavior.

### 2.5 Multi-Step Reduction

```
M ⟶* N   iff   M = M₀ ⟶ M₁ ⟶ ... ⟶ Mₙ = N   (reflexive transitive closure)
```

### 2.6 Normal Forms

A term M is in **normal form** if:
1. M is a value, OR
2. M is an application that cannot reduce (partial function case)

**Examples:**
```
true                                    // value (atom)
X => X                                  // value (function)
theorem (imp a b)                       // value (theorem of atom structure)
(head nil)                              // normal form (partial application)
(not xyz)                               // normal form (xyz not a boolean)
```

---

## 3. Type System (Optional Sort System)

While the core calculus is untyped, we can define a simple sort system for static analysis.

### 3.1 Sorts

```
Sorts (τ):
  τ ::= •         (computational sort)
      | ⊢         (propositional sort)
```

### 3.2 Typing Judgment

```
Γ ⊢ M : τ
```

### 3.3 Selected Typing Rules

```
──────────  [ATOM]
Γ ⊢ atom : •


Var : τ ∈ Γ
───────────  [VAR]
Γ ⊢ Var : τ


Γ, Var : τ₁ ⊢ M : τ₂
──────────────────────  [FUN]
Γ ⊢ (Var => M) : τ₁ → τ₂


Γ ⊢ M : τ₁ → τ₂    Γ ⊢ N : τ₁
────────────────────────────  [APP]
Γ ⊢ M N : τ₂


Γ ⊢ M : •
────────────────  [THEOREM]
Γ ⊢ theorem M : ⊢


Γ ⊢ M : τ    Γ ⊢ N : τ
──────────────────────  [CHOICE]
Γ ⊢ M | N : τ
```

**Note:** This type system is optional and not part of the core calculus.

---

## 4. Metatheory

### 4.1 Theorem: Determinism

**Statement:** If M ⟶ N and M ⟶ N', then N = N'.

**Proof:** By induction on the derivation of M ⟶ N. Pattern matching returns a unique substitution (or fails), ensuring deterministic reduction.

### 4.2 Theorem: Progress (Weak Form)

**Statement:** If M is closed and well-formed, then either:
1. M is a value, OR
2. M is in normal form (including partial applications), OR
3. There exists N such that M ⟶ N

**Note:** This is a weak progress theorem. Terms can reach normal form without being values (partial function applications).

### 4.3 Theorem: Confluence

**Statement:** If M ⟶* N₁ and M ⟶* N₂, then there exists P such that N₁ ⟶* P and N₂ ⟶* P.

**Proof:** Follows from determinism. Since reduction is deterministic, there is at most one reduction path from any term.

### 4.4 Theorem: Static Soundness of Axiom Detection

**Statement:** If a closed term M does not syntactically contain `theorem` at the top level, then M ⟶* V implies V ≠ theorem W unless the reduction uses a subterm that contains `theorem`.

**Informal Proof:** The `theorem` constructor can only appear via:
1. Explicit use in the term (detected by static analysis as an axiom)
2. Reduction of a subterm that produces it (derived theorem)

Static analysis identifies all direct axiom declarations. Derived theorems must come from applying functions that produce theorems.

---

## 5. Concrete Syntax

### 5.1 Lexical Structure

**Tokens:**
- **Atoms:** Lowercase identifiers matching `[a-z][a-zA-Z0-9_]*`
- **Variables:** Uppercase identifiers matching `[A-Z][a-zA-Z0-9_]*`
- **Wildcard:** `_`
- **Keywords:** `theorem`
- **Operators:** `=>`, `|`, `(`, `)`
- **Comments:** `//` for line comments, `/* */` for block comments

### 5.2 Grammar

```
program    ::= definition*

definition ::= identifier '=' term

term       ::= atom
             | Var
             | 'theorem' term
             | term term                        // application
             | pattern '=>' term                // function
             | term '|' term                    // choice
             | '(' term ')'                     // grouping

pattern    ::= atom
             | Var
             | '_'
             | 'theorem' pattern
             | '(' pattern pattern ')'          // application pattern
```

### 5.3 Example Programs

**Boolean logic:**
```
not = true => false | false => true

and = true => (X => X) | false => (_ => false)

or = true => (_ => true) | false => (X => X)
```

**Modus ponens:**
```
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
```

**List operations:**
```
head = (cons X Xs) => X

tail = (cons X Xs) => Xs

length = nil => zero | (cons _ Xs) => (succ (length Xs))
```

---

## 6. Standard Library (Proposed)

### 6.1 Boolean Logic

```
// Base values
true = true
false = false

// Negation
not = true => false | false => true

// Conjunction
and = true => (X => X) | false => (_ => false)

// Disjunction
or = true => (_ => true) | false => (X => X)

// Implication (constructor)
imp = P => Q => (imp P Q)

// Biconditional (constructor)
iff = P => Q => (iff P Q)

// Negation (constructor for propositions)
neg = P => (neg P)
```

### 6.2 Propositional Inference Rules

```
// Modus Ponens: (P → Q), P ⊢ Q
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Modus Tollens: (P → Q), ¬Q ⊢ ¬P
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

// Hypothetical Syllogism: (P → Q), (Q → R) ⊢ (P → R)
hs = (theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R)

// Conjunction Introduction: P, Q ⊢ (P ∧ Q)
conj_intro = (theorem P) => (theorem Q) => theorem (and P Q)

// Conjunction Elimination Left: (P ∧ Q) ⊢ P
conj_elim_l = (theorem (and P Q)) => theorem P

// Conjunction Elimination Right: (P ∧ Q) ⊢ Q
conj_elim_r = (theorem (and P Q)) => theorem Q

// Disjunction Introduction Left: P ⊢ (P ∨ Q)
disj_intro_l = (theorem P) => Q => theorem (or P Q)

// Disjunction Introduction Right: Q ⊢ (P ∨ Q)
disj_intro_r = P => (theorem Q) => theorem (or P Q)

// Double Negation Elimination: ¬¬P ⊢ P
double_neg_elim = (theorem (neg (neg P))) => theorem P

// Contrapositive: (P → Q) ⊢ (¬Q → ¬P)
contrapositive = (theorem (imp P Q)) => theorem (imp (neg Q) (neg P))
```

### 6.3 Natural Numbers (Church/Peano Style)

```
// Zero
zero = zero

// Successor
succ = N => (succ N)

// Addition
add = zero => (N => N) | (succ M) => (N => (succ (add M N)))

// Multiplication
mul = zero => (_ => zero) | (succ M) => (N => (add N (mul M N)))
```

### 6.4 Lists

```
// Constructors
nil = nil
cons = X => Xs => (cons X Xs)

// Head (partial)
head = (cons X Xs) => X

// Tail (partial)
tail = (cons X Xs) => Xs

// Length
length = nil => zero | (cons _ Xs) => (succ (length Xs))

// Append
append = nil => (Ys => Ys) | (cons X Xs) => (Ys => (cons X (append Xs Ys)))

// Map
map = F => (nil => nil | (cons X Xs) => (cons (F X) (map F Xs)))
```

### 6.5 Quantifiers (Higher-Order Abstract Syntax)

```
// Universal quantifier: ∀P where P is a predicate (Var → Prop)
forall = P => (forall P)

// Existential quantifier: ∃P where P is a predicate (Var → Prop)
exists = P => (exists P)

// Universal Instantiation: (∀P) ⊢ P(t)
ui = (theorem (forall P)) => T => theorem (P T)

// Universal Generalization: P(x) ⊢ ∀P
// (Assuming x is arbitrary/not free in assumptions)
ug = (theorem Prop) => P => theorem (forall P)

// Existential Introduction: P(t) ⊢ ∃P
ei = (theorem Prop) => P => theorem (exists P)

// Existential Elimination: (∃P), (∀x. P(x) → Q) ⊢ Q
ee = (theorem (exists P)) => (theorem (forall (X => (imp (P X) Q)))) => theorem Q
```

---

## 7. Implementation Considerations

### 7.1 Parser

**Lexer:**
- Identify atoms (lowercase), variables (uppercase), keywords, operators
- Handle comments

**Parser:**
- Precedence climbing or operator precedence parsing
- Handle right-associativity of `=>` and `|`
- Left-associativity of application

**AST Construction:**
- Build tree respecting precedence
- Validate pattern syntax (only allowed in function LHS of `=>`)

### 7.2 Pattern Matcher

```python
def match(pattern, value):
    """Returns dict of bindings or None if match fails."""
    if isinstance(pattern, VarPattern):
        return {pattern.name: value}
    elif isinstance(pattern, WildcardPattern):
        return {}
    elif isinstance(pattern, AtomPattern):
        if isinstance(value, Atom) and pattern.atom == value.atom:
            return {}
        else:
            return None
    elif isinstance(pattern, AppPattern):
        if isinstance(value, App):
            bindings1 = match(pattern.lhs, value.lhs)
            if bindings1 is None:
                return None
            bindings2 = match(pattern.rhs, value.rhs)
            if bindings2 is None:
                return None
            return {**bindings1, **bindings2}
        else:
            return None
    elif isinstance(pattern, TheoremPattern):
        if isinstance(value, Theorem):
            return match(pattern.inner, value.inner)
        else:
            return None
    else:
        return None
```

### 7.3 Evaluator

```python
def reduce_step(term):
    """Perform one reduction step. Returns new term or None if no reduction."""
    if isinstance(term, App):
        # Try to reduce function position
        if not is_value(term.func):
            func_reduced = reduce_step(term.func)
            if func_reduced is not None:
                return App(func_reduced, term.arg)
        
        # Try to reduce argument position
        elif not is_value(term.arg):
            arg_reduced = reduce_step(term.arg)
            if arg_reduced is not None:
                return App(term.func, arg_reduced)
        
        # Both are values - try application
        else:
            if isinstance(term.func, Function):
                bindings = match(term.func.pattern, term.arg)
                if bindings is not None:
                    return substitute(term.func.body, bindings)
                else:
                    return None  # No match, no reduction
            
            elif isinstance(term.func, Choice):
                # Try left alternative
                result = reduce_step(App(term.func.left, term.arg))
                if result is not None:
                    return result
                # If left fails, try right
                return reduce_step(App(term.func.right, term.arg))
            
            else:
                return None  # Can't apply non-function
    
    elif isinstance(term, Theorem):
        if not is_value(term.inner):
            inner_reduced = reduce_step(term.inner)
            if inner_reduced is not None:
                return Theorem(inner_reduced)
    
    return None  # No reduction
```

### 7.4 Static Analyzer

**Axiom Detection:**

```python
def is_axiom(definition):
    """Check if definition directly declares a theorem (is an axiom)."""
    return contains_theorem_at_top(definition.body)

def contains_theorem_at_top(term):
    """Check if term is a theorem constructor at top level."""
    if isinstance(term, Theorem):
        return True
    return False
```

**Exhaustiveness Checking (Optional):**

Check if pattern matching is exhaustive for a given type. This is optional but helpful for catching errors.

---

## 8. Examples

### 8.1 Simple Boolean Computation

```
not true
= (true => false | false => true) true
= (true => false) true           // Try first case
= false                          // Match succeeds, return false
```

```
not false
= (true => false | false => true) false
= (true => false) false          // Try first case, no match
= (false => true) false          // Try second case
= true                           // Match succeeds
```

### 8.2 Modus Ponens Derivation

```
ax1 = theorem (imp p q)
ax2 = theorem p

result = mp ax1 ax2
```

**Reduction:**
```
mp (theorem (imp p q)) (theorem p)
= ((theorem (imp P Q)) => (theorem P) => theorem Q) (theorem (imp p q)) (theorem p)

// Apply first arrow
= ((theorem P) => theorem Q)[P := p, Q := q] (theorem p)
= ((theorem p) => theorem q) (theorem p)

// Apply second arrow
= (theorem q)[P matched]
= theorem q  ✓
```

### 8.3 List Length Computation

```
mylist = (cons 1 (cons 2 (cons 3 nil)))

length mylist
```

**Reduction:**
```
length (cons 1 (cons 2 (cons 3 nil)))
= (nil => zero | (cons _ Xs) => (succ (length Xs))) (cons 1 (cons 2 (cons 3 nil)))
= (cons _ Xs => (succ (length Xs))) (cons 1 (cons 2 (cons 3 nil)))
= (succ (length (cons 2 (cons 3 nil))))[Xs := (cons 2 (cons 3 nil))]
= (succ (length (cons 2 (cons 3 nil))))
= (succ (succ (length (cons 3 nil))))
= (succ (succ (succ (length nil))))
= (succ (succ (succ zero)))
```

### 8.4 Partial Function (No Match)

```
head nil
= ((cons X Xs) => X) nil
// Pattern (cons X Xs) doesn't match nil
// No alternative provided
// No reduction - term stays as (head nil)
```

This is not an error! It's simply an undefined value (partial function).

---

## 9. Extensions and Future Work

### 9.1 Module System

Organize large proof libraries:
```
module Logic where
  export mp, mt, hs
  
  mp = ...
  mt = ...
  hs = ...

import Logic (mp, hs)
```

### 9.2 Let Bindings

For convenience:
```
let X = expr in body
```

Desugars to:
```
(X => body) expr
```

### 9.3 Recursive Definitions

Allow self-reference in top-level definitions:
```
factorial = zero => (succ zero) | (succ N) => mul (succ N) (factorial N)
```

Implemented via fixed-point combinator internally.

### 9.4 Pattern Guards

Allow conditions in patterns:
```
divide = X Y | Y != zero => (div X Y)
```

### 9.5 Reflection and Reification

For meta-programming:
```
quote M       // reify term as data
unquote M     // evaluate data as term
typeof M      // inspect structure
```

---

## 10. Comparison to Related Systems

### 10.1 vs. Lambda Calculus

**Similarities:**
- Function abstraction and application
- Substitution-based semantics
- Beta-reduction

**Differences:**
- Built-in pattern matching (not encoded)
- Partial functions natural
- Theorem marker for proof tracking

### 10.2 vs. ML/Haskell

**Similarities:**
- Pattern matching syntax
- Currying
- Higher-order functions

**Differences:**
- No static type system (or optional)
- Opposite case convention (uppercase vars, lowercase atoms)
- Theorem marker for logic
- Simpler (fewer features)

### 10.3 vs. Prolog

**Similarities:**
- Pattern matching
- Case convention (uppercase vars)
- Logic programming oriented

**Differences:**
- No backtracking (deterministic)
- No unification (structural matching only)
- First-class functions
- Explicit computation

### 10.4 vs. Coq/Agda/Lean

**Similarities:**
- Goal: verify proofs
- Support user-defined inference rules

**Differences:**
- No dependent types
- No universe hierarchy
- Propositions are data, not types
- Simpler semantics
- No termination checking
- Partial functions allowed

---

## 11. Summary

The Logi language achieves:

1. **Minimalism:** 6 core constructs
2. **Expressiveness:** First-order logic with user-defined rules
3. **Simplicity:** Pattern matching as the sole binding mechanism
4. **Clarity:** One pattern per arrow eliminates ambiguity
5. **Elegance:** Partial functions arise naturally from failed pattern matches
6. **Practicality:** Clean syntax for common cases (currying, no end token)

**Core innovation:** Unifying computation and proof through pattern matching, with static soundness via the `theorem` keyword, avoiding the complexity of dependent types while retaining expressiveness for logic verification.

---

## Appendix A: Complete BNF Grammar

```
<program>     ::= <definition>*

<definition>  ::= <identifier> "=" <term>

<term>        ::= <atom>
                | <variable>
                | "theorem" <term>
                | <term> <term>
                | <pattern> "=>" <term>
                | <term> "|" <term>
                | "(" <term> ")"

<pattern>     ::= <atom>
                | <variable>
                | "_"
                | "theorem" <pattern>
                | "(" <pattern> <pattern> ")"

<atom>        ::= [a-z][a-zA-Z0-9_]*

<variable>    ::= [A-Z][a-zA-Z0-9_]*

<identifier>  ::= <atom> | <variable>
```

---

## Appendix B: Reduction Examples

See PROOF_EXAMPLES.md for detailed worked examples of:
- Boolean logic operations
- Propositional inference rules
- Quantifier instantiation
- List operations
- Complex proof derivations

---

End of Formal Specification.
