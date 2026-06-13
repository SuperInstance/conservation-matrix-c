# conservation-matrix-c — Conservation Laws for Ternary Agent Populations

**conservation-matrix-c** is a C library that enforces and verifies **conservation invariants** in ternary agent systems — populations where each agent takes one of three actions (Avoid −1, Unknown 0, Choose +1). The core invariant is **γ + η = C**: the sum of the population's mean action value γ and the Shannon entropy η of the action distribution should remain approximately constant across generations. The library provides conservation trackers, fitness convergence monitors, Shannon entropy computation, population advantage metrics, and a 5-species Lotka-Volterra ecology simulator.

## Why It Matters

Without conservation enforcement, ternary populations exhibit pathological dynamics: either collapsing to homogeneous all-avoid (γ → −1, η → 0) or oscillating chaotically. The conservation invariant γ + η = C acts as a **population-level homeostat** — when entropy drops (agents converge), the mean value must compensate, and vice versa. This mirrors Noether's theorem in physics: conserved quantities correspond to symmetries of the dynamics. In practice, tracking this invariant lets fleet operators detect when an agent ecosystem is healthy (C is stable) versus degenerating (C drifts). The library's 5-species Lotka-Volterra solver enables ecological modeling of strategy diversity, ensuring that no strategy species goes extinct — the prerequisite for long-term population resilience.

## How It Works

### Conservation Tracking

For a population of $n$ agents with ternary actions $a_i \in \{-1, 0, +1\}$, we compute three ratios per generation:

$$\gamma_{\text{avoid}} = \frac{|\{i : a_i = -1\}|}{n}, \quad \gamma_{\text{unknown}} = \frac{|\{i : a_i = 0\}|}{n}, \quad \gamma_{\text{choose}} = \frac{|\{i : a_i = +1\}|}{n}$$

The tracker stores these ratios for up to 10,000 generations. Conservation is verified by checking that the standard deviation of the avoid ratio $\sigma(\gamma_{\text{avoid}})$ is below a threshold:

$$\text{conserved} \iff \sigma(\gamma_{\text{avoid}}) < \epsilon$$

**Complexity**: Recording is $O(n)$ per generation (counting actions). Mean/std computation is $O(G)$ where $G$ is the number of recorded generations. Space: $O(G)$ for storing ratio history.

### Shannon Entropy

The diversity of the action distribution is measured by Shannon entropy:

$$H = -\sum_{i} p_i \log_2 p_i$$

where $p_i$ is the proportion of agents taking action $i$. For a uniform ternary distribution ($\frac{1}{3}, \frac{1}{3}, \frac{1}{3}$), $H = \log_2 3 \approx 1.585$ bits (maximum). For a homogeneous population, $H = 0$.

The conservation invariant states:

$$C \approx \gamma + H = \frac{1}{n}\sum_i a_i + H(p)$$

When $C$ is stable across generations, the system is in a healthy equilibrium between exploitation (high γ) and exploration (high H).

### Fitness Convergence

A simple target-tracking monitor. Records fitness values over generations and checks convergence:

$$\text{converged} \iff |f_{\text{current}} - f_{\text{target}}| < \text{tolerance}$$

Also computes total improvement $f_{\text{current}} - f_0$ to measure evolutionary progress.

### Population Advantage

Compares the **average fitness of a population** against the **best individual**:

$$\text{advantage} = \bar{f}_{\text{pop}} - f_{\text{best}}$$

When advantage > 0, the population as a whole outperforms any single individual — a strong signal that diversity (high η) is beneficial. This provides empirical justification for maintaining the conservation invariant.

### 5-Species Lotka-Volterra Ecology

The library models five strategy species (Explorer, Diplomat, Marksman, Climber, Prospector) using generalized **Lotka-Volterra competition equations**:

$$\frac{dN_i}{dt} = r_i N_i \left(1 - \frac{N_i}{K_i} - \sum_{j \neq i} \frac{\alpha_{ij} N_j}{K_i}\right)$$

where $r_i$ is the intrinsic growth rate, $K_i$ is the carrying capacity, and $\alpha_{ij}$ is the competition coefficient (how much species $j$ inhibits species $i$). Euler integration with timestep $dt$ advances the system. **Resilience** is measured as the fraction of species with population ≥ 1.0. When all five survive, the ecosystem is considered resilient.

### Conservation Verification Across Scales

The library tests conservation at population sizes from 10 to 5,000 agents. The invariant should hold regardless of scale — if it breaks at large $n$, the system has a fundamental instability.

## Quick Start

