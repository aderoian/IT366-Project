#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdint.h>

#include "gfc_vector.h"

/** @brief A dense list of active bodies in the physics simulation */
typedef struct phys_BodyManager_S {
	struct Entity_S **active;
	uint32_t activeCount;
	uint32_t activeCapacity;
} phys_BodyManager;

/** @brief Represents a collision between 2 bodies. Stores the necessary information to resolve the collision.  */
typedef struct phys_CollisionContact_s {
	struct Entity_S *a;
	struct Entity_S *b;
	GFC_Vector2D contactPoint;
	GFC_Vector2D contactNormal;
	GFC_Vector2D aVelocity;
	GFC_Vector2D bVelocity;
	float penetrationDepth;
} phys_CollisionContact;

/** @brief Represents 2 bodies that could potentially be colliding. Generated in broad-phase. */
typedef struct phys_CollisionPair_S {
	struct Entity_S *a;
	struct Entity_S *b;
} phys_CollisionPair;

/** @brief Axis-Aligned Bounding Box shape defined by min and max points */
typedef GFC_Vector2D phys_AABBShape[2];

/**
 * @brief Initializes the physics system.
 * @param initialSize The initial size for the body manager's active body array.
 */
void phys_init(uint32_t initialSize);

/**
 * @brief Frees all resources used by the physics system.
 */
void phys_free(void);

/**
 * @brief Adds a body to the physics simulation.
 * @param entity The entity containing the body to add.
 */
void phys_addRigidbody(struct Entity_S *entity);

/**
 * @brief Removes a body from the physics simulation.
 * @param entity The entity containing the body to remove.
 */
void phys_removeRigidbody(struct Entity_S *entity);

/**
 * @brief Applies a force to the body of the given entity.
 * @param ent The entity whose body will receive the force.
 * @param force The force vector to apply.
 */
void phys_addForce(struct Entity_S *ent, GFC_Vector2D force);

/**
 * @brief Applies an impulse to the body of the given entity.
 * @param ent The entity whose body will receive the impulse.
 * @param impulse The impulse vector to apply.
 */
void phys_addImpulse(struct Entity_S *ent, GFC_Vector2D impulse);

/**
 * @brief Integrates the bodies managed by the given body manager over the specified time step.
 * Updates positions and velocities based on applied forces and impulses.
 * @param b_manager The body manager containing the bodies to integrate.
 * @param deltaTime The time step over which to integrate.
 */
void phys_integrate(const phys_BodyManager *b_manager, float deltaTime);

void phys_step(float deltaTime);

/**
 * @brief Initializes the collision manager.
 * @param initialSize The initial size for collision pair and contact buffers.
 */
void phys_collision_initManager(uint32_t initialSize);

/**
 * @brief Frees all resources used by the collision manager.
 */
void phys_collision_freeManager(void);

void phys_collision_updateBoundingShapes(const phys_BodyManager* b_manager);

/**
 * @brief Executes the broad-phase of collision detection.
 * Generates a list of collision pairs and other necessary information to be sent to the narrow-phase.
 * @param b_manager The body manager containing the bodies to check for collisions.
 */
void phys_collision_detectBroadPhase(phys_BodyManager* b_manager);

/**
 * @brief Executes the narrow-phase of collision detection.
 * Processes the collision pairs generated in the broad-phase to determine actual collisions and contact points.
 */
void phys_collision_detectNarrowPhase(void);

/**
 * @brief Checks for collision between two AABB shapes and calculates collision details if they collide.
 * @param aPos The position of the first AABB shape.
 * @param a The first AABB shape.
 * @param bPos The position of the second AABB shape.
 * @param b The second AABB shape.
 * @param outNormal Output parameter for the collision normal vector.
 * @param outPenetration Output parameter for the penetration depth.
 * @param outContactPoint Output parameter for the contact point of collision.
 * @return 1 if a collision is detected, 0 otherwise.
 */
int phys_collision_collideAABBAABB(GFC_Vector2D aPos, phys_AABBShape a, GFC_Vector2D bPos, phys_AABBShape b, GFC_Vector2D *outNormal, float* outPenetration, GFC_Vector2D *outContactPoint);

/**
 * @brief Resolves a single collision contact between two bodies.
 * @note This function handles ONLY velocity resolution.
 * @param contact The collision contact information to resolve.
 */
