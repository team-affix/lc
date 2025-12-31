# Logi: Use Cases Beyond Logic

## Overview

While Logi is designed for logic verification, **it's actually a complete programming language**. The pattern matching, currying, and quotation make it suitable for many computational tasks beyond theorem proving.

---

## Category 1: Pure Computation

### Data Structure Manipulation

**Lists:**
```logi
// Standard list operations
map = F => (nil => nil | (cons X Xs) => (cons (F X) (map F Xs)))

filter = Pred => (nil => nil 
  | (cons X Xs) => ((Pred X) => (cons X (filter Pred Xs)) 
                    | _ => (filter Pred Xs)))

fold = F => Acc => (nil => Acc 
  | (cons X Xs) => (fold F (F Acc X) Xs))

// Usage
double = X => (mul two X)
list = (cons one (cons two (cons three nil)))
doubled = map double list
// → (cons two (cons four (cons six nil)))
```

**Trees:**
```logi
// Binary tree
leaf = X => (leaf X)
node = L => R => (node L R)

// Tree map
tree_map = F => (
  (leaf X) => (leaf (F X))
  | (node L R) => (node (tree_map F L) (tree_map F R)))

// Tree fold
tree_sum = (leaf X) => X
  | (node L R) => (add (tree_sum L) (tree_sum R))
```

**Dictionaries/Maps:**
```logi
// Association list
empty = nil
insert = K => V => Dict => (cons (pair K V) Dict)

lookup = K => (nil => none
  | (cons (pair K2 V) Rest) => 
      ((eq K K2) => (some V) | _ => (lookup K Rest)))

// Usage
dict = empty
dict2 = insert {name} {alice} dict
dict3 = insert {age} {30} dict2
result = lookup {name} dict3  // → (some {alice})
```

### String Processing

```logi
// String as list of chars
str_concat = Str1 => Str2 => (append Str1 Str2)

str_length = nil => zero
  | (cons _ Rest) => (succ (str_length Rest))

str_reverse = Str => (reverse_helper Str nil)

reverse_helper = nil => Acc => Acc
  | (cons C Rest) => Acc => (reverse_helper Rest (cons C Acc))

// Pattern matching on strings
starts_with = (cons C Rest) => (cons C2 _) => (eq C C2)
  | _ => _ => false
```

### Arithmetic and Math

```logi
// More sophisticated arithmetic
pow = Base => zero => one
  | Base => (succ N) => (mul Base (pow Base N))

factorial = zero => one
  | (succ N) => (mul (succ N) (factorial N))

fibonacci = zero => zero
  | one => one
  | (succ (succ N)) => (add (fibonacci (succ N)) (fibonacci N))

// GCD
gcd = X => zero => X
  | X => Y => (gcd Y (mod X Y))
```

---

## Category 2: Symbolic Computation

### Computer Algebra System

```logi
// Symbolic expressions with quotation
plus_sym = X => Y => {plus X Y}
mul_sym = X => Y => {mul X Y}

// Symbolic differentiation
deriv = Var => {Var} => one
  | Var => {_} => zero
  | Var => {plus X Y} => {plus (deriv Var X) (deriv Var Y)}
  | Var => {mul X Y} => {plus (mul X (deriv Var Y)) (mul Y (deriv Var X))}
  | Var => {pow X N} => {mul N (mul (pow X (sub N one)) (deriv Var X))}

// Usage: d/dx (x^2 + x)
expr = {plus (pow {x} two) {x}}
derivative = deriv {x} expr
// → {plus (mul two (mul (pow {x} one) one)) one}

// Simplification
simplify = {plus X zero} => X
  | {plus zero X} => X
  | {mul X one} => X
  | {mul one X} => X
  | {mul X zero} => zero
  | {mul zero X} => zero
  | {plus {plus X Y} Z} => {plus X (plus Y Z)}
  | X => X
```

### Expression Evaluation

```logi
// Evaluate symbolic expressions with environment
eval = Env => {Var} => (lookup Var Env)
  | Env => {plus X Y} => (add (eval Env {X}) (eval Env {Y}))
  | Env => {mul X Y} => (mul (eval Env {X}) (eval Env {Y}))
  | Env => {N} => N  // Numbers evaluate to themselves

// Usage
env = (insert {x} three (insert {y} five empty))
expr = {plus (mul {x} {y}) {x}}
result = eval env expr  // → 18  (3*5 + 3)
```