```c
#include "conservation_matrix.h"

/* 1. Track conservation across generations */
conservation_tracker_t tracker;
cm_tracker_init(&tracker, 100);  // population size: 100

ternary_action_t actions[100];
for (int gen = 0; gen < 50; gen++) {
    /* ... agents decide actions ... */
    cm_tracker_record(&tracker, actions, 100);
}

printf("Avoid ratio: %.3f ± %.3f\n",
       cm_tracker_avoid_mean(&tracker),
       cm_tracker_avoid_std(&tracker));

int conserved = cm_tracker_verify_conservation(&tracker, 0.02);
printf("Conservation: %s\n", conserved ? "STABLE" : "VIOLATED");

/* 2. Compute Shannon entropy */
double props[3] = { 0.5, 0.3, 0.2 };  // avoid, unknown, choose
double H = cm_shannon_entropy(props, 3);
printf("Entropy: %.4f bits\n", H);  // ~1.485

/* 3. Monitor fitness convergence */
fitness_convergence_t fit;
cm_fitness_init(&fit, 0.988);  // target fitness
cm_fitness_record(&fit, 0.803);
cm_fitness_record(&fit, 0.95);
cm_fitness_record(&fit, 0.985);
printf("Converged: %s\n", cm_fitness_converged(&fit, 0.01) ? "YES" : "NO");

/* 4. Simulate 5-species ecology */
ecology_t eco;
cm_ecology_init(&eco, 10.0);  // 10 individuals per species

double growth_rates[5] = {0.1, 0.08, 0.12, 0.09, 0.11};
double carrying_caps[5] = {100, 80, 120, 90, 110};
double alpha[5][5] = { /* competition matrix */ };
for (int t = 0; t < 2000; t++) {
    cm_ecology_step(&eco, growth_rates, carrying_caps, alpha, 0.1);
}
printf("Resilience: %.2f\n", cm_ecology_resilience(&eco));
```

```bash
# Build
gcc -O2 -c src/conservation_matrix.c -o conservation_matrix.o
gcc -O2 tests/test_conservation.c src/conservation_matrix.c -o test_conservation -lm && ./test_conservation
```

## API

### Types

```c
typedef enum {
    TERNARY_AVOID  = -1,
    TERNARY_UNKNOWN = 0,
    TERNARY_CHOOSE = 1
} ternary_action_t;

typedef enum {
    SPECIES_EXPLORER, SPECIES_DIPLOMAT, SPECIES_MARKSMAN,
    SPECIES_CLIMBER, SPECIES_PROSPECTOR, SPECIES_COUNT
} strategy_species_t;
```

### Conservation Tracker

```c
void   cm_tracker_init(conservation_tracker_t *t, size_t pop_size);
void   cm_tracker_record(conservation_tracker_t *t, const ternary_action_t *actions, size_t n);
double cm_tracker_avoid_mean(const conservation_tracker_t *t);
double cm_tracker_choose_mean(const conservation_tracker_t *t);
double cm_tracker_unknown_mean(const conservation_tracker_t *t);
double cm_tracker_avoid_std(const conservation_tracker_t *t);
int    cm_tracker_verify_conservation(const conservation_tracker_t *t, double threshold);
```

### Fitness & Entropy

```c
void   cm_fitness_init(fitness_convergence_t *f, double target);
void   cm_fitness_record(fitness_convergence_t *f, double value);
int    cm_fitness_converged(const fitness_convergence_t *f, double tolerance);
double cm_shannon_entropy(const double *proportions, size_t n);
```

### Population & Ecology

```c
double cm_population_advantage(const double *pop_fitness, size_t n, double best_individual);
int    cm_population_wins(const double *pop_fitness, size_t n, double best_individual);
double cm_avoid_choose_ratio(size_t avoid_count, size_t choose_count);
void   cm_ecology_init(ecology_t *e, double initial_pop);
void   cm_ecology_step(ecology_t *e, const double *r, const double *k,
                       const double alpha[5][5], double dt);
double cm_ecology_resilience(const ecology_t *e);
int    cm_ecology_all_survive(const ecology_t *e, double threshold);
```

## Architecture Notes

conservation-matrix-c is the **analytical foundation** of the SuperInstance conservation framework. It provides the C-level primitives for computing γ, η, and their sum C = γ + η across agent populations of any size. The 5-species ecology model maps directly to the fleet's strategy taxonomy (Explorer = foraging/scout agents, Diplomat = coordination agents, Marksman = high-precision inference, Climber = optimization/ranking, Prospector = exploration/discovery). The Lotka-Volterra dynamics ensure that strategy diversity is maintained as a dynamical equilibrium rather than enforced by rules.

See: [SuperInstance Architecture](https://github.com/SuperInstance/SuperInstance/blob/main/ARCHITECTURE.md)

## References

1. Shannon, C. E. (1948). "A Mathematical Theory of Communication." *Bell System Technical Journal* 27 — The entropy measure used for diversity quantification.
2. Lotka, A. J. (1925). *Elements of Physical Biology.* — Original formulation of the predator-prey/competition equations generalized to $n$ species.
3. MacArthur, R. H. (1955). "Fluctuations of Animal Populations and a Measure of Community Stability." *Ecology* 36(3) — Connection between species diversity and ecosystem resilience.

## License

MIT
