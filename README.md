# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution — terms are an algebraic data type owned by freelist pools and threaded with `std::shared_ptr`.

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

**Resumable interpreter:** construct a `runtime` with an output register and the term to normalize. Call `step()` while `!done()`. The NF is written into `out` as a `std::shared_ptr<expr>` into the runtime’s `rc_pool<expr>`. There is no β-budget — stop calling `step()` to cap compute. Terms are not interned — compare with deep structural equality.

**Pools + RC:** `manifest` owns three `rc_pool`s (`expr` / `val` / `env`). Façades inject alloc capabilities (`IAllocExpr` / `IAllocVal` / `IAllocEnv` → `rc_pool::alloc`). Facade `make_*` returns a `std::shared_ptr` whose deleter enqueues the slot on a pending list. `rc_pool::collect_one()` retires one pending slot (`optional::reset` + freelist). `garbage_collector` is injected with the three `rc_pool`s and fixpoint-drains them via `collect()`. `runtime` (injected `ICollectGarbage`) calls `collect()` every 1024 steps and again in its destructor after dropping the interpreter. Edges inside `expr` / `val` / `env` are `shared_ptr`s (DAG by construction). Each `rc_pool` must outlive every external `shared_ptr` it created.

**Construction monopoly (infrastructure):** only `rc_pool::alloc` (via the façades) may place `expr` / `val` / `env` into storage or attach freelist deleters. Inside `core/hpp` and `core/cpp`, do not `make_shared` those types or wrap raw pool pointers yourself — copy/move the `shared_ptr`s façades return.

**User / test inputs** may be hand-built (stack locals, `make_shared`, a caller-owned `rc_pool`, etc.). `runtime` **`import`s** the input into its manifest `expr_pool` at construction so internal closures only see pool-managed nodes.

**Long-lived NFs:** `out` aliases runtime `rc_pool` storage. Reset `out` (or import into a caller-owned pool) before destroying the `runtime`, otherwise pool teardown races with live deleters.

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`); child edges are `shared_ptr<expr>` |
| `expr_pool` / `val_pool` / `env_pool` | Façades over `IAllocExpr` / `IAllocVal` / `IAllocEnv` (`rc_pool::alloc`) |
| `rc_pool` | Owned by `manifest`; `optional` deque + pending + freelist; `collect_one` |
| `garbage_collector` | Fixpoint drain over the three `rc_pool`s directly |
| `funcall` | Entry request (args + refs to parent return registers) |
| `continuation` | Stack item `{frame, stage}`; stage is a variant of distinct stage VOs |
| `reducer` / `reifier` | Per-stage `process(frame&, stage)` → `optional<pair<next_stage, child_funcall>>` (`nullopt` = pop) |
| `processor` | `init_continuation(funcall)` + forwards `process` to reducer/reifier |
| `interpreter` | Nested `visit` on continuation/stage; `step` / `done` |
| `normalizer` | `make_clo(term, nil)` then returns root `reify_val_funcall` |
| `manifest` | VO composition root: owns `rc_pool`s + façades + GC + NbE stack; ctor imports input + seeds the interpreter |
| `runtime` | Façade templated on `ICollectGarbage`; `step` / `done`; `collect()` every 1024 steps |

Variables use **de Bruijn indices** (`var(0)` = innermost binder). Function WHNF is a `clo` whose `term` is an `abs`. Fresh binders in reification use de Bruijn levels (`fvar`).

**LC env vs C++ program frames:** `env_pool::make_env` builds lambda-calculus environment cells. Program frames are separate CEK-style control records inside continuations.

## Layout

```
core/
  hpp/value_objects/     expr, val, env, stages, frames, funcalls, continuations, manifest
  hpp/infrastructure/    rc_pool, garbage_collector, pools, reducer, reifier, processor, interpreter, normalizer, runtime
  cpp/                   non-template implementations
  test/unit|integration/ Google Test
```

## Example

```cpp
// Hand-built or pool-built input is fine; runtime imports it.
auto id = std::make_shared<expr>(expr{expr::abs{std::make_shared<expr>(expr{expr::var{0}})}});
std::shared_ptr<expr> out;
{
    nbe_runtime rt(out, id.get());
    while (!rt.done())
        rt.step();
    // Keep a copy that outlives the runtime:
    rc_pool<expr> user_nodes;
    expr_pool<rc_pool<expr>> user{user_nodes};
    auto kept = user.import(out);
    out.reset();
} // runtime (and its pools) destroyed; `kept` remains valid
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
