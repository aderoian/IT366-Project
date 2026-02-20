#ifndef DEF_H
#define DEF_H

#include <gfc_types.h>

#include "gfc_vector.h"

typedef struct SJson_S def_data_t;

typedef struct Definition_s {
    Uint32 _refc;
    char *name;
    def_data_t *data;
} def_t;

void def_init(unsigned int maxDefs);

/**
 * @brief load a definition file
 * @param filename the file to load
 * @return NULL on error or the loaded definition
 */
def_data_t *def_load(const char *filename);

/**
 * @brief load all definition files in a directory
 * @param directory the directory to load from
 */
void def_load_directory(const char *directory);

/**
 * @brief free a previously loaded definition
 * @param def the definition to free
 */
void def_free(def_t *def);

/**
 * @brief Get an object value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @return NULL on error or the definition data value
 */
def_data_t *def_data_get_obj(def_data_t *def, const char *key);

/**
 * @brief Get an array value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @return NULL on error or the definition data value
 */
def_data_t *def_data_get_array(def_data_t *def, const char *key);

/**
 * @brief Get an string value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @return NULL on error or the definition data value
 */
const char *def_data_get_string(def_data_t *def, const char *key);

/**
 * @brief Get a string value from a definition data
 * @param def the definition data to search
 * @return NULL on error or the definition data value
 */
const char *def_data_get_string_value(def_data_t *def);

/**
 * @brief Get an int value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_int(def_data_t *def, const char *key, int *output);

/**
 * @brief Get an int value from a definition data
 * @param def the definition data to search
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_int_value(def_data_t *def, int *output);

/**
 * @brief Get a float value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_float(def_data_t *def, const char *key, float *output);

/**
 * @brief Get a float value from a definition data
 * @param def the definition data to search
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_float_value(def_data_t *def, float *output);

/**
 * @brief Get a double value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_double(def_data_t *def, const char *key, double *output);

/**
 * @brief retrieve the nth element in the definition data array
 * @param array the definition data array
 * @param n the index of the element to get
 * @return NULL on error or the def_data_t value otherwise
 */
def_data_t *def_data_array_get_nth(def_data_t *array, int n);

/**
 * @brief get the count of elements in a definition data array
 * @param array the definition data array
 * @param count where the count is written to
 * @return 0 on error, 1 if the count was retrieved successfully
 */
int def_data_array_get_count(def_data_t *array, int *count);

int def_data_get_vector2d(def_data_t *def, const char *key, GFC_Vector2D *output);

#endif /* DEF_H */