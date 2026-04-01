#ifndef BUTTON_H
#define BUTTON_H
#include "client/ui/overlay.h"

overlay_element_t *button_create_image(GFC_Vector2D position, GFC_Vector2D size, const char *image, void (*onClick)(overlay_element_t *element));

#endif /* BUTTON_H */