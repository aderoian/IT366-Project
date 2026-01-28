#include <stdio.h>
#include "gfc_list.h"

#include "physics.h"

#include "entity.h"

#define SOLVER_ITERATIONS 15

#define POSITIONAL_CORRECTION_PERCENT 0.8f
#define POSITIONAL_CORRECTION_SLOP 0.01f

static phys_BodyManager manager = {0};

/** @brief Buffer for storing collision pairs detected in the broad-phase */
static GFC_List* collisionPairBuffer = NULL;
/** @brief Buffer for storing collision contacts detected in the narrow-phase */
static GFC_List* collisionContactBuffer = NULL;

/** @brief Sweep-and-Prune Axis's */
static phys_SAPAxis c_sapXAxis;
static phys_SAPAxis c_sapYAxis;

void phys_init(const uint32_t initialSize) {
	manager.active = gfc_allocate_array(sizeof(struct Entity_S*), initialSize);
	manager.activeCount = 0;
	manager.activeCapacity = initialSize;
	phys_collision_initManager(initialSize);
}

void phys_free(void) {
	if (manager.active) {
		free(manager.active);
		manager.active = NULL;
		manager.activeCount = 0;
		manager.activeCapacity = 0;
	}
	phys_collision_freeManager();
}

void phys_addRigidbody(struct Entity_S *entity) {
	if (!entity) return;
	if (manager.activeCount >= manager.activeCapacity) {
		// Resize active rigidbody array
		uint32_t newCapacity = manager.activeCapacity * 2;
		struct Entity_S** newActive = gfc_allocate_array(sizeof(struct Entity_S*), newCapacity);
		if (!newActive) return;
		for (uint32_t i = 0; i < manager.activeCount; i++) {
			newActive[i] = manager.active[i];
		}
		free(manager.active);
		manager.active = newActive;
		manager.activeCapacity = newCapacity;
	}
	manager.active[manager.activeCount++] = entity;

	phys_collision_sweepAndPruneInsert(entity);
}
void phys_removeRigidbody(struct Entity_S *entity) {
	if (!entity) return;
	uint32_t index = UINT32_MAX;
	for (uint32_t i = 0; i < manager.activeCount; i++) {
		if (manager.active[i] == entity) {
			index = i;
			break;
		}
	}
	if (index == UINT32_MAX) return; // Not found

	phys_collision_sweepAndPruneRemove(entity);

	manager.active[index] = manager.active[--manager.activeCount];
}

void phys_addForce(Entity *ent, GFC_Vector2D force){
	gfc_vector2d_add(ent->forces, ent->forces, force);
}

void phys_addImpulse(Entity *ent, GFC_Vector2D impulse){
	GFC_Vector2D deltaV;
	gfc_vector2d_scale(deltaV, impulse, ent->invMass);
	gfc_vector2d_add(ent->velocity, ent->velocity, deltaV);
}

void phys_integrate(const phys_BodyManager *rb_manager, const float deltaTime) {
	uint32_t i;
	Entity *ent;
	GFC_Vector2D vel;
	if (rb_manager == NULL) {
		return;
	}

	for (i = 0; i < rb_manager->activeCount; i++) {
		ent = rb_manager->active[i];

		// Linear integration
		gfc_vector2d_scale(vel, ent->forces, ent->invMass);
		gfc_vector2d_scale(vel, vel, deltaTime);
		gfc_vector2d_add(ent->velocity, vel, ent->velocity);
		gfc_vector2d_scale(vel, ent->velocity, deltaTime);
		gfc_vector2d_add(ent->position, ent->position, vel);

		// Clear accumulator
		ent->forces = gfc_vector2d(0, 0);
	}
}

void phys_step(const float deltaTime) {
	// 1. Integrate rigidbodies (Apply forces, update velocities and positions)
	phys_integrate(&manager, deltaTime);

	// 2. Update collision bounding shapes
	phys_collision_updateBoundingShapes(&manager);

	// 3. Broad-phase collision: gather a list of pairs of potential colliding bodies
	phys_collision_detectBroadPhase(&manager);

	// 4. Narrow-phase collision: process the pairs and generate collision contacts
	phys_collision_detectNarrowPhase();

	// 5. Resolve collisions based on the detected contacts
	phys_collision_resolveCollisions();

	// 6. Process collision callbacks for involved rigidbodies
	//phys_collision_processCollisionCallbacks(&manager);
}

