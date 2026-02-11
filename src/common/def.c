#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "simple_json.h"

#include "common/logger.h"
#include "common/def.h"

typedef struct def_tManager_s {
    def_t *definitions;
    int defMax;
} def_tManager;

def_tManager def_manager = {0};

void def_close(void) {
    int i;
    if (def_manager.definitions) {
        for (i = 0; i < def_manager.defMax; i++) {
            if (def_manager.definitions[i]._refc) {
                def_free(&def_manager.definitions[i]);
            }
        }
        free(def_manager.definitions);
        def_manager.definitions = NULL;
        def_manager.defMax = 0;
    }
}

void def_init(unsigned int maxDefs) {
    def_manager.definitions = (def_t *)calloc(maxDefs, sizeof(def_t));
    if (!def_manager.definitions) {
        log_error("Failed to allocate memory for definitions");
        return;
    }
    def_manager.defMax = maxDefs;
    atexit(def_close);
}

def_t *def_new(void) {
    int i;
    for (i = 0; i < def_manager.defMax; i++) {
        if (!def_manager.definitions[i]._refc) {
            def_manager.definitions[i]._refc += 1;
            return &def_manager.definitions[i];
        }
    }
    log_warn("No free definition slots available");
    return NULL;
}

def_data_t *def_load(const char *filename) {
    def_t *def;
    def_data_t *data;
    int i;
    if (!filename) return NULL;
    for (i = 0; i < def_manager.defMax; i++) {
        if (def_manager.definitions[i]._refc && strcmp(def_manager.definitions[i].name, filename) == 0) {
            def_manager.definitions[i]._refc++;
            return def_manager.definitions[i].data;
        }
    }

    def = def_new();
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

void def_load_directory(const char *directory) {
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
            if (!def_load(fName)) {
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

int def_data_get_int(def_data_t *def, const char *key, int *output) {
    return sj_object_get_value_as_int(def, key, output);
}

int def_data_get_float(def_data_t *def, const char *key, float *output) {
    return sj_object_get_value_as_float(def, key, output);
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