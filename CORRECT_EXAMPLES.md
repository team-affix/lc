# Logi: Correct Syntax Examples

## All Examples Use Current Correct Syntax

**Arrow:** `=>` (not `->`)  
**Pipe:** Binary infix `|` (not prefix with braces)  
**Quotes:** `{M}` (for preventing reduction only)  
**Multiple args:** Curry with `X => Y => body` (not `X Y => body`)

---

## Boolean Logic

```logi
// Values
true = true
false = false

// NOT
not = true => false | false => true

// AND (curried)
and = true => (X => X) | false => (_ => false)

// OR (curried)
or = true => (_ => true) | false => (X => X)

// Examples
not true                 // → false
and true false           // → false
or false true            // → true
```

---

## Propositional Constructors

```logi
// Implication (atom constructor - no quotes needed)
imp = P => Q => (imp P Q)

// Negation (atom constructor)
neg = P => (neg P)

// Conjunction (atom constructor)
conj = P => Q => (conj P Q)

// Disjunction (atom constructor)
disj = P => Q => (disj P Q)

// Usage
prop1 = imp p q              // → (imp p q)
prop2 = neg (imp p q)        // → (neg (imp p q))
prop3 = conj p (neg q)       // → (conj p (neg q))
```

---

## Inference Rules

```logi
// Modus Ponens: (P → Q), P ⊢ Q
mp = (theorem (imp P Q)) => (theorem P) => theorem Q

// Modus Tollens: (P → Q), ¬Q ⊢ ¬P
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

// Hypothetical Syllogism: (P → Q), (Q → R) ⊢ (P → R)
hs = (theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R)

// Conjunction Introduction: P, Q ⊢ (P ∧ Q)
conj_intro = (theorem P) => (theorem Q) => theorem (conj P Q)

// Conjunction Elimination Left: (P ∧ Q) ⊢ P
conj_elim_l = (theorem (conj P Q)) => theorem P

// Conjunction Elimination Right: (P ∧ Q) ⊢ Q
conj_elim_r = (theorem (conj P Q)) => theorem Q
```

---

## Axioms and Derivations

```logi
// Define propositions as atoms
p = p
q = q
r = r

// Axioms (marked with theorem)
ax1 = theorem (imp p q)      // P → Q
ax2 = theorem p               // P

// Derive Q using modus ponens
result = mp ax1 ax2          // → theorem q  ✓
```

---

## Lists

```logi
// Constructors
nil = nil
cons = X => Xs => (cons X Xs)

// Head (partial function)
head = (cons X Xs) => X

// Tail (partial function)
tail = (cons X Xs) => Xs

// Length
length = nil => zero | (cons _ Xs) => (succ (length Xs))

// Append
append = nil => (Ys => Ys) 
  | (cons X Xs) => (Ys => (cons X (append Xs Ys)))

// Map (curried)
map = F => (nil => nil | (cons X Xs) => (cons (F X) (map F Xs)))

// Filter
filter = Pred => (nil => nil 
  | (cons X Xs) => ((Pred X) => (cons X (filter Pred Xs)) 
                    | _ => (filter Pred Xs)))
```

---

## Natural Numbers

```logi
// Zero
zero = zero

// Successor
succ = N => (succ N)

// Numerals
one = (succ zero)
two = (succ one)
three = (succ two)
four = (succ three)
five = (succ four)

// Addition
add = zero => (N => N) 
  | (succ M) => (N => (succ (add M N)))

// Multiplication
mul = zero => (_ => zero) 
  | (succ M) => (N => (add N (mul M N)))

// Comparison
lte = zero => (_ => true)
  | (succ M) => zero => false
  | (succ M) => (succ N) => (lte M N)
```

---

## Quotation Examples

### Building Unreduced Terms

```logi
// Without quotes - arguments reduce
prop1 = imp (not true) q
// → (imp false q)  ✗ (not true) reduced to false

// With quotes - arguments preserved
prop2 = imp {not true} q
// → (imp {not true} q)  ✓ preserved
```

