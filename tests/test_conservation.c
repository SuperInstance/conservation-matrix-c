/* Tests for conservation-matrix-c */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "../src/conservation_matrix.h"

#define ASSERT_FEQ(a, b, tol) do { \
    double _a = (a), _b = (b), _tol = (tol); \
    if (fabs(_a - _b) > _tol) { \
        fprintf(stderr, "FAIL %s:%d: %f != %f (tol %f)\n", \
                __FILE__, __LINE__, _a, _b, _tol); \
        return 1; \
    } \
} while(0)

int test_ternary_values() {
    assert(TERNARY_AVOID == -1);
    assert(TERNARY_UNKNOWN == 0);
    assert(TERNARY_CHOOSE == 1);
    return 0;
}

int test_tracker_record() {
    conservation_tracker_t t;
    cm_tracker_init(&t, 100);
    
    ternary_action_t actions[100];
    for (int i = 0; i < 50; i++) actions[i] = TERNARY_AVOID;
    for (int i = 50; i < 80; i++) actions[i] = TERNARY_UNKNOWN;
    for (int i = 80; i < 100; i++) actions[i] = TERNARY_CHOOSE;
    
    cm_tracker_record(&t, actions, 100);
    ASSERT_FEQ(cm_tracker_avoid_mean(&t), 0.5, 0.001);
    ASSERT_FEQ(cm_tracker_unknown_mean(&t), 0.3, 0.001);
    ASSERT_FEQ(cm_tracker_choose_mean(&t), 0.2, 0.001);
    return 0;
}

int test_conservation_across_scales() {
    int sizes[] = {10, 100, 1000, 5000};
    for (int s = 0; s < 4; s++) {
        int pop = sizes[s];
        conservation_tracker_t t;
        cm_tracker_init(&t, pop);
        
        ternary_action_t *actions = malloc(pop * sizeof(ternary_action_t));
        for (int g = 0; g < 50; g++) {
            for (int i = 0; i < pop; i++) {
                switch (i % 10) {
                    case 0: case 1: case 2: case 3: case 4:
                        actions[i] = TERNARY_AVOID; break;
                    case 5: case 6: case 7:
                        actions[i] = TERNARY_UNKNOWN; break;
                    default:
                        actions[i] = TERNARY_CHOOSE; break;
                }
            }
            cm_tracker_record(&t, actions, pop);
        }
        ASSERT_FEQ(cm_tracker_avoid_mean(&t), 0.5, 0.01);
        assert(cm_tracker_verify_conservation(&t, 0.02));
        free(actions);
    }
    return 0;
}

int test_fitness_convergence() {
    fitness_convergence_t f;
    cm_fitness_init(&f, 0.988);
    
    double vals[] = {0.803, 0.85, 0.90, 0.94, 0.97, 0.985, 0.988};
    for (int i = 0; i < 7; i++) cm_fitness_record(&f, vals[i]);
    
    ASSERT_FEQ(cm_fitness_current(&f), 0.988, 0.001);
    assert(cm_fitness_converged(&f, 0.01));
    ASSERT_FEQ(cm_fitness_improvement(&f), 0.185, 0.01);
    return 0;
}

int test_fitness_not_converged() {
    fitness_convergence_t f;
    cm_fitness_init(&f, 0.988);
    cm_fitness_record(&f, 0.803);
    cm_fitness_record(&f, 0.82);
    assert(!cm_fitness_converged(&f, 0.01));
    return 0;
}

int test_fitness_empty() {
    fitness_convergence_t f;
    cm_fitness_init(&f, 0.988);
    ASSERT_FEQ(cm_fitness_current(&f), 0.0, 0.001);
    assert(!cm_fitness_converged(&f, 0.01));
    ASSERT_FEQ(cm_fitness_improvement(&f), 0.0, 0.001);
    return 0;
}

int test_shannon_entropy_uniform() {
    double props[5] = {0.2, 0.2, 0.2, 0.2, 0.2};
    double h = cm_shannon_entropy(props, 5);
    ASSERT_FEQ(h, log2(5.0), 0.01);
    return 0;
}

int test_shannon_entropy_dominated() {
    double props[2] = {0.99, 0.01};
    double h = cm_shannon_entropy(props, 2);
    assert(h < 0.1);
    return 0;
}

int test_shannon_entropy_single() {
    double props[1] = {1.0};
    double h = cm_shannon_entropy(props, 1);
    ASSERT_FEQ(h, 0.0, 0.001);
    return 0;
}

int test_population_advantage() {
    double pop[] = {0.85, 0.90, 0.88, 0.92, 0.87};
    double adv = cm_population_advantage(pop, 5, 0.84);
    assert(adv > 0.0);
    assert(cm_population_wins(pop, 5, 0.84));
    return 0;
}

int test_population_loses() {
    double pop[] = {0.5, 0.6, 0.55};
    assert(!cm_population_wins(pop, 3, 0.9));
    return 0;
}

