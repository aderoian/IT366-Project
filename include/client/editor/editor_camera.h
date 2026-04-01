#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H
#include "gfc_vector.h"

struct entity_s;

struct entity_s *editor_camera_spawn(GFC_Vector2D position);

#endif /* EDITOR_CAMERA_H */