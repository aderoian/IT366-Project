#ifndef EDITOR_OVERLAY_H
#define EDITOR_OVERLAY_H

#include "client/ui/overlay.h"

extern overlay_t *g_editorOverlay;

void editor_overlay_init(overlay_t** overlay, size_t initialCapacity, uint8_t visible);

#endif /* EDITOR_OVERLAY_H */