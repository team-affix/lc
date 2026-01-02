# Logi: Implementation Guide

## Quick Reference

**Start here if you want to implement Logi!**

This guide provides concrete steps to build the Logi interpreter from scratch.

---

## Overview

**What you're building:** An interpreter for the Logi logic verification language.

**Core features:**
- Pattern matching for all functions
- Currying (one pattern per arrow)
- Partial functions (no match = undefined, not error)
- Theorem tracking for proof verification
- Static axiom detection

**Estimated effort:** 800-1000 lines of code, 2-3 weeks

---

## Prerequisites

### Knowledge Required

- **Functional programming basics** (functions, recursion, pattern matching)
- **Parsing fundamentals** (tokenization, AST construction, precedence)
- **Semantics** (substitution, reduction, evaluation)

### Recommended Languages

**Best choices:**
1. **Python** - Fast prototyping, easy testing (recommended for first implementation)
2. **OCaml** - Elegant, pattern matching native, ML-family
3. **Haskell** - Pure functional, great for language implementation
4. **Rust** - Performance + safety, good for production

**This guide uses Python** for examples, but translates easily to other languages.

---

## Architecture Overview

```
┌─────────────┐
│   Source    │
│   Code      │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Lexer     │  Tokenize into atoms, vars, keywords, operators
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Parser    │  Build AST with correct precedence
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   AST       │  Abstract syntax tree
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Evaluator  │  Pattern match + reduce
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   Result    │  Normal form or value
└─────────────┘
```

---

## Phase 1: Data Structures

### AST Node Types

```python
from dataclasses import dataclass
from typing import Union, List

# Terms
@dataclass
class Atom:
    name: str  # lowercase identifier

@dataclass
class Var:
    name: str  # uppercase identifier

@dataclass
class App:
    func: 'Term'
    arg: 'Term'

@dataclass
class Function:
    pattern: 'Pattern'
    body: 'Term'

@dataclass
class Choice:
    left: 'Term'
    right: 'Term'

@dataclass
class Theorem:
    inner: 'Term'

Term = Union[Atom, Var, App, Function, Choice, Theorem]

# Patterns
@dataclass
class PatAtom:
    atom: str

@dataclass
class PatVar:
    var: str

@dataclass
class PatWild:
    pass

@dataclass
class PatApp:
    func: 'Pattern'
    arg: 'Pattern'

@dataclass
class PatTheorem:
    inner: 'Pattern'

Pattern = Union[PatAtom, PatVar, PatWild, PatApp, PatTheorem]
```

---

## Phase 2: Lexer

### Token Types

```python
from enum import Enum

class TokenType(Enum):
    ATOM = "ATOM"           # lowercase identifier
    VAR = "VAR"             # uppercase identifier
    ARROW = "ARROW"         # =>
    PIPE = "PIPE"           # |
    LPAREN = "LPAREN"       # (
    RPAREN = "RPAREN"       # )
    WILDCARD = "WILDCARD"   # _
    THEOREM = "THEOREM"     # theorem
    EQUALS = "EQUALS"       # =
    EOF = "EOF"

@dataclass
class Token:
    type: TokenType
    value: str
    line: int
    column: int
```

### Lexer Implementation

