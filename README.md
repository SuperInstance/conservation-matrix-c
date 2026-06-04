# conservation-matrix-c

C implementation of conservation laws in ternary agent systems {-1, 0, +1}.

Cross-language companion to [conservation-matrix](https://github.com/SuperInstance/conservation-matrix-rs) (Rust).

## API

- `conservation_tracker_t` — ratio tracking, conservation verification
- `fitness_convergence_t` — fitness convergence toward target
- `cm_shannon_entropy()` — Shannon diversity index
- `cm_population_advantage()` — population vs individual fitness
- `ecology_t` — 5-species Lotka-Volterra dynamics

## Build & Test

```bash
gcc -o test_conservation tests/test_conservation.c src/conservation_matrix.c -lm -Wall -O2
./test_conservation
```

18 tests passing. No external dependencies.

## License

MIT
