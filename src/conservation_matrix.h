/* conservation-matrix-c: Conservation laws in ternary agent systems */
#ifndef CONSERVATION_MATRIX_H
#define CONSERVATION_MATRIX_H

#include <stddef.h>
#include <math.h>

/* Ternary action */
typedef enum {
    TERNARY_AVOID  = -1,
    TERNARY_UNKNOWN = 0,
    TERNARY_CHOOSE = 1
} ternary_action_t;

/* Conservation tracker: monitors ratio conservation across generations */
#define CM_MAX_GENERATIONS 10000

typedef struct {
    double avoid_ratios[CM_MAX_GENERATIONS];
    double unknown_ratios[CM_MAX_GENERATIONS];
    double choose_ratios[CM_MAX_GENERATIONS];
    size_t count;
    size_t population_size;
} conservation_tracker_t;

void cm_tracker_init(conservation_tracker_t *t, size_t pop_size);
void cm_tracker_record(conservation_tracker_t *t, const ternary_action_t *actions, size_t n);
double cm_tracker_avoid_mean(const conservation_tracker_t *t);
double cm_tracker_choose_mean(const conservation_tracker_t *t);
double cm_tracker_unknown_mean(const conservation_tracker_t *t);
double cm_tracker_avoid_std(const conservation_tracker_t *t);
int cm_tracker_verify_conservation(const conservation_tracker_t *t, double threshold);

/* Fitness convergence */
typedef struct {
    double history[CM_MAX_GENERATIONS];
    size_t count;
    double target;
} fitness_convergence_t;

void cm_fitness_init(fitness_convergence_t *f, double target);
void cm_fitness_record(fitness_convergence_t *f, double value);
double cm_fitness_current(const fitness_convergence_t *f);
int cm_fitness_converged(const fitness_convergence_t *f, double tolerance);
double cm_fitness_improvement(const fitness_convergence_t *f);

/* Shannon entropy */
double cm_shannon_entropy(const double *proportions, size_t n);

/* Population advantage */
double cm_population_advantage(const double *pop_fitness, size_t n, double best_individual);
int cm_population_wins(const double *pop_fitness, size_t n, double best_individual);

/* Avoid:choose ratio */
double cm_avoid_choose_ratio(size_t avoid_count, size_t choose_count);

/* 5-species ecology */
typedef enum {
    SPECIES_EXPLORER  = 0,
    SPECIES_DIPLOMAT  = 1,
    SPECIES_MARKSMAN  = 2,
    SPECIES_CLIMBER   = 3,
    SPECIES_PROSPECTOR = 4,
    SPECIES_COUNT     = 5
} strategy_species_t;

typedef struct {
    double populations[SPECIES_COUNT];
    size_t n_species;
} ecology_t;

void cm_ecology_init(ecology_t *e, double initial_pop);
void cm_ecology_step(ecology_t *e, const double *growth_rates, const double *k,
                     const double alpha[5][5], double dt);
double cm_ecology_resilience(const ecology_t *e);
int cm_ecology_all_survive(const ecology_t *e, double threshold);

#endif /* CONSERVATION_MATRIX_H */