```python
import re

class Lexer:
    def __init__(self, source: str):
        self.source = source
        self.pos = 0
        self.line = 1
        self.column = 1
    
    def current_char(self):
        if self.pos >= len(self.source):
            return None
        return self.source[self.pos]
    
    def advance(self):
        if self.current_char() == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        self.pos += 1
    
    def skip_whitespace(self):
        while self.current_char() in ' \t\n\r':
            self.advance()
    
    def skip_comment(self):
        if self.current_char() == '/' and self.peek() == '/':
            # Line comment
            while self.current_char() != '\n' and self.current_char() is not None:
                self.advance()
        elif self.current_char() == '/' and self.peek() == '*':
            # Block comment
            self.advance()  # skip /
            self.advance()  # skip *
            while not (self.current_char() == '*' and self.peek() == '/'):
                self.advance()
            self.advance()  # skip *
            self.advance()  # skip /
    
    def peek(self):
        if self.pos + 1 >= len(self.source):
            return None
        return self.source[self.pos + 1]
    
    def read_identifier(self):
        start_pos = self.pos
        while self.current_char() and (self.current_char().isalnum() or self.current_char() == '_'):
            self.advance()
        return self.source[start_pos:self.pos]
    
    def next_token(self) -> Token:
        while self.current_char():
            # Skip whitespace and comments
            if self.current_char() in ' \t\n\r':
                self.skip_whitespace()
                continue
            
            if self.current_char() == '/' and self.peek() in '/*':
                self.skip_comment()
                continue
            
            # Save position for token
            line = self.line
            col = self.column
            
            # Two-character tokens
            if self.current_char() == '=' and self.peek() == '>':
                self.advance()
                self.advance()
                return Token(TokenType.ARROW, '=>', line, col)
            
            # Single-character tokens
            if self.current_char() == '|':
                self.advance()
                return Token(TokenType.PIPE, '|', line, col)
            
            if self.current_char() == '(':
                self.advance()
                return Token(TokenType.LPAREN, '(', line, col)
            
            if self.current_char() == ')':
                self.advance()
                return Token(TokenType.RPAREN, ')', line, col)
            
            if self.current_char() == '=':
                self.advance()
                return Token(TokenType.EQUALS, '=', line, col)
            
            if self.current_char() == '_':
                self.advance()
                return Token(TokenType.WILDCARD, '_', line, col)
            
            # Identifiers and keywords
            if self.current_char().isalpha():
                ident = self.read_identifier()
                
                if ident == 'theorem':
                    return Token(TokenType.THEOREM, ident, line, col)
                elif ident[0].isupper():
                    return Token(TokenType.VAR, ident, line, col)
                else:
                    return Token(TokenType.ATOM, ident, line, col)
            
            raise Exception(f"Unexpected character: {self.current_char()} at {line}:{col}")
        
        return Token(TokenType.EOF, '', self.line, self.column)
    
    def tokenize(self) -> List[Token]:
        tokens = []
        while True:
            token = self.next_token()
            tokens.append(token)
            if token.type == TokenType.EOF:
                break
        return tokens
```

---

## Phase 3: Parser

### Precedence

**From lowest to highest:**
1. `|` (choice) - right-associative
2. `=>` (function) - right-associative
3. Application - left-associative

### Parser Implementation

```python
class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0
    
    def current_token(self) -> Token:
        if self.pos >= len(self.tokens):
            return self.tokens[-1]  # EOF
        return self.tokens[self.pos]
    
    def advance(self):
        self.pos += 1
    
    def expect(self, token_type: TokenType) -> Token:
        token = self.current_token()
        if token.type != token_type:
            raise Exception(f"Expected {token_type}, got {token.type}")
        self.advance()
        return token
    
    # Parse program (list of definitions)
    def parse_program(self):
        definitions = []
        while self.current_token().type != TokenType.EOF:
            definitions.append(self.parse_definition())
        return definitions
    
    # Parse definition: name = term
    def parse_definition(self):
        name_token = self.current_token()
        if name_token.type not in [TokenType.ATOM, TokenType.VAR]:
            raise Exception(f"Expected identifier, got {name_token.type}")
        name = name_token.value
        self.advance()
        
        self.expect(TokenType.EQUALS)
        
        term = self.parse_term()
        
        return (name, term)
    
    # Parse term (lowest precedence: choice |)
    def parse_term(self) -> Term:
        return self.parse_choice()
    
    # Choice: term | term (right-associative)
    def parse_choice(self) -> Term:
        left = self.parse_function()
        
        if self.current_token().type == TokenType.PIPE:
            self.advance()
            right = self.parse_choice()  # Right-associative
            return Choice(left, right)
        
        return left
    
    # Function: pattern => term (right-associative)
    def parse_function(self) -> Term:
        # Try to parse as pattern => term
        # Need to be careful: could be application
        
        # Look ahead for =>
        saved_pos = self.pos
        try:
            pattern = self.parse_pattern()
            if self.current_token().type == TokenType.ARROW:
                self.advance()
                body = self.parse_function()  # Right-associative
                return Function(pattern, body)
            else:
                # Not a function, restore and parse as application
                self.pos = saved_pos
                return self.parse_application()
        except:
            # Failed to parse as pattern, try as application
            self.pos = saved_pos
            return self.parse_application()
    
    # Application (left-associative, highest precedence)
    def parse_application(self) -> Term:
        left = self.parse_atom_term()
        
        while self.current_token().type in [TokenType.ATOM, TokenType.VAR, TokenType.LPAREN, TokenType.THEOREM]:
            right = self.parse_atom_term()
            left = App(left, right)
        
        return left
    
    # Atomic term
    def parse_atom_term(self) -> Term:
        token = self.current_token()
        
        if token.type == TokenType.ATOM:
            self.advance()
            return Atom(token.value)
        
        elif token.type == TokenType.VAR:
            self.advance()
            return Var(token.value)
        
        elif token.type == TokenType.THEOREM:
            self.advance()
            inner = self.parse_atom_term()
            return Theorem(inner)
        
        elif token.type == TokenType.LPAREN:
            self.advance()
            term = self.parse_term()
            self.expect(TokenType.RPAREN)
            return term
        
        else:
            raise Exception(f"Unexpected token in term: {token.type}")
    
    # Parse pattern
    def parse_pattern(self) -> Pattern:
        return self.parse_pattern_app()
    
    # Pattern application
    def parse_pattern_app(self) -> Pattern:
        left = self.parse_atom_pattern()
        
        # Check if followed by more patterns (application)
        while self.current_token().type in [TokenType.ATOM, TokenType.VAR, TokenType.WILDCARD, TokenType.LPAREN, TokenType.THEOREM]:
            right = self.parse_atom_pattern()
            left = PatApp(left, right)
        
        return left
    
    # Atomic pattern
    def parse_atom_pattern(self) -> Pattern:
        token = self.current_token()
        
        if token.type == TokenType.ATOM:
            self.advance()
            return PatAtom(token.value)
        
        elif token.type == TokenType.VAR:
            self.advance()
            return PatVar(token.value)
        
        elif token.type == TokenType.WILDCARD:
            self.advance()
            return PatWild()
        
        elif token.type == TokenType.THEOREM:
            self.advance()
            inner = self.parse_atom_pattern()
            return PatTheorem(inner)
        
        elif token.type == TokenType.LPAREN:
            self.advance()
            pattern = self.parse_pattern()
            self.expect(TokenType.RPAREN)
            return pattern
        
        else:
            raise Exception(f"Unexpected token in pattern: {token.type}")
```

