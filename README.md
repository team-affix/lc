# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution and no `unique_ptr` AST — terms are an algebraic data type stored in an `expr_pool` (`deque` bump allocation, no hash-consing).

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

**Resumable interpreter:** `normalizer::normalize` returns a root `funcall`; construct an `interpreter` with that call and a `processor`. The caller drives `step()` until it returns `false`, then checks `done()` for successful completion. The NF is written into the caller’s `out` register (the interpreter does not return the solution). There is no β-budget — stop calling `step()` to cap compute. Terms are not interned — compare with deep structural equality (pointer identity and shallow `operator==` on `abs`/`app` are not meaningful across separately built trees).

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`) |
| `expr_pool` | Bump ownership of terms (`deque`, `const expr*`) |
| `val` | WHNF values: Krivine `clo(term, environment)` / `fvar` / `napp` |
| `env` | Linked DAG cells: `bound_value` (`const val*`) + `parent` (`make_env`) |
| `env_pool` / `val_pool` | Bump storage for ephemeral env/val nodes (fresh per normalize) |
| `funcall` | Entry request (args + refs to parent return registers) |
| `continuation` | Stack item `{frame, stage}`; stage is a variant of distinct stage VOs |
| `reducer` / `reifier` | Per-stage `process(frame&, stage)` → next stage + optional child `funcall` |
| `processor` | `init_continuation(funcall)` + forwards `process` to reducer/reifier |
| `interpreter` | Nested `visit` on continuation/stage; `step` / `done` |
| `normalizer` | `make_clo(term, nil)` then returns root `reify_val_funcall` |

Variables use **de Bruijn indices** (`var(0)` = innermost binder). Function WHNF is a `clo` whose `term` is an `abs`. Fresh binders in reification use de Bruijn levels (`fvar`).

**LC env vs C++ program frames:** `env_pool::make_env` builds lambda-calculus environment cells. Program frames are separate CEK-style control records inside continuations.

## Layout

```
core/
  hpp/value_objects/     expr, val, env, stages, frames, funcalls, continuations
  hpp/infrastructure/    pools, reducer, reifier, processor, interpreter, normalizer
  cpp/                   non-template implementations
  test/unit|integration/ Google Test
```

## Example

```cpp
expr_pool pool;
env_pool envs;
val_pool vals;
env_lookup lookup;
reducer<val_pool, val_pool, env_pool, env_lookup> red(vals, vals, envs, lookup);
reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool, val_pool> re(
    pool, pool, pool, vals, envs, vals);
processor<decltype(red), decltype(re)> proc(red, re);
normalizer<val_pool> norm(vals);

const expr* id = pool.make_abs(pool.make_var(0));
const expr* nf;
interpreter<continuation, decltype(proc)> interp(
    proc, norm.normalize(nf, pool.make_app(id, id)));
while (interp.step()) {
}
if (interp.done()) {
    // nf equals id by deep structural comparison of the trees
}
```

Prefer constructing **fresh** `env_pool` / `val_pool` for each normalize job (no `reset`).

## Build & test

```bash
make core                 # release library
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20 and the GoogleTest git submodule (`git submodule update --init`).

## License

See [LICENSE](LICENSE).
