#include "common/game/collision.h"

#include "common/logger.h"
#include "common/game/world/chunk.h"

int collision_check(const entity_t *a, const entity_t *b) {
    GFC_Rect aBoundingBox, bBoundingBox;
    if (!a || !b) {
        return 0;
    }

    if ((a->layers & b->layers) == 0) {
        return 0; // No collision if layers don't match
    }

    aBoundingBox = a->boundingBox;
    bBoundingBox = b->boundingBox;
    aBoundingBox.x += a->position.x;
    aBoundingBox.y += a->position.y;
    bBoundingBox.x += b->position.x;
    bBoundingBox.y += b->position.y;

    if (!gfc_rect_overlap(aBoundingBox, bBoundingBox)) {
        return 0; // No collision if bounding boxes don't overlap
    }

    return 1; // Collision detected
}

int collision_check_chunk(const chunk_t *chunk, const entity_t *ent, GFC_Vector2D newPosition) {
    size_t i, j;
    entity_t *otherEnt;
    GFC_Rect aBoundingBox, bBoundingBox;
    if (!chunk || !ent) {
        return 0;
    }

    for (i = 0; i < gfc_list_count(chunk->entities); i++) {
        otherEnt = gfc_list_get_nth(chunk->entities, i);
        if (otherEnt == ent) continue; // Skip self
        if ((ent->layers & otherEnt->layers) == 0) continue; // Skip if layers don't match

        aBoundingBox = ent->boundingBox;
        bBoundingBox = otherEnt->boundingBox;
        aBoundingBox.x += newPosition.x;
        aBoundingBox.y += newPosition.y;
        bBoundingBox.x += otherEnt->position.x;
        bBoundingBox.y += otherEnt->position.y;

        if (gfc_rect_overlap(aBoundingBox, bBoundingBox)) {
            return 0; // Collision detected, cannot move
        }
    }

    return 1; // No collision detected, can move
}

int collision_check_world(const world_t *world, const entity_t *ent,
    GFC_Vector2D newPosition) {
    int chunkX, chunkY, i, j;
    chunk_t *chunk;
    if (!world || !ent) {
        return 0;
    }

    chunkX = pos_to_chunk_coord(newPosition.x);
    chunkY = pos_to_chunk_coord(newPosition.y);
    for (i = chunkX - 1; i <= chunkX + 1; i++) {
        for (j = chunkY - 1; j <= chunkY + 1; j++) {
            chunk = world_get_chunk(world, i, j);
            if (!chunk) continue;
            if (!collision_check_chunk(chunk, ent, newPosition)) {
                return 0; // Collision detected in this chunk, cannot move
            }
        }
    }

    return 1; // No collision detected in surrounding chunks, can move
}

int collision_check_chunk_bounding(const chunk_t *chunk, GFC_Rect boundingBox) {
    size_t i, j;
    entity_t *otherEnt;
    GFC_Rect otherBoundingBox;
    if (!chunk) {
        return 0;
    }

    for (i = 0; i < gfc_list_count(chunk->entities); i++) {
        otherEnt = gfc_list_get_nth(chunk->entities, i);

        otherBoundingBox = otherEnt->boundingBox;
        otherBoundingBox.x += otherEnt->position.x;
        otherBoundingBox.y += otherEnt->position.y;

        if (gfc_rect_overlap(boundingBox, otherBoundingBox)) {
            return 0; // Collision detected, bounding box is not clear
        }
    }

    return 1; // No collision detected, bounding box is clear
}

int collision_check_world_bounding(const world_t *world, GFC_Rect boundingBox) {
    int chunkXStart, chunkYStart, chunkXEnd, chunkYEnd, i, j;
    chunk_t *chunk;
    if (!world) {
        return 0;
    }

    chunkXStart = pos_to_chunk_coord(boundingBox.x);
    chunkYStart = pos_to_chunk_coord(boundingBox.y);
    chunkXEnd = pos_to_chunk_coord(boundingBox.x + boundingBox.w);
    chunkYEnd = pos_to_chunk_coord(boundingBox.y + boundingBox.h);

    for (i = chunkXStart; i <= chunkXEnd; i++) {
        for (j = chunkYStart; j <= chunkYEnd; j++) {
            chunk = world_get_chunk(world, i, j);
            if (!chunk) continue;

            if (!collision_check_chunk_bounding(chunk, boundingBox)) {
                return 0; // Collision detected in this chunk, bounding box is not clear
            }
        }
    }

    return 1; // No collision detected in surrounding chunks, bounding box is clear
}