---

## Category 3: Interpreters and Compilers

### Lambda Calculus Interpreter

```logi
// Lambda calculus AST (using quotation)
var = X => {var X}
lam = X => Body => {lam X Body}
app = F => Arg => {app F Arg}

// Substitution
subst = {var X} => VarName => Val => 
  ((eq X VarName) => Val | _ => {var X})
  | {lam X Body} => VarName => Val =>
      ((eq X VarName) => {lam X Body}  // Shadowing
       | _ => {lam X (subst Body VarName Val)})
  | {app F Arg} => VarName => Val =>
      {app (subst F VarName Val) (subst Arg VarName Val)}

// Beta reduction
reduce = {app {lam X Body} Arg} => (subst Body X Arg)
  | {app F Arg} => {app (reduce F) Arg}
  | {lam X Body} => {lam X (reduce Body)}
  | X => X

// Normalize to normal form
normalize = Expr => 
  (let Reduced = (reduce Expr) in
   ((eq Expr Reduced) => Expr | _ => (normalize Reduced)))
```

### Simple Stack Machine

```logi
// Stack machine instructions
push = N => {push N}
add_inst = {add}
mul_inst = {mul}

// Execute instructions
exec = nil => Stack => Stack
  | (cons {push N} Rest) => Stack => 
      (exec Rest (cons N Stack))
  | (cons {add} Rest) => (cons X (cons Y Stack)) =>
      (exec Rest (cons (add X Y) Stack))
  | (cons {mul} Rest) => (cons X (cons Y Stack)) =>
      (exec Rest (cons (mul X Y) Stack))

// Usage: (3 + 5) * 2
program = (cons {push three} 
          (cons {push five} 
          (cons {add} 
          (cons {push two} 
          (cons {mul} nil)))))

result = exec program nil
// → (cons sixteen nil)
```

---

## Category 4: Domain-Specific Languages

### Query Language

```logi
// Simple query DSL
select = Fields => From => Where => {query Fields From Where}

// Sample data
people = (cons (record {name} {alice} {age} {30})
         (cons (record {name} {bob} {age} {25})
         (cons (record {name} {charlie} {age} {30}) nil)))

// Query execution
run_query = {query Fields From Where} =>
  (let Filtered = (filter Where From) in
   (map (project Fields) Filtered))

project = Fields => Record => (map (get_field Record) Fields)

// Usage
query = (select (cons {name} nil) 
                people 
                (R => (eq (get_field R {age}) {30})))

results = run_query query
// → (cons {alice} (cons {charlie} nil))
```

### Configuration Language

```logi
// Configuration DSL with validation
config = Settings => {config Settings}

// Validation rules
validate_port = P => ((gt P zero) => ((lt P {65536}) => true | _ => false) | _ => false)

validate_config = {config Settings} =>
  (all validate_setting Settings)

validate_setting = (setting {port} Value) => (validate_port Value)
  | (setting {host} Value) => true  // Host always valid
  | _ => false

// Usage
my_config = (config (cons (setting {port} {8080})
                     (cons (setting {host} {localhost}) nil)))

is_valid = validate_config my_config  // → true
```

---

## Category 5: Meta-Programming and Macros

### Macro System

```logi
// Define macros that transform code
defmacro = Name => Pattern => Expansion => (macro Name Pattern Expansion)

// Macro: unless condition body = if (not condition) body
unless_macro = (macro {unless}
                     ({unless Cond Body})
                     ({if (not Cond) Body}))

// Macro expansion
expand = {unless Cond Body} => {if (not Cond) Body}
  | {if Cond Then} => {if Cond Then}  // Base forms
  | {Func Arg} => {(expand Func) (expand Arg)}
  | X => X

// Usage
code = {unless (eq X zero) (print {nonzero})}
expanded = expand code
// → {if (not (eq X zero)) (print {nonzero})}
```

### Code Generation

```logi
// Generate repetitive code
gen_getter = Field => {lambda R (get_field R Field)}

gen_setter = Field => {lambda R V (set_field R Field V)}

// Generate accessor methods for record fields
gen_accessors = Fields =>
  (map gen_getter Fields)  // Returns list of getter functions

// Usage
fields = (cons {name} (cons {age} (cons {email} nil)))
getters = gen_accessors fields
// → List of three getter functions
```

