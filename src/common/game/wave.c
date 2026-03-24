#include "common/game/wave.h"

#include "common/logger.h"

int wave_get_available_enemy(const enemy_def_t *enemyDefs, const int count, const int waveNumber, const enemy_def_t **outDefs) {
    int out_count = 0;

    for (int i = 0; i < count; i++) {
        if (waveNumber >= enemyDefs[i].minWave &&
            (enemyDefs[i].maxWave == -1 || waveNumber <= enemyDefs[i].maxWave)) {
            outDefs[out_count++] = &enemyDefs[i];
            }
    }

    return out_count;
}

const enemy_def_t *wave_pick_enemy(const enemy_def_t **enemyDefs, const int count) {
    int i;
    float total = 0.0f, r;

    for (i = 0; i < count; i++) {
        total += enemyDefs[i]->weight;
    }

    r = rand_float(0.0f, total);

    for (i = 0; i < count; i++) {
        r -= enemyDefs[i]->weight;
        if (r <= 0.0f) {
            return enemyDefs[i];
        }
    }

    return enemyDefs[count - 1];
}

void wave_generate(const enemy_def_t *enemyDefs, const int count, const int wave, wave_t *out) {
    const enemy_def_t *available[64], *e;
    float budget;
    int attempts = 0, available_count = wave_get_available_enemy(enemyDefs, count, wave, available);

    budget = wave_compute_budget(wave);
    out->count = 0;

    while (budget > 0.0f && out->count < MAX_WAVE_ENEMIES) {
        e = wave_pick_enemy(available, available_count);

        if (e->cost <= budget) {
            out->enemies[out->count++] = e;
            budget -= e->cost;
        }

        // prevent infinite loops
        if (++attempts > 1000) break;
    }
}
