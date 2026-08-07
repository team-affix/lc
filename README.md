# lc

Efficient C++ lambda calculus via **Normalization by Evaluation** (Krivine machine + reification).

## Overview

`lc` reduces untyped lambda terms to β-normal form using call-by-need evaluation to weak head normal form, then reification. There is no term substitution and no `unique_ptr` AST — terms are an algebraic data type stored in an `expr_pool` (`deque` bump allocation, no hash-consing).

**Closed terms only:** every `var` must be bound by some enclosing `abs`. Unbound lookup is a debug assertion, not a runtime value.

**β-budget:** `normalize(out, term, reductions_left)` returns `false` when a β-step would run with no remaining budget (`out` untouched). Not resumable — retry with a larger budget. Terms are not interned — compare with deep structural equality (pointer identity and shallow `operator==` on `abs`/`app` are not meaningful across separately built trees).

## Design

| Piece | Role |
|-------|------|
| `expr` | Surface syntax: `var` / `abs` / `app` (`std::variant`) |
| `expr_pool` | Bump ownership of terms (`deque`, `const expr*`) |
| `val` | WHNF values: Krivine `clo(term, environment)` / `fvar` / `napp` |
| `env` | Linked DAG frames: `bound_value` (`const val*`) + `parent` |
| `env_pool` / `val_pool` | Bump storage for ephemeral env/val nodes (fresh per normalize) |
| `reducer` | in-place `whnf(const val*&)` to WHNF (`bool` + β-budget); memoize by overwrite |
| `reifier` | Value → β-normal `expr` (`bool` + out + depth + β-budget) |
| `normalizer` | `make_clo(term, nil)` then `reify(…, 0, …)` |

Variables use **de Bruijn indices** (`var(0)` = innermost binder). Function WHNF is a `clo` whose `term` is an `abs`. Fresh binders in reification use de Bruijn levels (`fvar`).

## Layout

```
core/
  hpp/value_objects/     expr, val, env
  hpp/infrastructure/    pools, reducer, reifier, normalizer
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
reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool, val_pool,
        decltype(red)>
    re(pool, pool, pool, vals, envs, vals, red);
normalizer<val_pool, decltype(re)> norm(vals, re);

const expr* id = pool.make_abs(pool.make_var(0));
const expr* nf;
uint64_t budget = 1000;
if (!norm.normalize(nf, pool.make_app(id, id), budget))
    /* β-budget exhausted */;
// nf equals id by deep structural comparison of the trees
```

Prefer constructing **fresh** `env_pool` / `val_pool` for each `normalize` (no `reset`).

## Build & test

```bash
make core                 # release library
make core_debug_fast      # preferred local test binary
./build/core_debug_fast
```

Requires C++20 and the GoogleTest git submodule (`git submodule update --init`).

## License

See [LICENSE](LICENSE).
