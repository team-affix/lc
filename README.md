# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution and no `unique_ptr` AST — terms are an algebraic data type stored in an `expr_pool` (`deque` bump allocation, no hash-consing).

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

**Resumable interpreter:** construct a `runtime` with an output register and the term to normalize. Drive `step()` until it returns `false`, then check `done()`. The NF is written into `out` (allocated in the runtime’s owned `expr_pool`). There is no β-budget — stop calling `step()` to cap compute. Input terms need not live in any pool. Terms are not interned — compare with deep structural equality.

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`) |
| `expr_pool` | Bump ownership of terms (`deque`, `const expr*`) |
| `val` | WHNF values: Krivine `clo(term, environment)` / `fvar` / `napp` |
| `env` | Linked DAG cells: `bound_value` (`const val*`) + `parent` (`make_env`) |
| `env_pool` / `val_pool` | Bump storage for ephemeral env/val nodes |
| `funcall` | Entry request (args + refs to parent return registers) |
| `continuation` | Stack item `{frame, stage}`; stage is a variant of distinct stage VOs |
| `reducer` / `reifier` | Per-stage `process(frame&, stage)` → `optional<pair<next_stage, child_funcall>>` (`nullopt` = pop) |
| `processor` | `init_continuation(funcall)` + forwards `process` to reducer/reifier |
| `interpreter` | Nested `visit` on continuation/stage; `step` / `done` |
| `normalizer` | `make_clo(term, nil)` then returns root `reify_val_funcall` |
| `manifest` | VO composition root: owns pools + NbE stack; ctor seeds the interpreter |
| `runtime` | Façade: owns `manifest`; `step` / `done` forward to `manifest.interp` |

Variables use **de Bruijn indices** (`var(0)` = innermost binder). Function WHNF is a `clo` whose `term` is an `abs`. Fresh binders in reification use de Bruijn levels (`fvar`).

**LC env vs C++ program frames:** `env_pool::make_env` builds lambda-calculus environment cells. Program frames are separate CEK-style control records inside continuations.

## Layout

```
core/
  hpp/value_objects/     expr, val, env, stages, frames, funcalls, continuations, manifest
  hpp/infrastructure/    pools, reducer, reifier, processor, interpreter, normalizer, runtime
  cpp/                   non-template implementations
  test/unit|integration/ Google Test
```

## Example

```cpp
expr_pool pool; // only for building the input term, if desired
const expr* id = pool.make_abs(pool.make_var(0));
const expr* nf;
runtime rt(nf, pool.make_app(id, id));
while (rt.step()) {
}
if (rt.done()) {
    // nf equals id by deep structural comparison of the trees
}
```

Keep the `runtime` alive as long as you use `nf` (NF nodes live in the manifest’s `expr_pool`).

## Build & test

```bash
make core                 # release library
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20 and the GoogleTest git submodule (`git submodule update --init`).

## License

See [LICENSE](LICENSE).