void phys_collision_initManager(const uint32_t initialSize) {
	collisionPairBuffer = gfc_list_new_size(initialSize);
	if (!collisionPairBuffer) {
		return;
	}

	collisionContactBuffer = gfc_list_new_size(initialSize);
	if (!collisionContactBuffer) {
		free(collisionPairBuffer);
		collisionPairBuffer = NULL;
	}

	phys_collision_sweepAndPruneInit(initialSize * 4);
}

void phys_collision_freeManager(void) {
	if (collisionPairBuffer) {
		gfc_list_delete(collisionPairBuffer);
		collisionPairBuffer = NULL;
	}
	if (collisionContactBuffer) {
		gfc_list_delete(collisionContactBuffer);
		collisionContactBuffer = NULL;
	}

	phys_collision_sweepAndPruneClose();
}

void phys_collision_updateBoundingShapes(const phys_BodyManager* rb_manager) {
	uint32_t i;
	Entity *ent;
	for (i = 0; i < rb_manager->activeCount; i++) {
		ent = rb_manager->active[i];

		gfc_vector2d_add(ent->worldBounds[0], ent->position, ent->localBounds[0]);
		gfc_vector2d_add(ent->worldBounds[1], ent->position, ent->localBounds[1]);

		phys_collision_sweepAndPruneUpdate(ent->sapIndex, ent->worldBounds);
	}
}

void phys_collision_detectBroadPhase(phys_BodyManager* rb_manager) {
	gfc_list_clear(collisionPairBuffer);
	// Sweep-and-Prune broad-phase collision detection
	phys_collision_sweepAndPruneSort(rb_manager);
	phys_collision_sweepAndPruneDetectAxisCollisions(&c_sapXAxis, rb_manager);
}

void phys_collision_detectNarrowPhase(void) {
	gfc_list_clear(collisionContactBuffer);

	uint32_t i;
	Entity *a, *b;
	phys_CollisionPair* pair;
	phys_CollisionContact *contact;
	GFC_Vector2D normal, contactPoint = {0};
	float penetration;

	for (i = 0; i < collisionPairBuffer->size; i++) {
		pair = (phys_CollisionPair*)gfc_list_get_nth(collisionPairBuffer, i);
		if (!pair) continue;

		a = pair->a;
		b = pair->b;

		if (!phys_collision_collideAABBAABB(a->position, a->localBounds, b->position, b->localBounds, &normal, &penetration, &contactPoint)) {
				continue;
		}

		contact = gfc_allocate_array(sizeof(phys_CollisionContact), 1);
		
		contact->a = pair->a;
		contact->b = pair->b;
		gfc_vector2d_copy(contact->contactPoint, contactPoint);
		gfc_vector2d_copy(contact->contactNormal, normal);
		gfc_vector2d_copy(contact->aVelocity, a->velocity);
		gfc_vector2d_copy(contact->bVelocity, b->velocity);
		contact->penetrationDepth = penetration;
		gfc_list_append(collisionContactBuffer, contact);
	}
}

