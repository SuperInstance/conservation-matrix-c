/* conservation-matrix-c: Implementation */
#include "conservation_matrix.h"
#include <string.h>
#include <math.h>

/* --- Conservation Tracker --- */

void cm_tracker_init(conservation_tracker_t *t, size_t pop_size) {
    memset(t, 0, sizeof(*t));
    t->population_size = pop_size;
}

void cm_tracker_record(conservation_tracker_t *t, const ternary_action_t *actions, size_t n) {
    if (t->count >= CM_MAX_GENERATIONS) return;
    size_t avoid = 0, unknown = 0, choose = 0;
    for (size_t i = 0; i < n; i++) {
        switch (actions[i]) {
            case TERNARY_AVOID:  avoid++; break;
            case TERNARY_UNKNOWN: unknown++; break;
            case TERNARY_CHOOSE: choose++; break;
        }
    }
    double total = (double)n;
    t->avoid_ratios[t->count] = avoid / total;
    t->unknown_ratios[t->count] = unknown / total;
    t->choose_ratios[t->count] = choose / total;
    t->count++;
}

static double mean_of(const double *arr, size_t n) {
    if (n == 0) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) sum += arr[i];
    return sum / (double)n;
}

static double std_of(const double *arr, size_t n) {
    if (n == 0) return 0.0;
    double m = mean_of(arr, n);
    double var = 0.0;
    for (size_t i = 0; i < n; i++) var += (arr[i] - m) * (arr[i] - m);
    return sqrt(var / (double)n);
}

double cm_tracker_avoid_mean(const conservation_tracker_t *t) {
    return mean_of(t->avoid_ratios, t->count);
}

double cm_tracker_choose_mean(const conservation_tracker_t *t) {
    return mean_of(t->choose_ratios, t->count);
}

double cm_tracker_unknown_mean(const conservation_tracker_t *t) {
    return mean_of(t->unknown_ratios, t->count);
}

double cm_tracker_avoid_std(const conservation_tracker_t *t) {
    return std_of(t->avoid_ratios, t->count);
}

int cm_tracker_verify_conservation(const conservation_tracker_t *t, double threshold) {
    return cm_tracker_avoid_std(t) < threshold;
}

/* --- Fitness Convergence --- */

void cm_fitness_init(fitness_convergence_t *f, double target) {
    memset(f, 0, sizeof(*f));
    f->target = target;
}

void cm_fitness_record(fitness_convergence_t *f, double value) {
    if (f->count >= CM_MAX_GENERATIONS) return;
    f->history[f->count++] = value;
}

double cm_fitness_current(const fitness_convergence_t *f) {
    if (f->count == 0) return 0.0;
    return f->history[f->count - 1];
}

int cm_fitness_converged(const fitness_convergence_t *f, double tolerance) {
    if (f->count == 0) return 0;
    return fabs(f->history[f->count - 1] - f->target) < tolerance;
}

double cm_fitness_improvement(const fitness_convergence_t *f) {
    if (f->count < 2) return 0.0;
    return f->history[f->count - 1] - f->history[0];
}

/* --- Shannon Entropy --- */

double cm_shannon_entropy(const double *proportions, size_t n) {
    double h = 0.0;
    for (size_t i = 0; i < n; i++) {
        if (proportions[i] > 0.0) {
            h -= proportions[i] * log2(proportions[i]);
        }
    }
    return h;
}

/* --- Population Advantage --- */

double cm_population_advantage(const double *pop_fitness, size_t n, double best_individual) {
    if (n == 0) return 0.0;
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) sum += pop_fitness[i];
    return (sum / (double)n) - best_individual;
}

int cm_population_wins(const double *pop_fitness, size_t n, double best_individual) {
    return cm_population_advantage(pop_fitness, n, best_individual) > 0.0;
}

/* --- Avoid:Choose Ratio --- */

double cm_avoid_choose_ratio(size_t avoid_count, size_t choose_count) {
    if (choose_count == 0) return 0.0;
    return (double)avoid_count / (double)choose_count;
}

/* --- Ecology --- */

void cm_ecology_init(ecology_t *e, double initial_pop) {
    for (size_t i = 0; i < SPECIES_COUNT; i++) {
        e->populations[i] = initial_pop;
    }
    e->n_species = SPECIES_COUNT;
}

void cm_ecology_step(ecology_t *e, const double *r, const double *k,
                     const double alpha[5][5], double dt) {
    double new_pop[5];
    for (size_t i = 0; i < SPECIES_COUNT; i++) {
        double competition = 0.0;
        for (size_t j = 0; j < SPECIES_COUNT; j++) {
            if (j != i) competition += alpha[i][j] * e->populations[j] / k[i];
        }
        double dn = r[i] * e->populations[i] *
                    (1.0 - e->populations[i] / k[i] - competition) * dt;
        new_pop[i] = e->populations[i] + dn;
        if (new_pop[i] < 0.01) new_pop[i] = 0.01;
    }
    memcpy(e->populations, new_pop, sizeof(new_pop));
}

int cm_ecology_all_survive(const ecology_t *e, double threshold) {
    for (size_t i = 0; i < SPECIES_COUNT; i++) {
        if (e->populations[i] < threshold) return 0;
    }
    return 1;
}

double cm_ecology_resilience(const ecology_t *e) {
    size_t surviving = 0;
    for (size_t i = 0; i < SPECIES_COUNT; i++) {
        if (e->populations[i] >= 1.0) surviving++;
    }
    return (double)surviving / (double)SPECIES_COUNT;
}
