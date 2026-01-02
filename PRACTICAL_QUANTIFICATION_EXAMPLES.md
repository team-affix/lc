# Practical Examples of Multi-Case Existential Quantification

## The Challenge

Find realistic examples where:
1. We have a partial function with small finite domain
2. The outputs are propositional
3. Using `exists` over it is actually useful

---

## Example 1: Finding Solutions to Equations

### Problem: Does x² - 5x + 6 = 0 have a solution in {2, 3}?

**Mathematical setup:**
- Equation: x² - 5x + 6 = 0
- Factor form: (x-2)(x-3) = 0
- Solutions should be x=2 or x=3

**In Logi:**
```logi
// Define the polynomial
poly = X => (plus (square X) (plus (times (neg five) X) six))

// Check if at least one of our candidates is a root
has_root = exists (
    (two => (eq (poly two) zero)) |
    (three => (eq (poly three) zero))
)

// Evaluate: poly(2) = 4 - 10 + 6 = 0 ✓
// Evaluate: poly(3) = 9 - 15 + 6 = 0 ✓
// Result: has_root → true (both work!)
```

**Why this is useful:** We're testing specific candidate solutions, not doing abstract quantification.

---

## Example 2: Access Control / Authorization

### Problem: Can this user access the resource?

**Scenario:** A system where resources can be accessed by specific users who have permissions.

```logi
// Check if user has direct permission OR is an admin
can_access = resource => user => exists (
    (direct => (has_direct_permission user resource)) |
    (admin => (is_admin user))
)

// Usage:
theorem (can_access secret_file alice)
```

**Why this is useful:** Multiple authorization paths - direct permission OR admin status.

---

## Example 3: Proving Compositeness

### Problem: Show that N is composite by finding a small prime factor

**Scenario:** To prove a number is composite, show it's divisible by a small prime.

```logi
// N is composite if it has a prime divisor in {2, 3, 5, 7}
is_composite_small = N => exists (
    (two => (divides two N)) |
    (three => (divides three N)) |
    (five => (divides five N)) |
    (seven => (divides seven N))
)

// Test: Is 15 composite?
theorem (is_composite_small fifteen)
// Check: divides(3, 15)? Yes! → true
```

**Why this is useful:** Bounded search for small factors is practical and efficient.

---

## Example 4: Satisfiability / Proof Search

### Problem: Find a satisfying assignment for a small propositional formula

**Scenario:** Given φ(x), check if x=true or x=false satisfies it.

```logi
// φ(x) = (x ∨ q) where q is some proposition
// Is there an assignment to x that makes φ true?
satisfiable = phi => exists (
    (true => (phi true)) |
    (false => (phi false))
)

// Example: φ(x) = (x ∨ false)
example_formula = X => (or X false)

theorem (satisfiable example_formula)
// Check: example_formula(true) = true ✓
// Result: true
```

**Why this is useful:** Exhaustive search over finite boolean domain.

---

## Example 5: Type Checking / Pattern Matching

### Problem: Does a value match any of several expected types/shapes?

**Scenario:** Check if a term has one of several valid forms.

```logi
// Check if term is a valid arithmetic expression
// (either addition or multiplication)
is_arith = term => exists (
    (add => (match_pattern term (add (var A) (var B)))) |
    (mul => (match_pattern term (mul (var A) (var B))))
)

// Test: is_arith (add two three) → true
// Test: is_arith (if true x y) → false
```

**Why this is useful:** Bounded type/shape checking for DSLs.

---

## Example 6: Game Theory / Strategy Selection

### Problem: Does at least one strategy win?

**Scenario:** In a game, check if player has a winning move.

```logi
// In tic-tac-toe, does player have a winning move in {pos1, pos2}?
has_winning_move = board => player => exists (
    (pos1 => (wins_after_move board player pos1)) |
    (pos2 => (wins_after_move board player pos2))
)

// Usage:
theorem (has_winning_move current_board alice)
```

**Why this is useful:** Bounded search through small set of candidate moves.

---

## Example 7: Cryptographic Key Search

### Problem: Does at least one of these keys decrypt the message?

**Scenario:** Try a small set of candidate keys.

```logi
// Check if any known key decrypts successfully
has_valid_key = ciphertext => expected_plaintext => exists (
    (key1 => (eq (decrypt ciphertext key1) expected_plaintext)) |
    (key2 => (eq (decrypt ciphertext key2) expected_plaintext)) |
    (key3 => (eq (decrypt ciphertext key3) expected_plaintext))
)
```

**Why this is useful:** Bounded key search (e.g., trying known backup keys).

---

## Example 8: Compiler / Optimization Selection

### Problem: Does at least one optimization reduce code size?

**Scenario:** Try multiple optimization strategies, succeed if any improves the code.

```logi
// Check if any optimization improves the program
has_improvement = program => exists (
    (inline => (reduces_size (optimize_inline program) program)) |
    (const_fold => (reduces_size (optimize_const_fold program) program)) |
    (dead_code => (reduces_size (optimize_dead_code program) program))
)
```