int test_avoid_choose_ratio() {
    double r = cm_avoid_choose_ratio(294, 1);
    ASSERT_FEQ(r, 294.0, 0.1);
    return 0;
}

int test_avoid_choose_ratio_zero() {
    double r = cm_avoid_choose_ratio(100, 0);
    ASSERT_FEQ(r, 0.0, 0.001);
    return 0;
}

int test_ecology_init() {
    ecology_t e;
    cm_ecology_init(&e, 20.0);
    for (int i = 0; i < SPECIES_COUNT; i++) {
        ASSERT_FEQ(e.populations[i], 20.0, 0.001);
    }
    return 0;
}

int test_ecology_all_survive() {
    ecology_t e;
    cm_ecology_init(&e, 20.0);
    
    double r[] = {1.0, 0.8, 1.2, 0.7, 0.5};
    double k[] = {100.0, 100.0, 100.0, 100.0, 100.0};
    double alpha[5][5] = {
        {1.0, 0.3, 0.2, 0.3, 0.2},
        {0.3, 1.0, 0.3, 0.2, 0.2},
        {0.2, 0.3, 1.0, 0.3, 0.3},
        {0.3, 0.2, 0.3, 1.0, 0.2},
        {0.2, 0.2, 0.3, 0.2, 1.0}
    };
    
    for (int step = 0; step < 5000; step++) {
        cm_ecology_step(&e, r, k, alpha, 0.01);
    }
    
    assert(cm_ecology_all_survive(&e, 1.0));
    ASSERT_FEQ(cm_ecology_resilience(&e), 1.0, 0.01);
    return 0;
}

int test_ecology_resilience_partial() {
    ecology_t e;
    cm_ecology_init(&e, 20.0);
    /* Kill 3 species */
    e.populations[2] = 0.001;
    e.populations[3] = 0.001;
    e.populations[4] = 0.001;
    assert(!cm_ecology_all_survive(&e, 1.0));
    ASSERT_FEQ(cm_ecology_resilience(&e), 0.4, 0.01);
    return 0;
}

int test_lv_two_species() {
    /* Classic 2-species competitive LV */
    double n1 = 50.0, n2 = 50.0;
    double r1 = 1.0, r2 = 1.0;
    double a12 = 0.5, a21 = 0.5;
    double k1 = 100.0, k2 = 100.0;
    double dt = 0.01;
    
    for (int i = 0; i < 10000; i++) {
        double dn1 = r1 * n1 * (1.0 - (n1 + a12 * n2) / k1) * dt;
        double dn2 = r2 * n2 * (1.0 - (n2 + a21 * n1) / k2) * dt;
        n1 += dn1; n2 += dn2;
        if (n1 < 0.01) n1 = 0.01;
        if (n2 < 0.01) n2 = 0.01;
    }
    
    assert(n1 > 1.0);
    assert(n2 > 1.0);
    ASSERT_FEQ(n1, 66.7, 10.0);
    ASSERT_FEQ(n2, 66.7, 10.0);
    return 0;
}

int test_tracker_empty() {
    conservation_tracker_t t;
    cm_tracker_init(&t, 100);
    ASSERT_FEQ(cm_tracker_avoid_mean(&t), 0.0, 0.001);
    ASSERT_FEQ(cm_tracker_avoid_std(&t), 0.0, 0.001);
    assert(cm_tracker_verify_conservation(&t, 0.01));
    return 0;
}

int main(void) {
    int failures = 0;
    int (*tests[])(void) = {
        test_ternary_values,
        test_tracker_record,
        test_conservation_across_scales,
        test_fitness_convergence,
        test_fitness_not_converged,
        test_fitness_empty,
        test_shannon_entropy_uniform,
        test_shannon_entropy_dominated,
        test_shannon_entropy_single,
        test_population_advantage,
        test_population_loses,
        test_avoid_choose_ratio,
        test_avoid_choose_ratio_zero,
        test_ecology_init,
        test_ecology_all_survive,
        test_ecology_resilience_partial,
        test_lv_two_species,
        test_tracker_empty
    };
    
    const char *names[] = {
        "ternary_values",
        "tracker_record",
        "conservation_across_scales",
        "fitness_convergence",
        "fitness_not_converged",
        "fitness_empty",
        "shannon_entropy_uniform",
        "shannon_entropy_dominated",
        "shannon_entropy_single",
        "population_advantage",
        "population_loses",
        "avoid_choose_ratio",
        "avoid_choose_ratio_zero",
        "ecology_init",
        "ecology_all_survive",
        "ecology_resilience_partial",
        "lv_two_species",
        "tracker_empty"
    };
    
    int n = sizeof(tests) / sizeof(tests[0]);
    printf("Running %d tests...\n", n);
    for (int i = 0; i < n; i++) {
        int result = tests[i]();
        if (result == 0) {
            printf("  PASS: %s\n", names[i]);
        } else {
            printf("  FAIL: %s\n", names[i]);
            failures++;
        }
    }
    printf("\n%d/%d passed\n", n - failures, n);
    return failures;
}
