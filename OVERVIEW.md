# Logi Language: Visual Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                  THE LOGI LANGUAGE                              │
│                                                                 │
│  A minimal logic verification language that unifies            │
│  computation and proof through pattern matching                │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  SYNTAX (7 Constructs)                                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. atom          // lowercase: true, false, imp                │
│  2. Var           // uppercase: X, Y, P, Q                      │
│  3. M M           // application (left-assoc)                   │
│  4. p => M        // function (right-assoc)                     │
│  5. M | M         // choice (binary infix, right-assoc)         │
│  6. theorem M     // theorem marker                             │
│  7. {M}           // quotation (prevent reduction)              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  KEY PRINCIPLES                                                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  • One pattern per arrow (curry for multiple args)             │
│  • Uppercase = variables, lowercase = atoms                     │
│  • Pipe | is binary infix (not prefix)                         │
│  • Partial functions natural (no match = no error)             │
│  • Quotation {M} prevents reduction                            │
│  • Arrow is => (not ->)                                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  EXAMPLE                                                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  // Boolean NOT                                                 │
│  not = true => false | false => true                           │
│                                                                 │
│  // Modus Ponens (curried)                                     │
│  mp = (theorem (imp P Q)) => (theorem P) => theorem Q          │
│                                                                 │
│  // Axioms                                                      │
│  ax1 = theorem (imp p q)                                       │
│  ax2 = theorem p                                               │
│                                                                 │
│  // Derive Q                                                    │
│  result = mp ax1 ax2    // → theorem q  ✓                     │
│                                                                 │
│  // Quotation (prevent reduction)                              │
│  term = {not true}      // → {not true} (frozen)              │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  DOCUMENTATION MAP                                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Entry Points:                                                  │
│  ├─ START_HERE.md .......................... Main entry         │
│  ├─ INDEX.md ............................... Alt index          │
│  └─ FILES_SUMMARY.md ....................... This overview      │
│                                                                 │
│  Learning (⭐⭐⭐ = Essential):                                   │
│  ├─ SYNTAX_REFERENCE.md ⭐⭐⭐ ................ Quick ref         │
│  ├─ QUICKSTART.md ⭐⭐⭐ ....................... Intro            │
│  ├─ LANGUAGE_SUMMARY.md ⭐⭐ .................. Summary          │
│  ├─ CORRECT_EXAMPLES.md ⭐ ................... Examples         │
│  └─ EXAMPLES.md ⭐⭐ .......................... Full examples    │
│                                                                 │
│  Reference:                                                     │
│  ├─ FORMAL_SPECIFICATION.md ⭐⭐⭐ ............ Authoritative    │
│  ├─ QUOTATION_CURLY.md ...................... Quotation        │
│  ├─ CONSTRUCTORS_VS_QUOTES.md ............... Conventions      │
│  └─ IMPLEMENTATION_GUIDE.md ................. How to build     │
│                                                                 │
│  Applications:                                                  │
│  ├─ USE_CASES.md ............................ 13 use cases     │
│  └─ FINAL_SUMMARY.md ........................ Evolution        │
│                                                                 │
│  Historical (⚠️ May show rejected syntax):                      │
│  ├─ RECONSIDERING.md ........................ Design debate     │
│  ├─ REVISED_THEORY.md ....................... Alternatives     │
│  ├─ ARROW_SEMANTICS.md ...................... Rejected         │
│  ├─ CLEAN_SYNTAX.md ......................... Rejected         │
│  ├─ CORRECTED_SYNTAX.md ..................... Transitional     │
│  └─ FINAL_DESIGN.md ......................... Pre-quotation    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  CORRECT SYNTAX (Reference)                                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Arrow:                                                         │
│    ✅ =>     (correct)                                          │
│    ❌ ->     (wrong)                                            │
│                                                                 │
│  Pipe:                                                          │
│    ✅ case1 => body1 | case2 => body2  (binary infix)          │
│    ❌ { | case1 => body1 }             (prefix - old)          │
│                                                                 │
│  Curly Braces:                                                  │
│    ✅ {not true}                        (quotation)             │
│    ❌ { X => X }                        (pattern match - old)   │
│                                                                 │
│  Multiple Arguments:                                            │
│    ✅ X => Y => body                    (curried)               │
│    ❌ X Y => body                       (multi-pattern - old)   │
│                                                                 │
│  Case:                                                          │
│    ✅ Uppercase vars (X, Y, P, Q)                               │
│    ✅ Lowercase atoms (true, false, imp)                        │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  IMPLEMENTATION STATUS                                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Theory:              ✅ Complete                               │
│  Formal Spec:         ✅ Complete                               │
│  Examples:            ✅ Complete (20+)                         │
│  Implementation:      ⬜ Not yet built                          │
│                                                                 │
│  Next Step: Build interpreter (~1000 LOC, 2-3 weeks)          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│  START READING HERE                                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. START_HERE.md ............................ Entry point      │
│  2. SYNTAX_REFERENCE.md ..................... Learn syntax      │
│  3. QUICKSTART.md ........................... Learn language    │
│  4. FORMAL_SPECIFICATION.md ................. Full reference    │
│  5. EXAMPLES.md ............................. See it work       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
