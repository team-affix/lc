# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution — terms are an algebraic data type owned by freelist pools and threaded with `std::shared_ptr`.

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

**Resumable interpreter:** construct a `runtime` with an output register and the term to normalize. Call `step()` while `!done()`. The NF is written into `out` as a `std::shared_ptr<expr>` into the runtime’s owned `expr_pool`. There is no β-budget — stop calling `step()` to cap compute. Terms are not interned — compare with deep structural equality.

**Pools + RC:** `expr_pool` / `val_pool` / `env_pool` store nodes in a `deque` (stable addresses) and reuse slots via a freelist. `make_*` returns `std::shared_ptr` with a custom deleter that returns the slot to that pool. Mid-normalize junk is reclaimed when `shared_ptr`s drop. Edges inside `expr` / `val` / `env` are `shared_ptr`s (DAG by construction).

**Construction monopoly (infrastructure):** only the three pools may place `expr` / `val` / `env` into storage or attach freelist deleters. Inside `core/hpp` and `core/cpp`, do not `make_shared` those types or wrap raw pool pointers yourself — copy/move the `shared_ptr`s pools return.

**User / test inputs** may be hand-built (stack locals, `make_shared`, a caller-owned pool, etc.). `runtime` **`import`s** the input into its manifest `expr_pool` at construction so internal closures only see pool-managed nodes.

**Long-lived NFs:** `out` aliases runtime pool storage. Reset `out` (or `user_pool.import(out)`) before destroying the `runtime`, otherwise pool teardown races with live deleters.

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`); child edges are `shared_ptr<expr>` |
| `expr_pool` | Freelist pool: `make_var` / `make_abs` / `make_app` / `import` → `shared_ptr<expr>` |
| `val` | WHNF values: Krivine `clo(term, environment)` / `fvar` / `napp` (shared edges) |
| `env` | Linked cells: `bound_value` (`shared_ptr<val>`) + `parent` (`shared_ptr<env>`) |
| `env_pool` / `val_pool` | Freelist pools for ephemeral env/val nodes |
| `rc_pool` | Shared deque + freelist + iterative release worklist |
| `funcall` | Entry request (args + refs to parent return registers) |
| `continuation` | Stack item `{frame, stage}`; stage is a variant of distinct stage VOs |
| `reducer` / `reifier` | Per-stage `process(frame&, stage)` → `optional<pair<next_stage, child_funcall>>` (`nullopt` = pop) |
| `processor` | `init_continuation(funcall)` + forwards `process` to reducer/reifier |
| `interpreter` | Nested `visit` on continuation/stage; `step` / `done` |
| `normalizer` | `make_clo(term, nil)` then returns root `reify_val_funcall` |
| `manifest` | VO composition root: owns pools + NbE stack; ctor imports input + seeds the interpreter |
| `runtime` | Façade: owns `manifest`; `step` / `done` forward to `manifest.interp` |

Variables use **de Bruijn indices** (`var(0)` = innermost binder). Function WHNF is a `clo` whose `term` is an `abs`. Fresh binders in reification use de Bruijn levels (`fvar`).

**LC env vs C++ program frames:** `env_pool::make_env` builds lambda-calculus environment cells. Program frames are separate CEK-style control records inside continuations.

## Layout

```
core/
  hpp/value_objects/     expr, val, env, stages, frames, funcalls, continuations, manifest
  hpp/infrastructure/    rc_pool, pools, reducer, reifier, processor, interpreter, normalizer, runtime
  cpp/                   non-template implementations
  test/unit|integration/ Google Test
```

## Example

```cpp
// Hand-built or pool-built input is fine; runtime imports it.
auto id = std::make_shared<expr>(expr{expr::abs{std::make_shared<expr>(expr{expr::var{0}})}});
std::shared_ptr<expr> out;
{
    runtime rt(out, id.get());
    while (!rt.done())
        rt.step();
    // Keep a copy that outlives the runtime:
    expr_pool user;
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
