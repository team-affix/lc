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

Include headers from `core/hpp`. Build a closed term as a `std::shared_ptr<expr>`
tree (`var` / `abs` / `app`), own an output pool for the normal form, then drive
a `runtime`:

```cpp
#include "infrastructure/runtime.hpp"
#include "infrastructure/rc_pool.hpp"
#include "value_objects/expr.hpp"

// Input term (any shared_ptr tree is fine — make_shared or your own pool).
auto id = std::make_shared<expr>(
    expr{expr::abs{std::make_shared<expr>(expr{expr::var{0}})}});

// Output pool: only the normalized result is allocated here.
// It must outlive every shared_ptr returned by output().
rc_pool<expr> out_nodes;

runtime rt(id, out_nodes);
while (!rt.done())
    rt.step();

std::shared_ptr<expr> nf = rt.output();
// Use nf… then drop nf (and/or destroy out_nodes) when finished.
```

### API

| Call | Role |
|------|------|
| `runtime(term, out_nodes)` | Start normalizing `term`; NF nodes go into `out_nodes`. |
| `step()` | Advance one machine step. |
| `done()` | `true` when normalization finished. |
| `output()` | Root of the normal form (valid only after `done()`). |
| `space_usage()` | Approx. bytes of **eval** state (internal pools + stack). |

`step()` / `done()` are resumable, so you can interleave other work or enforce
limits:

```cpp
rc_pool<expr> out_nodes;
runtime rt(term, out_nodes);
for (uint64_t steps = 0;
     !rt.done() && steps < MAX_STEPS && rt.space_usage() < MAX_BYTES;
     ++steps)
    rt.step();

if (rt.done())
    auto nf = rt.output();
```

`space_usage()` does **not** include `out_nodes`. In the worst case, peak eval
space grows roughly linearly with the number of steps run so far.

### Lifetimes

- **`out_nodes`** must outlive the `runtime` and any `shared_ptr` from
  `output()`. Destroying `out_nodes` while those pointers are live is undefined.
- Destroying an incomplete `runtime` (before `done()`) is fine: eval state is
  dropped; `out_nodes` is untouched unless detach already wrote into it.
- Input `term` may be released once the `runtime` has been constructed (the
  runtime keeps what it needs).

### Memory

Evaluation allocates into internal freelist pools owned by the runtime. When the
last `shared_ptr` to a pooled node drops, that pool reclaims eagerly.

The normal form is a separate copy in your `out_nodes` pool so deep results can
be torn down without blowing the stack. Drop the output `shared_ptr`s, then
destroy `out_nodes`, to reclaim the NF.

## Build & test

```bash
git submodule update --init
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20. Also: `make core` (release library), `make core_profile` (gprof).

## License

See [LICENSE](LICENSE).