---

## Category 6: Program Verification (Hybrid Logic + Computation)

### Certified Sorting

```logi
// Sort implementation
insert_sort = nil => nil
  | (cons X Xs) => (insert X (insert_sort Xs))

insert = X => nil => (cons X nil)
  | X => (cons Y Ys) => 
      ((lte X Y) => (cons X (cons Y Ys))
       | _ => (cons Y (insert X Ys)))

// Sortedness predicate
sorted = nil => true
  | (cons _ nil) => true
  | (cons X (cons Y Rest)) =>
      ((lte X Y) => (sorted (cons Y Rest)) | _ => false)

// Proof that sorting preserves elements (sketch)
preserves_elem = Elem => List => 
  theorem (iff (member Elem List) 
               (member Elem (insert_sort List)))

// Proof that result is sorted
produces_sorted = List =>
  theorem (sorted (insert_sort List))
```

### Verified Stack Implementation

```logi
// Stack operations
empty_stack = nil
push_stack = X => Stack => (cons X Stack)
pop_stack = (cons X Stack) => (pair X Stack)
  | nil => none

// Stack invariant: size is correct
stack_size = nil => zero
  | (cons _ Rest) => (succ (stack_size Rest))

// Theorem: push increases size by 1
push_theorem = X => Stack =>
  theorem (eq (stack_size (push_stack X Stack))
              (succ (stack_size Stack)))

// Theorem: push then pop returns original
push_pop_theorem = X => Stack =>
  theorem (eq (pop_stack (push_stack X Stack))
              (pair X Stack))
```

---

## Category 7: Type Systems as Values

### Simple Type Checker

```logi
// Types as data
type_int = {int}
type_bool = {bool}
type_arrow = T1 => T2 => {arrow T1 T2}

// Type environment
extend = Env => Var => Type => (cons (pair Var Type) Env)

// Type checking
typecheck = Env => {var X} => (lookup X Env)
  | Env => {lam X Body} =>
      // Need to infer type of X somehow
      (let TyX = {???} in
       (let TyBody = (typecheck (extend Env X TyX) Body) in
        {arrow TyX TyBody}))
  | Env => {app F Arg} =>
      (let TyF = (typecheck Env F) in
       (let TyArg = (typecheck Env Arg) in
        (match_arrow TyF TyArg)))

match_arrow = {arrow TyArg TyRes} => TyArg2 =>
  ((ty_eq TyArg TyArg2) => TyRes | _ => type_error)

ty_eq = {int} => {int} => true
  | {bool} => {bool} => true
  | {arrow T1 T2} => {arrow T3 T4} =>
      ((ty_eq T1 T3) => (ty_eq T2 T4) | _ => false)
  | _ => _ => false
```

### Refinement Types

```logi
// Refinement types: {x: Int | x > 0}
refine = BaseType => Pred => {refine BaseType Pred}

// Check if value satisfies refinement
satisfies = Value => {refine BaseType Pred} =>
  ((Pred Value) => true | _ => false)

// Type of positive integers
pos_int = {refine {int} (X => (gt X zero))}

// Prove that a function preserves positivity
preserves_pos = Func =>
  theorem (forall (X => 
    (imp (satisfies X pos_int)
         (satisfies (Func X) pos_int))))
```

---

## Category 8: Theorem Proving Tactics

### Proof Search

```logi
// Automatic proof search with depth limit
auto_prove = Goal => Axioms => Depth =>
  ((eq Depth zero) => none
   | _ => (try_rules Goal Axioms Depth))

try_rules = Goal => nil => Depth => none
  | Goal => (cons Rule Rest) => Depth =>
      (let Result = (try_apply Rule Goal Axioms (sub Depth one)) in
       ((is_some Result) => Result 
        | _ => (try_rules Goal Rest Depth)))

// Try to apply modus ponens
try_mp = Goal => Axioms =>
  (find_impl Goal Axioms)

find_impl = Goal => nil => none
  | Goal => (cons (theorem {imp P Q}) Rest) =>
      ((eq Q Goal) => 
        (let ProofP = (auto_prove P Rest zero) in
         ((is_some ProofP) => (some (mp (theorem {imp P Q}) ProofP))
          | _ => (find_impl Goal Rest)))
       | _ => (find_impl Goal Rest))
```

### Tactic Combinators

