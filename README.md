# lc

An efficient C++ interpreter for **pure closed untyped lambda calculus**.
It normalizes terms to β-normal form.

## Approach

Evaluation uses **normalization by evaluation** (NbE):

1. A **Krivine-style** machine evaluates to weak head normal form under
  call-by-need (shared thunks / environments, so work is not repeated).
2. **Reification** quotes the result back to a syntactic normal form.

Reduction does not rewrite terms by substitution; bindings live in
environments and are looked up (so variable capture is not an issue).
Terms use **de Bruijn indices**. Only closed terms are supported — every
variable must be bound.

## Usage

Build a term as a `std::shared_ptr<expr>` tree (`var` / `abs` / `app`), then drive a `runtime` to normalize it:

```cpp
#include "infrastructure/runtime.hpp"
#include "value_objects/expr.hpp"

auto id = std::make_shared<expr>(
    expr{expr::abs{std::make_shared<expr>(expr{expr::var{0}})}});

runtime rt(id);           // optional 2nd arg: garbage collection interval (default 1024 steps)
while (!rt.done())
    rt.step();            // stop early anytime to cap work

std::shared_ptr<expr> nf = rt.output();  // standalone copy of the normal form
```

`step()` / `done()` make the interpreter resumable. `output()` is only valid
after `done()` and returns a self-contained tree you can keep after `runtime`
is destroyed.

## Garbage collection

Ephemeral nodes allocated during evaluation live in freelist pools. The runtime
runs garbage collection every N steps (`gc_interval`, default 1024) and once
more on destruction.

## Build & test

```bash
git submodule update --init
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20. Also: `make core` (release library), `make core_profile` (gprof).

## License

See [LICENSE](LICENSE).