void phys_collision_resolveContact(const phys_CollisionContact *contact) {
	Entity *a = contact->a, *b = contact->b;
	GFC_Vector2D tmp, rv, impulse, tangent;
	float velAlongNormal, e, j, jt, mu, len;

	// Velocity resolution (impulse) along the contact normal
	// rv = vb - va
	// rv_n = dot(rv, contactNormal)
	// j = -(1 + e) * rv_n / (invMassA + invMassB)
	// Avel -= j * contactNormal * invMassA
	// Bvel += j * contactNormal * invMassB

	gfc_vector2d_sub(rv, a->velocity, b->velocity);
	velAlongNormal = gfc_vector2d_dot_product(contact->contactNormal, rv);
	if (velAlongNormal > -0.001f) {
		return;
	}

	e = 0.0f; //TODO: no bounce, add restitution property to rigidbodies
	j = -(1 + e) * velAlongNormal / (a->invMass + b->invMass);
	gfc_vector2d_scale(impulse, contact->contactNormal, j);
	
	gfc_vector2d_scale(tmp, impulse, a->invMass);
	gfc_vector2d_sub(a->velocity, a->velocity, tmp);
	gfc_vector2d_scale(tmp, impulse, b->invMass);
	gfc_vector2d_add(b->velocity, b->velocity, tmp);

	// Tangental Velocity resolution (friction)
	gfc_vector2d_scale(tangent, contact->contactNormal, velAlongNormal);
	gfc_vector2d_sub(tangent, rv, tangent);
	gfc_vector2d_normalize(&tangent);

	jt = -gfc_vector2d_dot_product(rv, tangent) / (a->invMass + b->invMass);
	mu = 0.4f;

	// Coulomb friction model
	if (fabsf(jt) > j * mu)
		jt = j * mu * (jt < 0.0f ? -1.0f : 1.0f);

	GFC_Vector2D frictionImpulse;
	gfc_vector2d_scale(frictionImpulse, tangent, jt);
	len = gfc_vector2d_magnitude(tangent);
	len > 1e-6f ?gfc_vector2d_scale(tangent, tangent, 1.0f / len) : gfc_vector2d_copy(tangent, gfc_vector2d(0,0));
	gfc_vector2d_scale(tmp, frictionImpulse, a->invMass);
	gfc_vector2d_sub(a->velocity, a->velocity, tmp);
	gfc_vector2d_scale(tmp, frictionImpulse, b->invMass);
	gfc_vector2d_add(b->velocity, b->velocity, tmp);
}

void phys_collision_resolveContactPosition(const phys_CollisionContact *contact) {
	Entity *a = contact->a, *b = contact->b;
	GFC_Vector2D correction = {0}, tmp = {0};

	// Positional correction
	// correction = (penetrationDepth - slop) * percent * contactNormal
	// Apos -= correction * (invMassA / (invMassA + invMassB))
	// Bpos += correction * (invMassB / (invMassA + invMassB))
	gfc_vector2d_scale(correction, contact->contactNormal, fmaxf(contact->penetrationDepth - POSITIONAL_CORRECTION_SLOP, 0.0f) * POSITIONAL_CORRECTION_PERCENT);
	gfc_vector2d_scale(tmp, correction, a->invMass / (a->invMass + b->invMass));
	gfc_vector2d_sub(a->position, a->position, tmp);
	gfc_vector2d_scale(tmp, correction, b->invMass / (a->invMass + b->invMass));
	gfc_vector2d_add(b->position, b->position, tmp);
}


void phys_collision_resolveCollisions(void){
	uint32_t i;
	phys_CollisionContact* contact;
	for (uint32_t iter = 0; iter < SOLVER_ITERATIONS; iter++) {
		for (i = 0; i < collisionContactBuffer->size; i++) {
			contact = (phys_CollisionContact*)gfc_list_get_nth(collisionContactBuffer, i);
			if (contact) {
				phys_collision_resolveContact(contact);
			}
		}
	}

	phys_collision_resolveCollisionPositions();
}

void phys_collision_resolveCollisionPositions(void) {
	uint32_t i;
	phys_CollisionContact* contact;
	for (i = 0; i < collisionContactBuffer->size; i++) {
		contact = (phys_CollisionContact*)gfc_list_get_nth(collisionContactBuffer, i);
		if (contact) {
			phys_collision_resolveContactPosition(contact);
		}
	}
}

int phys_collision_checkAABBAABB(phys_AABBShape a, phys_AABBShape b) {
	return (a[0].x <= b[1].x && a[1].x >= b[0].x) &&
	       (a[0].y <= b[1].y && a[1].y >= b[0].y);
}