---

## Phase 4: Pattern Matcher

```python
def match_pattern(pattern: Pattern, value: Term) -> dict | None:
    """
    Try to match value against pattern.
    Returns dict of bindings on success, None on failure.
    """
    
    if isinstance(pattern, PatVar):
        # Variable pattern binds to anything
        return {pattern.var: value}
    
    elif isinstance(pattern, PatWild):
        # Wildcard matches anything, no binding
        return {}
    
    elif isinstance(pattern, PatAtom):
        # Atom pattern must match exactly
        if isinstance(value, Atom) and pattern.atom == value.name:
            return {}
        else:
            return None
    
    elif isinstance(pattern, PatApp):
        # Application pattern must match application
        if isinstance(value, App):
            left_bindings = match_pattern(pattern.func, value.func)
            if left_bindings is None:
                return None
            
            right_bindings = match_pattern(pattern.arg, value.arg)
            if right_bindings is None:
                return None
            
            # Merge bindings (check for conflicts)
            return {**left_bindings, **right_bindings}
        else:
            return None
    
    elif isinstance(pattern, PatTheorem):
        # Theorem pattern must match theorem
        if isinstance(value, Theorem):
            return match_pattern(pattern.inner, value.inner)
        else:
            return None
    
    else:
        return None
```

---

## Phase 5: Substitution

```python
def substitute(term: Term, bindings: dict) -> Term:
    """Apply bindings to term (capture-avoiding substitution)."""
    
    if isinstance(term, Atom):
        return term
    
    elif isinstance(term, Var):
        if term.name in bindings:
            return bindings[term.name]
        else:
            return term
    
    elif isinstance(term, App):
        return App(substitute(term.func, bindings),
                   substitute(term.arg, bindings))
    
    elif isinstance(term, Function):
        # Remove bound variables from bindings
        bound_vars = get_bound_vars(term.pattern)
        new_bindings = {k: v for k, v in bindings.items() if k not in bound_vars}
        
        return Function(term.pattern,
                       substitute(term.body, new_bindings))
    
    elif isinstance(term, Choice):
        return Choice(substitute(term.left, bindings),
                     substitute(term.right, bindings))
    
    elif isinstance(term, Theorem):
        return Theorem(substitute(term.inner, bindings))
    
    else:
        return term

def get_bound_vars(pattern: Pattern) -> set:
    """Get all variables bound by pattern."""
    if isinstance(pattern, PatVar):
        return {pattern.var}
    elif isinstance(pattern, PatWild):
        return set()
    elif isinstance(pattern, PatAtom):
        return set()
    elif isinstance(pattern, PatApp):
        return get_bound_vars(pattern.func) | get_bound_vars(pattern.arg)
    elif isinstance(pattern, PatTheorem):
        return get_bound_vars(pattern.inner)
    else:
        return set()
```

