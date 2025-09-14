#include "Physics.h"

namespace RiverCore {

void Physics::UpdatePhysics(std::vector<Entity>& entities, float deltaTime) {
    // Update physics for all entities that have physics enabled
    for (Entity& entity : entities) {
        if (entity.physApplied) {
            // Apply gravity
            ApplyGravity(entity, deltaTime);

            // Apply drag
            ApplyDrag(entity, deltaTime);

            // Integrate velocity into position
            IntegrateVelocity(entity, deltaTime);
        }
    }

    // Update collisions for all entities
    UpdateCollisions(entities);
}

void Physics::UpdateCollisions(std::vector<Entity>& entities) {
    // Clear all collision data
    for (Entity& entity : entities) {
        entity.collider.ClearCollisions();
    }

    // Check collisions between all entity pairs
    for (size_t i = 0; i < entities.size(); ++i) {
        Entity& entityA = entities[i];

        for (size_t j = i + 1; j < entities.size(); ++j) {
            Entity& entityB = entities[j];

            // Skip if either entity has no collision
            if (entityA.collider.type == ColliderType::NONE ||
                entityB.collider.type == ColliderType::NONE ||
                !entityA.collider.enabled || !entityB.collider.enabled) {
                continue;
            }

            if (CheckAABBCollision(entityA, entityB)) {
                // Determine collision sides
                float aFrameWidth = entityA.totalFrames > 1 ? (entityA.spriteWidth / 
                    static_cast<float>(entityA.totalFrames)) : entityA.spriteWidth;
                float bFrameWidth = entityB.totalFrames > 1 ? (entityB.spriteWidth / 
                    static_cast<float>(entityB.totalFrames)) : entityB.spriteWidth;

                float aWidth = aFrameWidth * abs(entityA.scale.x);
                float aHeight = entityA.spriteHeight * abs(entityA.scale.y);
                float bWidth = bFrameWidth * abs(entityB.scale.x);
                float bHeight = entityB.spriteHeight * abs(entityB.scale.y);

                float ax1 = entityA.position.x - (aWidth / 2.0f);
                float ax2 = entityA.position.x + (aWidth / 2.0f);
                float ay1 = entityA.position.y - (aHeight / 2.0f);
                float ay2 = entityA.position.y + (aHeight / 2.0f);

                float bx1 = entityB.position.x - (bWidth / 2.0f);
                float bx2 = entityB.position.x + (bWidth / 2.0f);
                float by1 = entityB.position.y - (bHeight / 2.0f);
                float by2 = entityB.position.y + (bHeight / 2.0f);

                // Check collision sides for entity A
                if (by1 >= ay1 && by1 <= ay2) {
                    entityA.collider.AddCollision(entityB.ID, 0); // top
                }
                if (bx2 >= ax1 && bx2 <= ax2) {
                    entityA.collider.AddCollision(entityB.ID, 1); // right
                }
                if (by2 >= ay1 && by2 <= ay2) {
                    entityA.collider.AddCollision(entityB.ID, 2); // bottom
                }
                if (bx1 >= ax1 && bx1 <= ax2) {
                    entityA.collider.AddCollision(entityB.ID, 3); // left
                }

                // Check collision sides for entity B (opposite directions)
                if (ay1 >= by1 && ay1 <= by2) {
                    entityB.collider.AddCollision(entityA.ID, 0); // top
                }
                if (ax2 >= bx1 && ax2 <= bx2) {
                    entityB.collider.AddCollision(entityA.ID, 1); // right
                }
                if (ay2 >= by1 && ay2 <= by2) {
                    entityB.collider.AddCollision(entityA.ID, 2); // bottom
                }
                if (ax1 >= bx1 && ax1 <= bx2) {
                    entityB.collider.AddCollision(entityA.ID, 3); // left
                }

                // Only resolve position if both entities are SOLID colliders
                if (entityA.collider.type == ColliderType::SOLID &&
                    entityB.collider.type == ColliderType::SOLID &&
                    entityA.physApplied && !entityB.physApplied) { // A is dynamic, B is static
                    float overlapX = std::min(ax2, bx2) - std::max(ax1, bx1);
                    float overlapY = std::min(ay2, by2) - std::max(ay1, by1);

                    // Resolve collision by moving A out of B along the smallest overlap
                    if (overlapX < overlapY) {
                        // Horizontal separation
                        if (entityA.position.x < entityB.position.x) {
                            entityA.position.x = bx1 - (aWidth / 2.0f); // Push left
                            entityA.velocity.x = std::min(0.0f, entityA.velocity.x);
                        } else {
                            entityA.position.x = bx2 + (aWidth / 2.0f); // Push right
                            entityA.velocity.x = std::max(0.0f, entityA.velocity.x);
                        }
                    } else {
                        // Vertical separation
                        if (entityA.position.y < entityB.position.y) {
                            entityA.position.y = by1 - (aHeight / 2.0f); // Push up (A above B)
                            entityA.velocity.y = std::min(0.0f, entityA.velocity.y);
                        } else {
                            entityA.position.y = by2 + (aHeight / 2.0f); // Push down (A below B)
                            entityA.velocity.y = std::max(0.0f, entityA.velocity.y);
                        }
                    }
                }
            }
        }
    }
}

bool Physics::CheckAABBCollision(const Entity& a, const Entity& b) const {
    float aFrameWidth = a.totalFrames > 1 ? (a.spriteWidth / static_cast<float>(a.totalFrames)) : a.spriteWidth;
    float bFrameWidth = b.totalFrames > 1 ? (b.spriteWidth / static_cast<float>(b.totalFrames)) : b.spriteWidth;

    float aWidth = aFrameWidth * abs(a.scale.x);
    float aHeight = a.spriteHeight * abs(a.scale.y);
    float bWidth = bFrameWidth * abs(b.scale.x);
    float bHeight = b.spriteHeight * abs(b.scale.y);

    float ax1 = a.position.x - (aWidth / 2.0f);
    float ax2 = a.position.x + (aWidth / 2.0f);
    float ay1 = a.position.y - (aHeight / 2.0f);
    float ay2 = a.position.y + (aHeight / 2.0f);

    float bx1 = b.position.x - (bWidth / 2.0f);
    float bx2 = b.position.x + (bWidth / 2.0f);
    float by1 = b.position.y - (bHeight / 2.0f);
    float by2 = b.position.y + (bHeight / 2.0f);

    return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
}

void Physics::ApplyForce(Entity& entity, const Vec2& force) {
    if (entity.mass > 0.0f) {
        entity.acceleration += force / entity.mass;
    }
}

void Physics::ApplyImpulse(Entity& entity, const Vec2& impulse) {
    if (entity.mass > 0.0f) {
        entity.velocity += impulse / entity.mass;
    }
}

void Physics::SetVelocity(Entity& entity, const Vec2& velocity) {
    entity.velocity = velocity;
}

void Physics::ApplyGravity(Entity& entity, float deltaTime) {
    Vec2 gravityForce = GetGravityVector() * entity.mass;
    ApplyForce(entity, gravityForce);
}

void Physics::ApplyDrag(Entity& entity, float deltaTime) {
    if (entity.drag > 0.0f) {
        Vec2 dragForce = entity.velocity * (-entity.drag);
        ApplyForce(entity, dragForce);
    }
}

void Physics::IntegrateVelocity(Entity& entity, float deltaTime) {
    // Update velocity from acceleration
    entity.velocity += entity.acceleration * deltaTime;

    // Update position from velocity
    entity.position += entity.velocity * deltaTime;

    // Reset acceleration (forces need to be applied each frame)
    entity.acceleration = Vec2::zero();
}

}