int phys_collision_collideAABBAABB(const GFC_Vector2D aPos, phys_AABBShape a, const GFC_Vector2D bPos, phys_AABBShape b,
									GFC_Vector2D *outNormal, float* outPenetration, GFC_Vector2D *outContactPoint){
	phys_AABBShape aWorld = {0}, bWorld = {0};
	float xOverlap, yOverlap;
	GFC_Vector2D normal, contactMin, contactMax, contactPoint;

	gfc_vector2d_add(aWorld[0], a[0], aPos);
	gfc_vector2d_add(aWorld[1], a[1], aPos);
	gfc_vector2d_add(bWorld[0], b[0], bPos);
	gfc_vector2d_add(bWorld[1], b[1], bPos);

	xOverlap = fminf(aWorld[1].x, bWorld[1].x) - fmaxf(aWorld[0].x, bWorld[0].x);
	yOverlap = fminf(aWorld[1].y, bWorld[1].y) - fmaxf(aWorld[0].y, bWorld[0].y);
	if (xOverlap <= 0 || yOverlap <= 0)
		return 0; // No collision

	// Find smallest overlap axis, compute normal and penetration depth
	*outPenetration = xOverlap;
	gfc_vector2d_copy(normal, gfc_vector2d(1, 0));
	if (aWorld[1].x > bWorld[1].x) {
		gfc_vector2d_negate(normal, normal);
	}
	if (yOverlap < *outPenetration) {
		*outPenetration = yOverlap;
		gfc_vector2d_copy(normal, gfc_vector2d(0, 1));
		if (aWorld[1].y > bWorld[1].y) {
			gfc_vector2d_negate(normal, normal);
		}
	}

	// Contact point (approximation)
	contactMin = gfc_vector2d(fmaxf(aWorld[0].x, bWorld[0].x), fmaxf(aWorld[0].y, bWorld[0].y));
	contactMax = gfc_vector2d(fminf(aWorld[1].x, bWorld[1].y), fminf(aWorld[1].y, bWorld[1].y));
	gfc_vector2d_add(contactPoint, contactMin, contactMax);
	gfc_vector2d_scale(contactPoint, contactPoint, 0.5f);
	gfc_vector2d_copy((*outNormal), normal);
	gfc_vector2d_copy((*outContactPoint), contactPoint);
	return 1;
}

void phys_collision_sweepAndPruneInit(const uint32_t initialSize) {
	phys_collision_sweepAndPruneInitAxis(&c_sapXAxis, initialSize);
	phys_collision_sweepAndPruneInitAxis(&c_sapYAxis, initialSize);
}

void phys_collision_sweepAndPruneClose(void) {
	phys_collision_sweepAndPruneCloseAxis(&c_sapXAxis);
	phys_collision_sweepAndPruneCloseAxis(&c_sapYAxis);
}

void phys_collision_sweepAndPruneInitAxis(phys_SAPAxis* axis, const uint32_t initialSize) {
	if (!axis) return;
	axis->endpoints = malloc(initialSize * sizeof(float));
	axis->indices = malloc(initialSize * sizeof(uint32_t));
	axis->isMin = malloc(initialSize * sizeof(uint8_t));
	axis->count = 0;
	axis->capacity = initialSize;
}

void phys_collision_sweepAndPruneCloseAxis(phys_SAPAxis* axis) {
	if (!axis) return;
	if (axis->endpoints) free(axis->endpoints);
	if (axis->indices) free(axis->indices);
	if (axis->isMin) free(axis->isMin);
}

void phys_collision_sweepAndPruneInsert(struct Entity_S *entity) {
	phys_collision_sweepAndPruneInsertAxis(&c_sapXAxis, entity, SAP_AXIS_X);
	phys_collision_sweepAndPruneInsertAxis(&c_sapYAxis, entity, SAP_AXIS_Y);
}

void phys_collision_sweepAndPruneInsertAxis(phys_SAPAxis* axis, struct Entity_S *entity, uint32_t axisIndex) {
	uint32_t minIdx, maxIdx;
	float minVal, maxVal;
	if (!axis) return;

	// Get bounding shape min and max for the specified axis
	minVal = axisIndex == SAP_AXIS_X ? entity->worldBounds[0].x : entity->worldBounds[0].y;
	maxVal = axisIndex == SAP_AXIS_X ? entity->worldBounds[1].x : entity->worldBounds[1].y;

	// Insert min endpoint
	// Find insertion index for min endpoint before finding max endpoint to avoid shifting issues
	minIdx = phys_collision_sweepAndPruneFindIndex(axis, minVal);
	phys_collision_sweepAndPruneInsertAt(axis, minIdx, entity, minVal, 1, axisIndex);

	// Insert max endpoint
	maxIdx = phys_collision_sweepAndPruneFindIndex(axis, maxVal);
	phys_collision_sweepAndPruneInsertAt(axis, maxIdx, entity, maxVal, 0, axisIndex);

	entity->sapIndex[0][axisIndex] = minIdx;
	entity->sapIndex[1][axisIndex] = maxIdx;
}

