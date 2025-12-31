# Quotation Mechanisms in Logi

## The Question

**Do we need a way to prevent reduction?** That is, can we build term structures as data without evaluating them?

---

## Current Behavior (No Quotation)

### What Reduces

```logi
// This reduces
not true  →  false

// If P is defined as (not true), this reduces arguments
imp P Q  →  imp false Q     // P reduces before being passed
```

### What Doesn't Reduce (Already Values)

```logi
// Atoms are values
true, false, imp

// Functions are values
X => body

// Applications of atoms to atoms (no function to apply)
(imp true false)    // This is already in normal form
```

---

## When Do We Need Quotation?

### Use Case 1: Building Syntax Without Evaluation

**Problem:**
```logi
// Want to build (imp (not true) Q) as data
// But (not true) reduces to false!

term = imp (not true) Q
// → imp false Q  ✗  (reduced!)
```

**What we want:**
```logi
term = quote (imp (not true) Q)
// → (imp (not true) Q)  ✓  (unreduced)
```

### Use Case 2: Meta-Programming

**Inspecting term structure:**
```logi
// Get the operator of a term
get_operator = (App F Arg) => F

// Want to analyze (not true) WITHOUT reducing it
op = get_operator (quote (not true))
// → not  ✓
```

### Use Case 3: Proving Properties About Programs

**Talking about program text:**
```logi
// "For all x, (not (not x)) reduces to x"
double_neg_theorem = forall (X => theorem (reduces (quote (not (not X))) X))
```

---

## Current Workaround: Constructor Convention

### The Pattern

**Define constructors that don't reduce:**

```logi
// Propositional constructors (just build structure)
imp = P => Q => (imp P Q)
neg = P => (neg P)
and = P => Q => (and P Q)
```

**These work because:**
- `imp`, `neg`, `and` in the result are atoms (not functions)
- `(imp P Q)` is an application of atom `imp` to arguments
- Since `imp` (the atom) isn't a function, no reduction happens

**Example:**
```logi
imp = P => Q => (imp P Q)

term = imp (not true) Q
// Reduces to: (imp false Q)
// The 'not true' reduces BEFORE being passed to imp
```

**But this works for building values:**
```logi
term = imp p q    // where p and q are atoms
// → (imp p q)  ✓  (no reduction)
```

---

## Proposed Solution: Add Quotation Primitive

### Syntax Addition

```logi
M ::= ...
    | quote M        // quotation (prevent reduction)
    | unquote M      // evaluation (explicit reduction)
```

### Semantics

**Quotation prevents reduction:**
```
quote M    is a value (doesn't reduce)
```

**Inside a quote, NO reduction happens:**
```
quote (not true)  ⟶  quote (not true)    // doesn't reduce to quote false
```

**Unquote forces evaluation:**
```
unquote (quote M)  ⟶  M
```

Then reduce `M` normally.

### Example Usage

**Build unreduced terms:**
```logi
term = quote (imp (not true) Q)
// → quote (imp (not true) Q)  ✓  (preserved)
```

**Extract and inspect:**
```logi
get_operator = (quote (F X)) => F

op = get_operator (quote (not true))
// → not  ✓
```

**Explicit reduction when needed:**
```logi
result = unquote (quote (not true))
// → unquote (quote (not true))
// → not true
// → false  ✓
```

---

## Design Decision: Should We Add It?

### Arguments FOR Adding Quotation

1. **Meta-programming** - Essential for term inspection and manipulation
2. **Proof about programs** - Need to talk about unreduced program text
3. **Reification** - Build syntax trees as first-class data
4. **Control over evaluation** - Sometimes you want data, not computation

### Arguments AGAINST Adding Quotation

1. **Complexity** - Adds another construct (now 7 instead of 6)
2. **Rarely needed** - Constructor convention works for most cases
3. **Can be added later** - Not essential for core logic verification
4. **Implementation burden** - More to implement and test

### Recommendation

**Add quotation as a future extension, not in initial version.**

**Rationale:**
- Core logic verification doesn't require it
- Constructor convention handles most common cases
- Can be added later without breaking existing code
- Keeps initial implementation simple

---

## Alternative: Hybrid Approach

### Add Only `quote` (No `unquote`)

**Minimal addition:**
```logi
M ::= ...
    | quote M
```

