#include "common/game/collision.h"

#include "common/logger.h"
#include "common/game/world/chunk.h"

int collision_check(const entity_t *a, const entity_t *b) {
    GFC_Rect aBoundingBox, bBoundingBox;
    if (!a || !b) {
        return 0;
    }

    if (!a->collidesWith || !(a->collidesWith(a, b) & COLLISION_SOLID)) {
        return 0; // No collision if a doesn't collide with b
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
    uint32_t collisionType, collided = 0;
    if (!chunk || !ent) {
        return 0;
    }

    for (i = 0; i < gfc_list_count(chunk->entities); i++) {
        otherEnt = gfc_list_get_nth(chunk->entities, i);
        if (otherEnt == ent) continue; // Skip self
        if (!ent->collidesWith) continue; // Skip if not collidable
        collisionType = ent->collidesWith(ent, otherEnt);
        if (!collisionType) continue; // Skip if collidesWith returns no collision

        aBoundingBox = ent->boundingBox;
        bBoundingBox = otherEnt->boundingBox;
        aBoundingBox.x += newPosition.x;
        aBoundingBox.y += newPosition.y;
        bBoundingBox.x += otherEnt->position.x;
        bBoundingBox.y += otherEnt->position.y;

        if (gfc_rect_overlap(aBoundingBox, bBoundingBox)) {
            if (collisionType & COLLISION_SOLID) {
                if (ent->onCollide && !ent->onCollide(ent, otherEnt, collisionType)) {
                    continue;
                }
                return 1; // Collision detected, cannot move
            }

            if (ent->onCollide) {
                ent->onCollide(ent, otherEnt, collisionType);
            }
            // If it's not a solid collision, we still want to trigger the onCollide event, but it doesn't block movement
            collided = 1;
        }
    }

    return collided; // No collision detected, can move
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
            if (collision_check_chunk(chunk, ent, newPosition)) {
                return 1; // Collision detected in this chunk, cannot move
            }
        }
    }

    return 0;
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

int collision_raycast_chunk(const chunk_t *chunk, const entity_t *ent, GFC_Edge2D ray, GFC_List *hits) {
    size_t i, j;
    entity_t *otherEnt;
    GFC_Rect otherBoundingBox;
    if (!chunk || !hits) {
        return 0;
    }

    for (i = 0; i < gfc_list_count(chunk->entities); i++) {
        otherEnt = gfc_list_get_nth(chunk->entities, i);

        if (otherEnt == ent) continue; // Skip self
        if (!ent->collidesWith || !(ent->collidesWith(ent, otherEnt) & COLLISION_SOLID)) continue; // Skip if not collidable

        otherBoundingBox = otherEnt->boundingBox;
        otherBoundingBox.x += otherEnt->position.x;
        otherBoundingBox.y += otherEnt->position.y;

        if (gfc_edge_rect_intersection(ray, otherBoundingBox)) {
            gfc_list_append(hits, otherEnt);
        }
    }

    return 1; // Raycast completed for this chunk
}

int collision_raycast_world(const world_t *world, const entity_t *ent, GFC_Vector2D start, GFC_Vector2D end, GFC_List *hits) {
    int chunkXStart, chunkYStart, chunkXEnd, chunkYEnd, i, j;
    GFC_Edge2D edge;
    chunk_t *chunk;
    int hit = 0;
    if (!world || !hits) {
        return 0;
    }

    chunkXStart = pos_to_chunk_coord(fminf(start.x, end.x));
    chunkYStart = pos_to_chunk_coord(fminf(start.y, end.y));
    chunkXEnd = pos_to_chunk_coord(fmaxf(start.x, end.x));
    chunkYEnd = pos_to_chunk_coord(fmaxf(start.y, end.y));

    edge = gfc_edge_from_vectors(start, end);

    for (i = chunkXStart; i <= chunkXEnd; i++) {
        for (j = chunkYStart; j <= chunkYEnd; j++) {
            chunk = world_get_chunk(world, i, j);
            if (!chunk) continue;
            if (collision_raycast_chunk(chunk, ent, edge, hits)) {
                hit = 1; // At least one hit detected in this chunk
            }
        }
    }

    return hit; // Return whether any hits were detected across all chunks
}

void collision_get_entities_in_range_chunk(const chunk_t *chunk, GFC_Circle bounding, uint32_t layerMask, GFC_List *entitiesInRange) {
    size_t i, j;
    entity_t *otherEnt;
    if (!chunk || !entitiesInRange) {
        return;
    }

    for (i = 0; i < gfc_list_count(chunk->entities); i++) {
        otherEnt = gfc_list_get_nth(chunk->entities, i);

        if (!(otherEnt->layers & layerMask)) continue; // Skip if not in layer mask

        if (gfc_point_in_cicle(otherEnt->position, bounding)) {
            gfc_list_append(entitiesInRange, otherEnt);
        }
    }
}

GFC_List * collision_get_entities_in_range(const world_t *world, GFC_Vector2D position, float range,
    uint32_t layerMask) {
    int chunkXStart, chunkYStart, chunkXEnd, chunkYEnd, i, j;
    chunk_t *chunk;
    GFC_List *entitiesInRange;
    if (!world) {
        return NULL;
    }

    chunkXStart = pos_to_chunk_coord(position.x - range);
    chunkYStart = pos_to_chunk_coord(position.y - range);
    chunkXEnd = pos_to_chunk_coord(position.x + range);
    chunkYEnd = pos_to_chunk_coord(position.y + range);

    entitiesInRange = gfc_list_new();
    GFC_Circle rangeCircle = gfc_circle(position.x, position.y, range);
    for (i = chunkXStart; i <= chunkXEnd; i++) {
        for (j = chunkYStart; j <= chunkYEnd; j++) {
            chunk = world_get_chunk(world, i, j);
            if (!chunk) continue;
            collision_get_entities_in_range_chunk(chunk, rangeCircle, layerMask, entitiesInRange);
        }
    }

    return entitiesInRange;
}