uint32_t phys_collision_sweepAndPruneFindIndex(phys_SAPAxis* axis, const float value) {
	uint32_t mid, low = 0, high;
	if (!axis) return UINT32_MAX;

	high = axis->count;
	while (low < high) {
		mid = (low + high) / 2;
		if (axis->endpoints[mid] < value) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}
	return low;
}

void phys_collision_sweepAndPruneInsertAt(phys_SAPAxis* axis, const uint32_t sap_index, struct Entity_S *entity, const float value, const uint8_t isMin, uint32_t axisIndex) {
	uint32_t i;
	if (!axis) return;
	if (axis->count >= axis->capacity)
		return; // TODO: Do we resize SAP? (would be costly)
	for (i = axis->count; i > sap_index; i--) {
		axis->endpoints[i] = axis->endpoints[i - 1];
		axis->indices[i] = axis->indices[i - 1];
		axis->isMin[i] = axis->isMin[i - 1];

		if (axis->isMin[i]) {
			axis->indices[i]->sapIndex[0][axisIndex] = i;
		} else {
			axis->indices[i]->sapIndex[1][axisIndex] = i;
		}
	}
	axis->endpoints[sap_index] = value;
	axis->indices[sap_index] = entity;
	axis->isMin[sap_index] = isMin;
	axis->count++;
}

void phys_collision_sweepAndPruneRemove(struct Entity_S *entity) {
	phys_collision_sweepAndPruneRemoveAxis(&c_sapXAxis, entity, SAP_AXIS_X);
	phys_collision_sweepAndPruneRemoveAxis(&c_sapYAxis, entity, SAP_AXIS_Y);
}

void phys_collision_sweepAndPruneRemoveAxis(phys_SAPAxis* axis, struct Entity_S *entity, uint32_t axisIndex) {
	uint32_t minIdx, maxIdx;

	// Get bounding shape min and max for the specified axis
	minIdx = entity->sapIndex[0][axisIndex];
	maxIdx = entity->sapIndex[1][axisIndex];
	phys_collision_sweepAndPruneRemoveAt(axis, maxIdx); // Remove max first to avoid shifting issues
	phys_collision_sweepAndPruneRemoveAt(axis, minIdx);
}

void phys_collision_sweepAndPruneRemoveAt(phys_SAPAxis* axis, const uint32_t sap_index) {
	if (!axis) return;
	if (sap_index >= axis->count) return;
	for (uint32_t i = sap_index; i < axis->count - 1; i++) {
		axis->endpoints[i] = axis->endpoints[i + 1];
		axis->indices[i] = axis->indices[i + 1];
		axis->isMin[i] = axis->isMin[i + 1];
	}
	axis->count--;
}

void phys_collision_sweepAndPruneUpdate(phys_CollisionHandle handle, phys_AABBShape worldBounding){
	c_sapXAxis.endpoints[handle[0][SAP_AXIS_X]] = worldBounding[0].x;
	c_sapXAxis.endpoints[handle[1][SAP_AXIS_X]] = worldBounding[1].x;
	c_sapYAxis.endpoints[handle[0][SAP_AXIS_Y]] = worldBounding[0].y;
	c_sapYAxis.endpoints[handle[1][SAP_AXIS_Y]] = worldBounding[1].y;
}