---

## Phase 6: Evaluator

```python
def is_value(term: Term) -> bool:
    """Check if term is a value (cannot reduce further as function)."""
    return isinstance(term, (Atom, Function, Choice, Theorem))

def reduce_step(term: Term) -> Term | None:
    """
    Perform one reduction step.
    Returns new term on success, None if no reduction possible.
    """
    
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
                # Try to match pattern
                bindings = match_pattern(term.func.pattern, term.arg)
                if bindings is not None:
                    return substitute(term.func.body, bindings)
                else:
                    return None  # No match - partial function
            
            elif isinstance(term.func, Choice):
                # Try left alternative
                left_result = reduce_step(App(term.func.left, term.arg))
                if left_result is not None:
                    return left_result
                # If left fails, try right
                return reduce_step(App(term.func.right, term.arg))
            
            else:
                return None  # Can't apply non-function
    
    elif isinstance(term, Theorem):
        # Reduce inside theorem
        if not is_value(term.inner):
            inner_reduced = reduce_step(term.inner)
            if inner_reduced is not None:
                return Theorem(inner_reduced)
    
    return None  # No reduction possible

def normalize(term: Term, max_steps: int = 1000) -> Term:
    """Reduce to normal form (with step limit)."""
    steps = 0
    while steps < max_steps:
        reduced = reduce_step(term)
        if reduced is None:
            break
        term = reduced
        steps += 1
    
    if steps >= max_steps:
        print(f"Warning: Reached step limit ({max_steps})")
    
    return term
```

---

## Phase 7: REPL

```python
class Environment:
    def __init__(self):
        self.definitions = {}
    
    def define(self, name: str, term: Term):
        self.definitions[name] = term
    
    def lookup(self, name: str) -> Term | None:
        return self.definitions.get(name)
    
    def resolve_names(self, term: Term) -> Term:
        """Replace names with their definitions."""
        if isinstance(term, Atom):
            defn = self.lookup(term.name)
            if defn:
                return defn
            return term
        
        elif isinstance(term, Var):
            return term
        
        elif isinstance(term, App):
            return App(self.resolve_names(term.func),
                      self.resolve_names(term.arg))
        
        # ... handle other cases ...
        
        return term

def repl():
    """Interactive Read-Eval-Print Loop."""
    env = Environment()
    
    print("Logi REPL")
    print("Type 'quit' to exit")
    
    while True:
        try:
            line = input(">>> ")
            
            if line.strip() == 'quit':
                break
            
            if not line.strip():
                continue
            
            # Tokenize
            lexer = Lexer(line)
            tokens = lexer.tokenize()
            
            # Parse
            parser = Parser(tokens)
            
            # Check if definition or expression
            if '=' in line:
                # Definition
                name, term = parser.parse_definition()
                env.define(name, term)
                print(f"Defined: {name}")
            else:
                # Expression
                term = parser.parse_term()
                
                # Resolve names
                resolved = env.resolve_names(term)
                
                # Normalize
                result = normalize(resolved)
                
                # Print
                print(pretty_print(result))
        
        except Exception as e:
            print(f"Error: {e}")

def pretty_print(term: Term) -> str:
    """Convert term to readable string."""
    if isinstance(term, Atom):
        return term.name
    elif isinstance(term, Var):
        return term.name
    elif isinstance(term, App):
        return f"({pretty_print(term.func)} {pretty_print(term.arg)})"
    elif isinstance(term, Function):
        return f"({pretty_print_pattern(term.pattern)} => {pretty_print(term.body)})"
    elif isinstance(term, Choice):
        return f"({pretty_print(term.left)} | {pretty_print(term.right)})"
    elif isinstance(term, Theorem):
        return f"(theorem {pretty_print(term.inner)})"
    else:
        return str(term)

def pretty_print_pattern(pattern: Pattern) -> str:
    """Convert pattern to readable string."""
    if isinstance(pattern, PatAtom):
        return pattern.atom
    elif isinstance(pattern, PatVar):
        return pattern.var
    elif isinstance(pattern, PatWild):
        return "_"
    elif isinstance(pattern, PatApp):
        return f"({pretty_print_pattern(pattern.func)} {pretty_print_pattern(pattern.arg)})"
    elif isinstance(pattern, PatTheorem):
        return f"(theorem {pretty_print_pattern(pattern.inner)})"
    else:
        return str(pattern)
```