```logi
// Tactic language
tac_id = Goal => (some Goal)

tac_fail = Goal => none

tac_seq = Tac1 => Tac2 => Goal =>
  (let Result1 = (Tac1 Goal) in
   ((is_some Result1) => (Tac2 (unwrap Result1))
    | _ => none))

tac_or = Tac1 => Tac2 => Goal =>
  (let Result1 = (Tac1 Goal) in
   ((is_some Result1) => Result1
    | _ => (Tac2 Goal)))

// Usage
solve = (tac_or (tac_seq simplify auto)
                (tac_seq split (tac_seq auto auto)))
```

---

## Category 9: Parser Combinators

```logi
// Parser type: String -> Option (Result, Rest)
parser = F => {parser F}

// Basic parsers
parse_char = C => {parser (lambda Str =>
  ((starts_with Str C) => (some (pair C (tail Str)))
   | _ => none))}

parse_digit = {parser (lambda Str =>
  (let C = (head Str) in
   ((is_digit C) => (some (pair C (tail Str)))
    | _ => none)))}

// Parser combinators
p_seq = P1 => P2 => {parser (lambda Str =>
  (let R1 = (run_parser P1 Str) in
   ((is_some R1) =>
     (let (pair V1 Rest1) = (unwrap R1) in
      (let R2 = (run_parser P2 Rest1) in
       ((is_some R2) =>
         (let (pair V2 Rest2) = (unwrap R2) in
          (some (pair (pair V1 V2) Rest2)))
        | _ => none)))
    | _ => none)))}

p_or = P1 => P2 => {parser (lambda Str =>
  (let R1 = (run_parser P1 Str) in
   ((is_some R1) => R1
    | _ => (run_parser P2 Str))))}

p_many = P => {parser (lambda Str =>
  (let R = (run_parser P Str) in
   ((is_some R) =>
     (let (pair V Rest) = (unwrap R) in
      (let (pair Vs Rest2) = (run_parser (p_many P) Rest) in
       (pair (cons V Vs) Rest2)))
    | _ => (pair nil Str))))}

// Build parsers
digit_parser = parse_digit
number_parser = (p_many digit_parser)
```

---

## Category 10: Game Logic and AI

### Game State Management

```logi
// Tic-tac-toe
empty_board = (board 
  (row empty empty empty)
  (row empty empty empty)
  (row empty empty empty))

make_move = Player => Row => Col => Board =>
  (update_board Board Row Col Player)

check_winner = Board =>
  ((check_rows Board) => (some (winner (check_rows Board)))
   | (check_cols Board) => (some (winner (check_cols Board)))
   | (check_diags Board) => (some (winner (check_diags Board)))
   | _ => none)

// Minimax AI
minimax = Board => Player => Depth =>
  ((check_winner Board) => (score (check_winner Board))
   | (eq Depth zero) => zero
   | _ => (best_move Board Player Depth))

best_move = Board => Player => Depth =>
  (let Moves = (valid_moves Board) in
   (maximize Player 
            (map (score_move Board Player Depth) Moves)))
```

---

## Category 11: Concurrency Patterns (Theoretical)

```logi
// Process calculus (π-calculus style)
proc_send = Chan => Val => {send Chan Val}
proc_recv = Chan => {recv Chan}
proc_par = P => Q => {par P Q}
proc_new = Chan => Body => {new Chan Body}

// Trace execution
trace = {send C V} => Env => 
  (cons (event {send} C V) (update_channel Env C V))
  | {recv C} => Env =>
      (let V = (read_channel Env C) in
       (cons (event {recv} C V) Env))
  | {par P Q} => Env =>
      (interleave (trace P Env) (trace Q Env))
  | {new C Body} => Env =>
      (trace Body (add_channel Env C))
```

---

## Category 12: Testing and Property-Based Testing

```logi
// QuickCheck-style property testing
forall_test = Gen => Prop => NumTests =>
  (test_loop Gen Prop NumTests zero)

test_loop = Gen => Prop => Total => Current =>
  ((eq Current Total) => (pass Total)
   | _ => (let Value = (Gen Current) in
          (let Result = (Prop Value) in
           ((Result) => (test_loop Gen Prop Total (succ Current))
            | _ => (fail Current Value)))))

// Generators
gen_nat = N => N
gen_list = N => (gen_list_helper N nil)
gen_list_helper = zero => Acc => Acc
  | (succ N) => Acc => (gen_list_helper N (cons N Acc))

// Property: reverse . reverse = id
prop_reverse = List =>
  (eq List (reverse (reverse List)))

// Run test
test_result = forall_test gen_list prop_reverse {100}
```