**Why this is useful:** Try multiple optimization passes, accept if any helps.

---

## Example 9: Proof by Cases (Small Finite Domain)

### Problem: Prove property holds by checking all cases

**Scenario:** Prove a predicate over a small finite domain.

```logi
// The predicate "is_small_prime" holds for some element of {2, 3, 5}
has_small_prime = exists (
    (two => (isprime two)) |
    (three => (isprime three)) |
    (five => (isprime five))
)

// All are prime, so this is true
theorem has_small_prime
```

**Why this is useful:** Constructive proof with explicit witnesses for small domains.

---

## Example 10: Network Reachability

### Problem: Can we reach destination via at least one gateway?

**Scenario:** Check if target is reachable through any of several gateways.

```logi
// Check if destination is reachable via gateway1 or gateway2
is_reachable = source => dest => exists (
    (gateway1 => (and (connected source gateway1) (connected gateway1 dest))) |
    (gateway2 => (and (connected source gateway2) (connected gateway2 dest)))
)

theorem (is_reachable my_computer internet)
```

**Why this is useful:** Multiple routing paths, succeed if any works.

---

## Best Example (Most Natural): Trial Division for Primality

### Problem: Show N is composite by trial division

**Realistic scenario:** Test if N has a small prime divisor.

```logi
// Trial division: check if N is divisible by small primes
// If yes, N is composite (not prime)
has_small_divisor = N => exists (
    (two => (divides two N)) |
    (three => (divides three N)) |
    (five => (divides five N)) |
    (seven => (divides seven N)) |
    (eleven => (divides eleven N)) |
    (thirteen => (divides thirteen N))
)

// Prove 21 is composite
theorem (has_small_divisor twentyone)
// Evaluates: divides(3, 21)? Yes!
// Result: true ✓
```

**Why this is the best example:**
1. **Natural bounded search** - We only test small divisors
2. **Practical algorithm** - This is how you actually check primality
3. **Constructive witness** - Finding a divisor proves compositeness
4. **Early exit** - As soon as one divisor is found, we're done
5. **Real-world use** - This is trial division!

---

## Most Elegant Example: SAT Solving (Small Formulas)

### Problem: Is formula satisfiable?

```logi
// Formula: (x ∨ ¬y) ∧ (¬x ∨ y)
// Check if there's a satisfying assignment for (x, y) ∈ {true, false}²

formula = X => Y => (and (or X (not Y)) (or (not X) Y))

// Is there any assignment that satisfies it?
is_sat = exists (
    (tt => (exists ((tt2 => (formula true true)) | (ff2 => (formula true false))))) |
    (ff => (exists ((tt2 => (formula false true)) | (ff2 => (formula false false)))))
)

// Or more directly (2-tuple):
is_sat_direct = exists (
    (true_true => (formula true true)) |
    (true_false => (formula true false)) |
    (false_true => (formula false true)) |
    (false_false => (formula false false))
)

// Result: true (satisfied by x=true, y=true)
```

**Why this is elegant:**
- Exhaustive search over finite domain
- Fundamental problem (SAT)
- Constructive witness

---

## The Pattern

### All good examples share:

1. **Finite candidate set** - Small, explicit set of possibilities
2. **At least one must work** - Disjunctive success condition
3. **Each candidate is testable** - Can check each one
4. **Bounded search** - Practical, terminates quickly
5. **Constructive** - Finding one witness is sufficient

### Common domains:

- **Small primes** {2, 3, 5, 7, ...}
- **Boolean values** {true, false}
- **Strategy choices** {strategy1, strategy2, ...}
- **Candidate solutions** {x1, x2, x3}
- **Routing paths** {gateway1, gateway2, ...}

---

## Why Contrived Examples Fail

**Bad example:**
```logi
exists ((two => (iseven two)) | (four => (iseven four)))
```

**Why it's bad:**
- We ALREADY KNOW 2 and 4 are even
- Not checking anything interesting
- Just encoding `true ∨ true`
- No actual search or discovery

**Good example:**
```logi
has_small_divisor = N => exists (
    (two => (divides two N)) |
    (three => (divides three N)) |
    (five => (divides five N))
)
```

**Why it's good:**
- We DON'T KNOW which (if any) divides N
- Actually computing something
- Useful algorithm (trial division)
- Real discovery process

---

## Recommendation for Documentation

Use **trial division** as the canonical example:

```logi
// Check if N has a small prime factor
is_composite = N => exists (
    (two => (divides two N)) |
    (three => (divides three N)) |
    (five => (divides five N)) |
    (seven => (divides seven N))
)

// Example: Is 15 composite?
theorem (is_composite fifteen)
// Evaluates: divides(2, 15)? No. divides(3, 15)? Yes! → true ✓

// Example: Is 13 composite?  
theorem (is_composite thirteen)
// Evaluates: divides(2, 13)? No. ... divides(7, 13)? No. → false ✗
```

**This is:**
- Natural ✓
- Practical ✓
- Non-trivial ✓
- Shows bounded search ✓
- Demonstrates early exit ✓
- Real algorithm ✓

---

End of Document.