void phys_collision_sweepAndPruneSort(const phys_BodyManager* rb_manager) {
	uint32_t i;
	Entity *entity;
	for (i = 0; i < rb_manager->activeCount; i++) {
		entity = rb_manager->active[i];

		phys_collision_sweepAndPruneSortEndpoint(&c_sapXAxis, entity->sapIndex[0][SAP_AXIS_X], SAP_AXIS_X);
		phys_collision_sweepAndPruneSortEndpoint(&c_sapXAxis, entity->sapIndex[1][SAP_AXIS_X], SAP_AXIS_X);
		phys_collision_sweepAndPruneSortEndpoint(&c_sapYAxis, entity->sapIndex[0][SAP_AXIS_Y], SAP_AXIS_Y);
		phys_collision_sweepAndPruneSortEndpoint(&c_sapYAxis, entity->sapIndex[1][SAP_AXIS_Y], SAP_AXIS_Y);
	}
}

void phys_collision_sweepAndPruneSortEndpoint(const phys_SAPAxis* axis, uint32_t sap_index, const uint32_t axisIndex) {
	const float* eps = axis->endpoints;

	// Move left
	while (sap_index > 0 && eps[sap_index] < eps[sap_index - 1]) {
		phys_collision_sweepAndPrunSwapEndpoints(axis, sap_index, sap_index - 1, axisIndex);
		sap_index--;
	}

	// Move right
	while (sap_index < axis->count - 1 && eps[sap_index] > eps[sap_index + 1]) {
		phys_collision_sweepAndPrunSwapEndpoints(axis, sap_index, sap_index + 1, axisIndex);
		sap_index++;
	}
}

void phys_collision_sweepAndPrunSwapEndpoints(const phys_SAPAxis* axis, const uint32_t sap_indexA, const uint32_t sap_indexB, const uint32_t axisIndex) {
	float tmpVal;
	Entity *tmpIdx, *a, *b;
	uint8_t tmpIsMin;

	// Swap endpoints
	tmpVal = axis->endpoints[sap_indexA];
	axis->endpoints[sap_indexA] = axis->endpoints[sap_indexB];
	axis->endpoints[sap_indexB] = tmpVal;
	tmpIdx = axis->indices[sap_indexA];
	axis->indices[sap_indexA] = axis->indices[sap_indexB];
	axis->indices[sap_indexB] = tmpIdx;
	tmpIsMin = axis->isMin[sap_indexA];
	axis->isMin[sap_indexA] = axis->isMin[sap_indexB];
	axis->isMin[sap_indexB] = tmpIsMin;

	// Update collision handles in rigidbodys
	a = axis->indices[sap_indexA];
	b = axis->indices[sap_indexB];
	if (axis->isMin[sap_indexA]) {
		a->sapIndex[0][axisIndex] = sap_indexA;
	} else {
		a->sapIndex[1][axisIndex] = sap_indexA;
	}
	if (axis->isMin[sap_indexB]) {
		b->sapIndex[0][axisIndex] = sap_indexB;
	} else {
		b->sapIndex[1][axisIndex] = sap_indexB;
	}
}

void phys_collision_sweepAndPruneDetectAxisCollisions(phys_SAPAxis* axis, phys_BodyManager* rb_manager) {
	uint32_t activeCount = 0, i, j;
	Entity *a, *b, *active[1024];
	phys_CollisionPair collisionPair = {0};

	for (i = 0; i < axis->count; i++) {
		if (axis->isMin[i]) {
			a = axis->indices[i];

			// Check collisions with all active bounding shapes
			for (j = 0; j < activeCount; j++) {
				b = active[j];
				if (a == b) {
					continue; // Skip self
				}

				if (!phys_collision_sweepAndPruneCheckOverlap(a->worldBounds, b->worldBounds, SAP_AXIS_Y)) {
					continue; // No overlap on this axis
				}

				if (!(a->layers & b->layers)) {
					continue; // Layers do not match
				}

				// Emit pair
				collisionPair.a = axis->indices[i];
				collisionPair.b = b;
				gfc_list_append(collisionPairBuffer, &collisionPair);
			}

			// Add to active list
			active[activeCount++] = a;
		} else {
			// Remove from active list
			for (j = 0; j < activeCount; j++) {
				if (active[j] == axis->indices[i]) {
					active[j] = active[activeCount - 1];
					activeCount--;
					break;
				}
			}
		}
	}
}