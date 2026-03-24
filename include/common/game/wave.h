#ifndef WAVE_H
#define WAVE_H

#define rand_float(min, max) ((min) + ((float)rand() / (float)RAND_MAX) * ((max) - (min)))

#define wave_compute_budget(wave) (15.0f * powf(1.25f, (float)(wave)));

#include "enemy.h"

#define MAX_WAVE_ENEMIES 256

typedef struct {
    const enemy_def_t *enemies[MAX_WAVE_ENEMIES];
    int count;
} wave_t;

int wave_get_available_enemy(const enemy_def_t *enemyDefs, int count, int waveNumber, const enemy_def_t **outDefs);

const enemy_def_t *wave_pick_enemy(const enemy_def_t **enemyDefs,  int count);

void wave_generate(const enemy_def_t *enemyDefs, int count, int wave, wave_t *out);

#endif /* WAVE_H */