void phys_collision_resolveContact(const phys_CollisionContact *contact);

/**
 * @brief Resolves the positional aspect of a single collision contact between two bodies.
 * @param contact The collision contact information to resolve.
 */
void phys_collision_resolveContactPosition(const phys_CollisionContact *contact);

/**
 * @brief Resolves all detected collisions by processing the collision contacts.
 * @note This function handles both velocity and positional resolution.
 */
void phys_collision_resolveCollisions(void);

/**
 * @brief Resolves the positional aspect of all detected collisions.
 */
void phys_collision_resolveCollisionPositions(void);

/**
 * @brief Invokes each bodies collision callback if it was involved with a collision.
 * @param b_manager The body manager containing the bodies to process callbacks for.
 */
void phys_collision_processCollisionCallbacks(phys_BodyManager* b_manager);

#define SAP_AXIS_X 0
#define SAP_AXIS_Y 1
#define SAP_AXIS_Z 2

/**
 * @brief Data structure representing a Sweep and Prune axis (SAP) for broad-phase collision detection
 *
 * Used internally for broad-phase collision detection between dynamic bodies.
 * Each SAP axis contains an array of endpoints (min and max values) for each dynamic body,
 * along with their corresponding indices and flags indicating whether they are minimum or maximum endpoints.
 *
 * A SAP axis is used to efficiently identify potential collisions by sorting and sweeping through the endpoints.
 * When two bodies' intervals overlap on all three axes (X, Y, Z), they are considered potential collision pairs
 * and are further processed in the narrow-phase collision detection.
 */
typedef struct phys_SAPAxis_S {
	/** Array of min and max endpoint values for each dynamic body */
	float* endpoints;
	/** Array of corresponding indices of the bodies to the endpoints */
	struct Entity_S ** indices;
	uint8_t* isMin;
	uint32_t count;
	uint32_t capacity;
} phys_SAPAxis;

/** @brief Data structure representing a collision handle for a body
 * Used internally to track a body's position within the Sweep and Prune structure.
 * Contains indices for the min and max endpoints on each SAP axis (X, Y, Z).
 * Also includes flags for additional state information.
 */
typedef uint32_t phys_CollisionHandle[2][2];

/**
 * @brief Initializes the Sweep and Prune structure for broad-phase collision detection.
 * @param initialSize The initial size for the axis endpoint arrays.
 */
void phys_collision_sweepAndPruneInit(uint32_t initialSize);

/**
 * @brief Closes the Sweep and Prune structure and releases all associated memory.
 */
void phys_collision_sweepAndPruneClose(void);

/**
 * @brief Initializes a Sweep and Prune axis for broad-phase collision detection.
 * @param axis The Sweep and Prune axis to initialize.
 * @param initialSize The initial size for the axis endpoint arrays.
 */
void phys_collision_sweepAndPruneInitAxis(phys_SAPAxis* axis, uint32_t initialSize);

/**
 * @brief Closes a Sweep and Prune axis and releases all associated memory.
 * @param axis The Sweep and Prune axis to close.
 */
void phys_collision_sweepAndPruneCloseAxis(phys_SAPAxis* axis);

/**
 * @brief Inserts a body into the Sweep and Prune structure for broad-phase collision detection.
 * @param entity The entity containing the body to insert.
 */
void phys_collision_sweepAndPruneInsert(struct Entity_S *entity);

/**
 * @brief Inserts the min and max endpoints of a body into a Sweep and Prune axis.
 * @param axis The Sweep and Prune axis to insert into.
 * @param entity The entity containing the body to insert.
 * @param axisIndex The axis index (0 for X, 1 for Y, 2 for Z).
 */
void phys_collision_sweepAndPruneInsertAxis(phys_SAPAxis* axis, struct Entity_S *entity, uint32_t axisIndex);

/**
 * @brief Finds the appropriate index to insert a value into a Sweep and Prune axis.
 * Uses binary search for efficiency.
 * @param axis The Sweep and Prune axis to search.
 * @param value The value to find the insertion index for.
 * @return The index at which to insert the value.
 */
uint32_t phys_collision_sweepAndPruneFindIndex(phys_SAPAxis* axis, float value);

