#ifndef INPUT_FIELD_H
#define INPUT_FIELD_H

#include "gfc_vector.h"
#include "client/ui/overlay.h"

overlay_element_t *input_field_create(GFC_Vector2D position, GFC_Vector2D size, const char *placeholder, int fontSize);

const char *input_field_get_text(struct overlay_element_s *element);

#endif /* INPUT_FIELD_H */