# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution — terms are an algebraic data type owned by freelist pools and threaded with `std::shared_ptr`.

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

**Resumable interpreter:** construct a `runtime` with the term to normalize. Call `step()` while `!done()`, then `output()` for the NF. There is no β-budget — stop calling `step()` to cap compute. Terms are not interned — compare with deep structural equality.

**Pools + RC:** `manifest` owns three `rc_pool`s (`expr` / `val` / `env`), a `garbage_collector`, and an `output_detacher`. Factories inject alloc capabilities (`IAllocExpr` / `IAllocVal` / `IAllocEnv` → `rc_pool::alloc`). Factory `make_*` returns a `std::shared_ptr` whose deleter enqueues the slot on a pending list. `rc_pool::collect_one()` retires one pending slot (`optional::reset` + freelist). The GC fixpoint-drains the three pools via `collect()`. `runtime` calls `manifest.gc.collect()` every `gc_interval` steps (ctor argument) and again in its destructor after dropping the interpreter and clearing its internal output register. Edges inside `expr` / `val` / `env` are `shared_ptr`s (DAG by construction).

**Inputs / outputs:** caller-owned `std::shared_ptr<expr>` input (any allocator, including `make_shared`) is used directly — no import into the runtime pools. Mid-run nodes still come from `rc_pool`. `runtime` owns an internal NF register; `output()` uses `manifest.detacher` to deep-copy that register onto standalone `make_shared` nodes and returns them. Call `output()` only after `done()`.

**Construction monopoly (mid-run):** only `rc_pool::alloc` (via the factories) may place ephemeral `expr` / `val` / `env` into pool storage. `output_detacher` is the deliberate `make_shared` boundary for user-facing results.

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`); child edges are `shared_ptr<expr>` |
| `expr_factory` / `val_factory` / `env_factory` | Typed `make_*` over `IAllocExpr` / `IAllocVal` / `IAllocEnv` (`rc_pool::alloc`) |
| `rc_pool` | Owned by `manifest`; `optional` deque + pending + freelist; `collect_one` |
| `garbage_collector` | Fixpoint drain over the three `rc_pool`s directly |
| `funcall` | Entry request (args + refs to parent return registers) |
| `continuation` | Stack item `{frame, stage}`; stage is a variant of distinct stage VOs |
| `reducer` / `reifier` | Per-stage `process(frame&, stage)` → `optional<pair<next_stage, child_funcall>>` (`nullopt` = pop) |
| `processor` | `init_continuation(funcall)` + forwards `process` to reducer/reifier |
| `interpreter` | Nested `visit` on continuation/stage; `step` / `done` |
| `initial_frame_generator` | `make_clo(term, nil)` then returns root `reify_val_funcall` (`IGenerateInitialFrame`) |
| `output_detacher` | Deep-copy NF onto standalone `make_shared` exprs (owned by `manifest`) |
| `manifest` | VO composition root: owns `rc_pool`s + factories + GC + detacher + NbE stack; ctor seeds the interpreter |
| `runtime` | Public façade over `manifest`; `step` / `done` / `output()`; collect on teardown |

Variables use **de Bruijn indices** (`var(0)` = innermost binder). Function WHNF is a `clo` whose `term` is an `abs`. Fresh binders in reification use de Bruijn levels (`fvar`).

**LC env vs C++ program frames:** `env_factory::make_env` builds lambda-calculus environment cells. Program frames are separate CEK-style control records inside continuations.

## Layout

```
core/
  hpp/value_objects/     expr, val, env, stages, frames, funcalls, continuations, manifest
  hpp/infrastructure/    rc_pool, garbage_collector, factories, output_detacher, reducer, reifier, processor, interpreter, initial_frame_generator, runtime
  cpp/                   non-template implementations
  test/unit|integration/ Google Test
```

## Example

```cpp
// Caller owns the input shared_ptr (any allocator). `output()` is standalone.
auto id = std::make_shared<expr>(expr{expr::abs{std::make_shared<expr>(expr{expr::var{0}})}});
runtime rt(id, 1024);
while (!rt.done())
    rt.step();
std::shared_ptr<expr> out = rt.output(); // standalone via output_detacher
```

## Build & test

```bash
make core                 # release library
make core_debug_fast      # preferred local test binary
make core_profile         # -O3 -g -pg test binary for gprof
./build/core_debug_fast
```

Profile (gprof), e.g. on a heavy case:

```bash
make core_profile
./build/core_profile --gtest_filter='RuntimePlaygroundTest.FactIterSeven'
gprof ./build/core_profile gmon.out | less
```

Requires C++20 and the GoogleTest git submodule (`git submodule update --init`).

## License

See [LICENSE](LICENSE).
