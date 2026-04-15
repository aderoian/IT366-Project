#ifndef BUTTON_H
#define BUTTON_H
#include "client/ui/overlay.h"
#include "client/ui/widget.h"

overlay_element_t *button_create_image(GFC_Vector2D position, GFC_Vector2D size, const char *image, void (*onClick)(overlay_element_t *element));

widget_t *button_create(const char *id, GFC_Vector2D position, GFC_Vector2D size, const char *text, const char *image, void (*onClick)(widget_t *widget));
#endif /* BUTTON_H */