---

## Category 13: Functional Reactive Programming

```logi
// Event streams
stream = Events => {stream Events}

map_stream = F => {stream Events} =>
  {stream (map F Events)}

filter_stream = Pred => {stream Events} =>
  {stream (filter Pred Events)}

merge_streams = {stream E1} => {stream E2} =>
  {stream (merge E1 E2)}

// Behaviors (time-varying values)
behavior = InitVal => Stream => {behavior InitVal Stream}

lift = F => {behavior V1 S1} => {behavior V2 S2} =>
  {behavior (F V1 V2) (map_stream F (merge_streams S1 S2))}

// Example: Mouse position
mouse_pos = {behavior (pair zero zero) mouse_events}
click_count = {behavior zero (filter_stream is_click mouse_events)}
```

---

## Summary: The Power of Logi

### As a General-Purpose Language

Logi can handle:
- ✅ Data structure manipulation (lists, trees, graphs)
- ✅ String processing
- ✅ Arithmetic and algorithms
- ✅ Functional programming patterns
- ✅ Recursion and higher-order functions

### With Quotation

Logi excels at:
- ✅ Symbolic computation (algebra, calculus)
- ✅ Meta-programming (macros, code generation)
- ✅ Building interpreters and compilers
- ✅ DSL implementation
- ✅ Term rewriting systems

### Unique Strength: Logic + Computation

Logi uniquely combines:
- ✅ Write normal programs
- ✅ Prove properties about those programs
- ✅ Use the same language for both
- ✅ No artificial separation

### Comparison to Other Languages

**vs. Haskell/ML:**
- Similar: Pattern matching, currying, purity
- Different: Simpler (no type system), theorem marker, quotation for meta-programming

**vs. Prolog:**
- Similar: Logic-oriented, pattern matching
- Different: Deterministic, no backtracking, first-class functions

**vs. Coq/Agda:**
- Similar: Proofs and programs together
- Different: Much simpler, no dependent types, propositions as data not types

**vs. Lisp:**
- Similar: Code as data (quotation), meta-programming
- Different: Pattern matching, logic-oriented, theorem tracking

---

## Recommended Use Cases

### Ideal For:

1. **Logic verification** - Original purpose
2. **Symbolic computation** - CAS, differentiation
3. **DSL implementation** - Query languages, config, rules
4. **Teaching** - Simple semantics, clear
5. **Certified code** - Programs with proofs
6. **Term rewriting** - Pattern matching is powerful
7. **Meta-programming** - Quotation enables reflection

### Not Ideal For:

1. **Performance-critical** - Interpreted, not optimized
2. **Large systems** - No module system yet
3. **I/O-heavy** - Pure functional, no side effects
4. **Systems programming** - No low-level access

---

## Example: Complete Mini-Project

### A Verified Evaluator for Arithmetic Expressions

```logi
// AST
const = N => {const N}
plus = E1 => E2 => {plus E1 E2}
times = E1 => E2 => {times E1 E2}

// Evaluator
eval = {const N} => N
  | {plus E1 E2} => (add (eval E1) (eval E2))
  | {times E1 E2} => (mul (eval E1) (eval E2))

// Simplifier
simp = {plus {const zero} E} => (simp E)
  | {plus E {const zero}} => (simp E)
  | {times {const one} E} => (simp E)
  | {times E {const one}} => (simp E)
  | {times {const zero} E} => {const zero}
  | {plus E1 E2} => {plus (simp E1) (simp E2)}
  | {times E1 E2} => {times (simp E1) (simp E2)}
  | E => E

// Theorem: simplification preserves meaning
simp_correct = E =>
  theorem (eq (eval (simp E)) (eval E))

// Test
expr = {plus {times {const two} {const three}} {const zero}}
simplified = simp expr  // → {times {const two} {const three}}
value = eval simplified  // → six
proof = simp_correct expr  // → theorem (eq six six)
```

**This combines:**
- Computation (eval, simp)
- Symbolic manipulation (quotation)
- Verification (simp_correct theorem)

---

**Logi is a true hybrid: a full programming language that can also reason about logic!**