/**
 * @brief Inserts a value into a Sweep and Prune axis at the specified index.
 * @param axis The Sweep and Prune axis to insert into.
 * @param sap_index The index at which to insert the value.
 * @param entity The entity containing the body to insert.
 * @param value The value to insert.
 * @param isMin Flag indicating whether the value is a minimum endpoint (1) or maximum endpoint (0).
 * @param axisIndex The axis index (0 for X, 1 for Y, 2 for Z).
 */
void phys_collision_sweepAndPruneInsertAt(phys_SAPAxis* axis, uint32_t sap_index, struct Entity_S *entity, float value, uint8_t isMin, uint32_t axisIndex);

/**
 * @brief Removes a body from the Sweep and Prune structure for broad-phase collision detection.
 * @param entity The entity containing the body to remove.
 */
void phys_collision_sweepAndPruneRemove(struct Entity_S *entity);

/** @brief Removes the min and max endpoints of a body from a Sweep and Prune axis.
 * @param axis The Sweep and Prune axis to remove from.
 * @param entity The entity containing the body to remove.
 * @param axisIndex The axis index (0 for X, 1 for Y, 2 for Z).
 */
void phys_collision_sweepAndPruneRemoveAxis(phys_SAPAxis* axis, struct Entity_S *entity, uint32_t axisIndex);

/**
 * @brief Removes an endpoint from a Sweep and Prune axis at the specified index.
 * @param axis The Sweep and Prune axis to remove from.
 * @param sap_index The index of the endpoint to remove.
 */
void phys_collision_sweepAndPruneRemoveAt(phys_SAPAxis* axis, uint32_t sap_index);

/**
 * @brief Updates a body's position in the Sweep and Prune structure for broad-phase collision detection.
 * @param handle The collision handle of the body to update.
 * @param worldBounding The new world-space AABB shape of the body.
 */
void phys_collision_sweepAndPruneUpdate(phys_CollisionHandle handle, phys_AABBShape worldBounding);

/**
 * @brief Sorts the endpoints of a Sweep and Prune using insertion sort.
 * @param b_manager The body manager containing the bodies associated with the axis.
 */
void phys_collision_sweepAndPruneSort(const phys_BodyManager* b_manager);

/**
 * @brief Sorts a single endpoint in a Sweep and Prune axis using insertion sort.
 * @param axis The Sweep and Prune axis to sort.
 * @param sap_index The index of the endpoint to sort.
 * @param axisIndex The axis index (0 for X, 1 for Y, 2 for Z).
 */
void phys_collision_sweepAndPruneSortEndpoint(const phys_SAPAxis* axis, uint32_t sap_index, uint32_t axisIndex);

/**
 * @brief Swaps two endpoints in a Sweep and Prune axis.
 * @param axis The Sweep and Prune axis containing the endpoints to swap.
 * @param sap_indexA The index of the first endpoint to swap.
 * @param sap_indexB The index of the second endpoint to swap.
 * @param axisIndex The axis index (0 for X, 1 for Y, 2 for Z).
 */
void phys_collision_sweepAndPrunSwapEndpoints(const phys_SAPAxis* axis, uint32_t sap_indexA, uint32_t sap_indexB, uint32_t axisIndex);

/**
 * @brief Detects potential collisions along a Sweep and Prune axis and updates the collision pair buffer.
 * @param axis The Sweep and Prune axis to detect collisions on.
 * @param b_manager The body manager containing the bodies associated with the axis.
 */
void phys_collision_sweepAndPruneDetectAxisCollisions(phys_SAPAxis* axis, phys_BodyManager* b_manager);

/** @brief Checks for overlap between two bodies along a specified axis in the Sweep and Prune algorithm.
 * @param a The AABB shape of the first body.
 * @param b The AABB shape of the second body.
 * @param axisIndex The axis index to check for overlap (0 for X, 1 for Y).
 * @return 1 if there is overlap on the specified axis, 0 otherwise
 */
static inline int phys_collision_sweepAndPruneCheckOverlap(phys_AABBShape a, phys_AABBShape b, const uint32_t axisIndex) {
	if (axisIndex == SAP_AXIS_X) {
		return !(a[1].x < b[0].x || b[1].x < a[0].x);
	} else {
		return !(a[1].y < b[0].y || b[1].y < a[0].y);
	}
}

#endif // PHYSICS_H