**Semantics:**
- `quote M` is a value (doesn't reduce)
- Inside `quote`, everything is frozen

**No unquote needed because:**
- Pattern matching can extract quoted terms
- Substitution naturally propagates values

**Example:**
```logi
// Build quoted term
term = quote (not true)

// Extract via pattern matching
get_inner = (quote X) => X

inner = get_inner term
// → not true  ✓  (extracted and now can reduce)
```

**This is simpler:** Only one construct added, no explicit unquote.

---

## Practical Examples

### Example 1: Meta-Theorem About Reduction

**Without quotation (awkward):**
```logi
// Can't express this cleanly
```

**With quotation:**
```logi
// "not true reduces to false"
reduction_theorem = theorem (reduces (quote (not true)) (quote false))
```

### Example 2: Term Builder

**Without quotation (works with convention):**
```logi
imp = P => Q => (imp P Q)

build_impl = imp p q
// → (imp p q)  ✓
```

**With quotation (more explicit):**
```logi
build_impl = quote (imp p q)
// → quote (imp p q)  ✓
```

### Example 3: Proof Checker

**With quotation (can inspect proof terms):**
```logi
// Check if proof is valid modus ponens
check_mp = (quote (mp (theorem (imp P Q)) (theorem P))) => valid

is_valid = check_mp (quote (mp ax1 ax2))
// → valid  ✓
```

---

## Implementation If Added

### Extended AST

```python
@dataclass
class Quote:
    inner: Term

Term = Union[Atom, Var, App, Function, Choice, Theorem, Quote]
```

### Extended Evaluator

```python
def reduce_step(term: Term) -> Term | None:
    # ... existing cases ...
    
    if isinstance(term, Quote):
        # Quote is a value, never reduces
        return None
    
    # When matching patterns, unpack quotes
    # (This is automatic - quotes become values that can be matched)
```

### Pattern Matching on Quotes

```python
# Pattern for quoted term
@dataclass
class PatQuote:
    inner: Pattern

# Match algorithm
def match_pattern(pattern: Pattern, value: Term) -> dict | None:
    # ... existing cases ...
    
    if isinstance(pattern, PatQuote):
        if isinstance(value, Quote):
            return match_pattern(pattern.inner, value.inner)
        else:
            return None
```

### Example in Extended Logi

```logi
// Define quote syntax
quote_not_true = quote (not true)

// Pattern match to extract
extract = (quote X) => X

result = extract quote_not_true
// → not true  ✓

// Now it can reduce
final = result
// → false  ✓
```

---

## Recommendation for Your Language

### Option A: Add Now (If You Need Meta-Programming)

**Add to FORMAL_SPECIFICATION.md:**

```
Terms:
  M ::= ...
      | quote M        (quotation)

Patterns:
  p ::= ...
      | quote p        (quote pattern)

Semantics:
  quote M  is a value (never reduces)
  
  Inside quote, no reduction occurs (frozen evaluation)
```

**Use when:**
- Building term structures programmatically
- Inspecting program structure (reflection)
- Proving properties about programs (meta-theorems)

### Option B: Add Later (Recommended)

**For now:**
- Use constructor convention (atoms as data constructors)
- Most logic verification doesn't need quotation
- Can add later as extension without breaking code

**Add when:**
- Users request meta-programming features
- Need to prove properties about reduction itself
- Want to build tactics that inspect proof structure

---

## Syntax Proposal (If Added)

### Option 1: Explicit Keyword

```logi
term = quote (not true)
```

**Pros:** Clear and obvious  
**Cons:** Verbose

### Option 2: Prefix Operator

```logi
term = '(not true)      // Single quote prefix
term = `(not true)      // Backtick prefix
```

**Pros:** Concise, Lisp-like  
**Cons:** Another operator to remember

### Option 3: Special Brackets

```logi
term = {not true}       // Curly braces for quote
term = [not true]       // Square brackets for quote
```

**Pros:** Visual distinction  
**Cons:** Conflicts with potential future syntax

**Recommendation:** Use explicit `quote` keyword for clarity.

---

## Current Answer to Your Question

### Is Quotation Supported?

**No, not explicitly in the current design.**

### Can You Prevent Reduction?

**Yes, indirectly via the constructor convention:**

```logi
// Define constructors (these don't reduce)
imp = P => Q => (imp P Q)

// Build terms (reduction happens in arguments before passing)
term = imp true false
// → (imp true false)  ✓  (imp is an atom in result, doesn't reduce further)
```

**But you CANNOT prevent reduction of arguments:**
```logi
term = imp (not true) Q
// → (imp false Q)  ✗  (not true reduces before being passed)
```

### Should We Add It?

**My recommendation: Add `quote` as a minimal extension.**

**Update syntax to:**
```logi
M ::= atom | Var | M M | pattern => M | M | M | theorem M | quote M
```

**This enables:**
1. Meta-programming (inspect terms)
2. Building unreduced syntax trees
3. Proving properties about programs
4. More control over evaluation

**Implementation cost:** ~50 lines of code, 1 hour of work

---

## Summary

### Current State
- ✅ Constructor convention works for building data
- ✗ Cannot prevent argument reduction
- ✗ Cannot inspect unreduced terms
- ✗ Limited meta-programming

### With Quotation Added
- ✅ Can build unreduced syntax trees
- ✅ Can inspect term structure
- ✅ Enables meta-programming
- ✅ Control over evaluation
- ⚠️ One more construct (7 total)

### My Recommendation

**Add `quote` as a minimal extension:**

```logi
quote M    // Prevent reduction of M and everything inside
```

**Pattern matching automatically handles extraction:**

```logi
extract = (quote X) => X
```

**This gives you full control when needed, while keeping most code simple.**

Would you like me to update the formal specification to include quotation?