---

## Phase 8: Testing

```python
def test_boolean_not():
    source = """
    not = true => false | false => true
    result = not true
    """
    
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    program = parser.parse_program()
    
    env = Environment()
    for name, term in program:
        env.define(name, term)
    
    result_term = env.lookup('result')
    resolved = env.resolve_names(result_term)
    normalized = normalize(resolved)
    
    assert isinstance(normalized, Atom)
    assert normalized.name == 'false'
    print("✓ Boolean NOT test passed")

def test_modus_ponens():
    source = """
    imp = P => Q => (imp P Q)
    mp = (theorem (imp P Q)) => (theorem P) => theorem Q
    ax1 = theorem (imp p q)
    ax2 = theorem p
    result = mp ax1 ax2
    """
    
    # ... similar to above ...
    
    # Check result is (theorem q)
    assert isinstance(normalized, Theorem)
    inner = normalized.inner
    assert isinstance(inner, Atom)
    assert inner.name == 'q'
    print("✓ Modus Ponens test passed")

# Run tests
if __name__ == '__main__':
    test_boolean_not()
    test_modus_ponens()
    print("\nAll tests passed!")
    
    # Start REPL
    repl()
```

---

## Phase 9: Standard Library

Create `stdlib.logi`:

```logi
// Boolean logic
true = true
false = false
not = true => false | false => true
and = true => (X => X) | false => (_ => false)
or = true => (_ => true) | false => (X => X)

// Propositional constructors
imp = P => Q => (imp P Q)
neg = P => (neg P)
iff = P => Q => (iff P Q)

// Inference rules
mp = (theorem (imp P Q)) => (theorem P) => theorem Q
mt = (theorem (imp P Q)) => (theorem (neg Q)) => theorem (neg P)
hs = (theorem (imp P Q)) => (theorem (imp Q R)) => theorem (imp P R)
conj_intro = (theorem P) => (theorem Q) => theorem (and P Q)
conj_elim_l = (theorem (and P Q)) => theorem P
conj_elim_r = (theorem (and P Q)) => theorem Q

// Lists
nil = nil
cons = X => Xs => (cons X Xs)
head = (cons X Xs) => X
tail = (cons X Xs) => Xs
length = nil => zero | (cons _ Xs) => (succ (length Xs))

// Natural numbers
zero = zero
succ = N => (succ N)
one = (succ zero)
two = (succ one)
three = (succ two)
add = zero => (N => N) | (succ M) => (N => (succ (add M N)))
mul = zero => (_ => zero) | (succ M) => (N => (add N (mul M N)))
```

---

## Summary Checklist

### Core Implementation

- [ ] Define AST data structures
- [ ] Implement lexer (tokenization)
- [ ] Implement parser (with correct precedence)
- [ ] Implement pattern matcher
- [ ] Implement substitution
- [ ] Implement evaluator (small-step reduction)
- [ ] Build REPL
- [ ] Write tests

### Standard Library

- [ ] Boolean logic
- [ ] Propositional inference rules
- [ ] Lists
- [ ] Natural numbers

### Optional Enhancements

- [ ] Static axiom detection
- [ ] Better error messages
- [ ] Syntax highlighting
- [ ] Step-by-step debugger
- [ ] Proof tree visualizer

---

## Common Pitfalls

### 1. Parsing Ambiguity

**Problem:** Is `X Y => Z` a function or application?

**Solution:** One pattern per arrow (require currying).

### 2. Pattern Match Failure

**Problem:** What to do when no pattern matches?

**Solution:** Return None (no reduction). This is partial function behavior, not an error!

### 3. Variable Scoping

**Problem:** Free variables in function bodies.

**Solution:** Proper capture-avoiding substitution. Remove bound pattern variables from bindings.

### 4. Infinite Recursion

**Problem:** Terms like `loop = loop` reduce forever.

**Solution:** Add step limit to normalization (already included above).

---

## Next Steps

1. **Start coding!** Use the structure above as a template
2. **Test incrementally** - Test each phase before moving to next
3. **Use examples from EXAMPLES.md** for validation
4. **Refer to FORMAL_SPECIFICATION.md** for edge cases

**Good luck building Logi!** 🚀

---

End of Implementation Guide.
