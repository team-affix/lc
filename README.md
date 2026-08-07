# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution and no `unique_ptr` AST — terms are an algebraic data type interned in an `expr_pool`.

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`) |
| `expr_pool` | Hash-consed ownership of terms (`const expr*`) |
| `val` | WHNF values: `clo` / `fvar` / `napp` |
| `env` | Linked DAG binders: `delayed` \| `ready` (memoize-on-force) |
| `env_pool` / `val_pool` | Bump storage for ephemeral env/val nodes (fresh per normalize) |
| `evaluator` | Krivine-style eval to WHNF |
| `reifier` | Value → β-normal `expr` |
| `normalizer` | `reify(eval(t, nil), 0)` |

Variables use **de Bruijn indices** (`var(0)` = innermost binder).

## Layout

```
core/
  hpp/value_objects/     expr, val, env, expr_hash
  hpp/infrastructure/    pools, evaluator, reifier, normalizer
  cpp/                   non-template implementations
  test/unit|integration/ Google Test
```

## Example

```cpp
expr_pool pool;
env_pool envs;
val_pool vals;
evaluator<val_pool, val_pool, env_pool> ev(vals, vals, envs);
reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool, decltype(ev)>
    re(pool, pool, pool, vals, envs, ev);
normalizer<decltype(ev), decltype(re)> norm(ev, re);

const expr* id = pool.make_abs(pool.make_var(0));
const expr* nf = norm.normalize(pool.make_app(id, id));
// nf == id (same interned pointer)
```

Prefer constructing **fresh** `env_pool` / `val_pool` for each `normalize` (no `reset`).

## Build & test

```bash
make core                 # release library
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20 and vendored GoogleTest under `googletest/`.

## License

See [LICENSE](LICENSE).
