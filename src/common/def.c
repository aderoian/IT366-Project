#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "simple_json.h"

#include "common/logger.h"
#include "common/def.h"

struct def_manager_s {
    def_t *definitions;
    size_t defMax;
};

void def_close(def_manager_t *manager) {
    size_t i;
    if (manager->definitions) {
        for (i = 0; i < manager->defMax; i++) {
            if (manager->definitions[i]._refc) {
                def_free(&manager->definitions[i]);
            }
        }
        free(manager->definitions);
        manager->definitions = NULL;
        manager->defMax = 0;
    }
}

def_manager_t *def_init(const size_t maxDefs) {
    def_manager_t *manager = malloc(sizeof(def_manager_t));
    if (!manager) {
        log_error("Failed to allocate memory for definition manager");
        return NULL;
    }

    manager->definitions = (def_t *)calloc(maxDefs, sizeof(def_t));
    if (!manager->definitions) {
        free(manager);
        log_error("Failed to allocate memory for definitions");
        return NULL;
    }
    manager->defMax = maxDefs;
    return manager;
}

def_t *def_new(const def_manager_t *manager) {
    size_t i;
    for (i = 0; i < manager->defMax; i++) {
        if (!manager->definitions[i]._refc) {
            manager->definitions[i]._refc += 1;
            return &manager->definitions[i];
        }
    }
    log_warn("No free definition slots available");
    return NULL;
}

def_data_t *def_load(const def_manager_t *manager, const char *filename) {
    def_t *def;
    def_data_t *data;
    size_t i;
    if (!filename) return NULL;
    for (i = 0; i < manager->defMax; i++) {
        if (manager->definitions[i]._refc && strcmp(manager->definitions[i].name, filename) == 0) {
            manager->definitions[i]._refc++;
            return manager->definitions[i].data;
        }
    }

    def = def_new(manager);
    if (!def) return NULL;

    data = sj_load(filename);
    if (!data) {
        log_error("Failed to load definition from file: %s", filename);
        def_free(def);
        return NULL;
    }

    def->name = strdup(filename);
    def->data = data;

    return data;
}

void def_load_directory(def_manager_t *manager, const char *directory) {
	struct stat status;
	mode_t mode;
	int result;
	DIR* dirP;
    struct dirent* entryP;
	char fName[1000];

    result = stat(directory, &status);
    if (result != 0) {
        log_error("Directory %s does not exist", directory);
        return;
    }

    mode = status.st_mode;
    if (!S_ISDIR(mode)) {
        log_error("%s is not a directory", directory);
        return;
    }

    dirP = opendir(directory);
    if (!dirP) {
        log_error("Failed to open directory: %s", directory);
        return;
    }

    while ((entryP = readdir(dirP)) != NULL) {
        if (entryP->d_type == DT_REG) {
            snprintf(fName, sizeof(fName), "%s/%s", directory, entryP->d_name);
            if (!def_load(manager, fName)) {
                log_error("Failed to load definition file: %s", fName);
            }
        }
    }

    closedir(dirP);
}

void def_free(def_t *def) {
    if (!def || !def->_refc) return;

    def->_refc--;
    if (def->_refc <= 0) return;

    if (def->name) {
        free(def->name);
        def->name = NULL;
    }
    if (def->data) {
        sj_free(def->data);
        def->data = NULL;
    }
    def->_refc = 0;
}

def_data_t *def_data_get_obj(def_data_t *def, const char *key) {
    return sj_object_get_value(def, key);
}

def_data_t *def_data_get_array(def_data_t *def, const char *key) {
    return sj_object_get_value(def, key);
}

const char *def_data_get_string(def_data_t *def, const char *key) {
    return sj_object_get_value_as_string(def, key);
}

const char * def_data_get_string_value(def_data_t *def) {
    return sj_get_string_value(def);
}

int def_data_get_int(def_data_t *def, const char *key, int *output) {
    return sj_object_get_value_as_int(def, key, output);
}

int def_data_get_int_value(def_data_t *def, int *output) {
    return sj_get_int32_value(def, output);
}

int def_data_get_float(def_data_t *def, const char *key, float *output) {
    return sj_object_get_value_as_float(def, key, output);
}

int def_data_get_float_value(def_data_t *def, float *output) {
    return sj_get_float_value(def, output);
}

int def_data_get_double(def_data_t *def, const char *key, double *output) {
    return sj_object_get_value_as_double(def, key, output);
}

def_data_t *def_data_array_get_nth(def_data_t *array, int n) {
    return sj_array_get_nth(array, n);
}

int def_data_array_get_count(def_data_t *array, int *count) {
    int c;
    c = sj_array_get_count(array);
    if (count) {
        *count = c;
    }
    return c;
}

int def_data_get_vector2d(def_data_t *def, const char *key, GFC_Vector2D *output) {
    def_data_t *valueArray;
    if (!def || !key || !output) return 0;

    valueArray = sj_object_get_value(def, key);
    if (!valueArray || !sj_is_array(valueArray)) return 0;

    if (sj_array_get_count(valueArray) < 2) return 0;

    if (!sj_get_float_value(sj_array_get_nth(valueArray, 0), &output->x)) return 0;
    if (!sj_get_float_value(sj_array_get_nth(valueArray, 1), &output->y)) return 0;

    return 1;
}

int def_data_get_vector2i(def_data_t *def, const char *key, GFC_Vector2I *output) {
    def_data_t *valueArray;
    if (!def || !key || !output) return 0;

    valueArray = sj_object_get_value(def, key);
    if (!valueArray || !sj_is_array(valueArray)) return 0;

    if (sj_array_get_count(valueArray) < 2) return 0;

    if (!sj_get_int32_value(sj_array_get_nth(valueArray, 0), &output->x)) return 0;
    if (!sj_get_int32_value(sj_array_get_nth(valueArray, 1), &output->y)) return 0;

    return 1;
}
