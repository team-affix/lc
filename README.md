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
#include "infrastructure/rc_pool.hpp"
#include "value_objects/expr.hpp"

auto id = std::make_shared<expr>(
    expr::abs{std::make_shared<expr>(expr::var{0})});

rc_pool<expr> out_nodes;  // owns the normalized result; must outlive nf
runtime rt(id, out_nodes);
while (!rt.done())
    rt.step();

std::shared_ptr<expr> nf = rt.output();  // nodes live in out_nodes
```

`step()` / `done()` make the interpreter resumable. `output()` is only valid
after `done()`. Keep `out_nodes` (and any returned `shared_ptr`s) alive as long
as you need the normal form; evaluation uses separate internal pools.

How to cap time and memory:

```cpp
rc_pool<expr> out_nodes;
runtime rt(term, out_nodes);
for (uint64_t steps = 0;
     !rt.done() && steps < MAX_STEPS && rt.space_usage() < MAX_BYTES;
     ++steps)
    rt.step();
```

`space_usage()` is the approximate number of bytes the lambda calculus program
currently takes up (internal eval pools plus the external out pool). In the
worst case, peak space is on the order of the number of steps run so far — it
grows linearly with the step count.

## Memory reclaim

Ephemeral nodes allocated during evaluation live in freelist pools. When the
last `shared_ptr` to a pooled node drops, that pool eagerly drains pending
slots (`try_collect_loop`). Destroying an incomplete `runtime` drops eval roots
and is well-defined. Drop output `shared_ptr`s and destroy `out_nodes` to reclaim
the normal form.

## Build & test

```bash
git submodule update --init
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20. Also: `make core` (release library), `make core_profile` (gprof).

## License

See [LICENSE](LICENSE).