### Pattern Matching on Quotes

```logi
// Extract from quote
extract = {X} => X

term = {not true}
result = extract term
// → not true  (extracted, now can reduce)
// → false
```

### Symbolic Differentiation

```logi
// Derivative
deriv = Var => {Var} => one
  | Var => {Const} => zero
  | Var => {plus X Y} => {plus (deriv Var {X}) (deriv Var {Y})}
  | Var => {mul X Y} => {plus {mul X (deriv Var {Y})} {mul Y (deriv Var {X})}}
  | Var => {pow X N} => {mul N {mul (pow X (sub N one)) (deriv Var {X})}}

// Usage: d/dx (x^2 + x)
expr = {plus (pow {x} two) {x}}
derivative = deriv {x} expr
// → {plus (mul two (mul (pow {x} one) one)) one}
```

### Meta-Programming

```logi
// Get operator of term
get_op = {(F _)} => F

op = get_op {not p}
// → not

// Check term structure
is_negation = {(neg _)} => true | _ => false

is_negation {neg p}      // → true
is_negation {imp p q}    // → false
```

---

## Quantifiers (HOAS)

```logi
// Universal quantifier
forall = P => (forall P)

// Existential quantifier
exists = P => (exists P)

// Universal Instantiation
ui = (theorem (forall P)) => T => theorem (P T)

// Example usage
ax = theorem (forall (X => (prop X)))
instance = ui ax a
// → theorem (prop a)  ✓
```

---

## Complete Proof Example

```logi
// Constructors
imp = P => Q => (imp P Q)
neg = P => (neg P)

// Propositional variables
p = p
q = q

// Axioms
ax1 = theorem (imp p q)       // P → Q
ax2 = theorem (neg q)          // ¬Q

// Modus Tollens
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)

// Derive ¬P
result = mt ax1 ax2
// → theorem (neg p)  ✓
```

**Full reduction trace:**
```
mt (theorem (imp p q)) (theorem (neg q))

// First application: match (imp p q) with (imp P Q)
= ((theorem (neg Q)) => theorem (neg P))[P := p, Q := q]
  (theorem (neg q))

= ((theorem (neg q)) => theorem (neg p))
  (theorem (neg q))

// Second application: match (neg q) with (neg q)
= theorem (neg p)  ✓
```

---

## Currying Examples

```logi
// Identity (one argument)
id = X => X

// Constant function (two arguments, curried)
const = X => _ => X

// Composition (three arguments, curried)
compose = F => G => X => F (G X)

// Usage
double = X => (mul two X)
triple = X => (mul three X)

times_six = compose triple double
result = times_six five
// → triple (double five)
// → triple ten
// → thirty  ✓
```

---

## Partial Functions

```logi
// head is only defined for non-empty lists
head = (cons X Xs) => X

head (cons 1 nil)    // → 1  ✓
head nil              // → (head nil)  (doesn't reduce, undefined)

// safe_head with default
safe_head = (cons X Xs) => X | nil => none

safe_head (cons 1 nil)  // → 1
safe_head nil            // → none  ✓
```

---

## Recursive Functions

```logi
// Factorial
factorial = zero => one
  | (succ N) => (mul (succ N) (factorial N))

factorial three
// → (mul three (factorial two))
// → (mul three (mul two (factorial one)))
// → (mul three (mul two (mul one (factorial zero))))
// → (mul three (mul two (mul one one)))
// → (mul three (mul two one))
// → (mul three two)
// → six  ✓
```

---

## Summary

All examples above use **correct current syntax:**

- ✅ Arrow: `=>`
- ✅ Pipe: binary infix `|`
- ✅ Quotes: `{M}` for quotation
- ✅ Currying: `X => Y => body`
- ✅ Case: Uppercase vars, lowercase atoms

**Copy and adapt these examples for your own code!**

For more examples, see [EXAMPLES.md](EXAMPLES.md).

For the complete specification, see [FORMAL_SPECIFICATION.md](FORMAL_SPECIFICATION.md).

---

End of Correct Examples.
