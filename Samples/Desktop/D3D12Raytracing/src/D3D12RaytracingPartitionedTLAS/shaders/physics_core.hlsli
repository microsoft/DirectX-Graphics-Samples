//*********************************************************
//
// Copyright (c) 2024-2025, NVIDIA CORPORATION. All rights reserved.
// Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// Domino physics, PCG and WangHash adapted from
// vk_partitioned_tlas/shaders/animation_physics.comp.glsl.
// See THIRD_PARTY_NOTICES.md for source revisions and acknowledgments.

// Rigid-body motion, ground contact, and neighboring-domino collisions.

// The solver supplies a repeatable workload that eventually stops changing
// transforms. Ground contact, path-neighbor collisions, and the settling rules
// below are scene-specific application policy. PTLAS consumes the resulting
// writes and has no knowledge of contact ages or sleeping bodies.
// All age thresholds use rendered FrameIndex ticks, which continue during
// pause. The solver applies those ages on the next running frame/single step.

static const float3 Gravity = float3(0.0, -9.81, 0.0);
static const float3 GroundNormal = float3(0.0, 1.0, 0.0);

float3x3 transform3(Motion motion)
{
    return float3x3(motion.row0.xyz, motion.row1.xyz, motion.row2.xyz);
}

float3x3 inverse3(float3x3 matrix)
{
    float3 a = matrix[0], b = matrix[1], c = matrix[2];
    float determinant = dot(a, cross(b, c));
    if(abs(determinant) < 1e-8) return float3x3(1,0,0, 0,1,0, 0,0,1);
    return transpose(float3x3(cross(b, c), cross(c, a), cross(a, b))) / determinant;
}

struct Box
{
    float3 corners[8];
};

Box boxOf(Motion motion)
{
    Box box;
    [unroll] for(uint index = 0; index < 8; ++index)
    {
        const float3 local = float3((index & 1u) != 0 ? 0.5 : -0.5,
                                    (index & 2u) != 0 ? 0.5 : -0.5,
                                    (index & 4u) != 0 ? 0.5 : -0.5);
        box.corners[index] = transformPoint(motion, local);
    }
    return box;
}

float3 cornerOf(Box box, uint index)
{
    return box.corners[index];
}

struct Collision
{
    float3 contactPoint;
    float3 normal;
    float penetration;
};

bool detectCollision(Box a, Box b, out Collision collision)
{
    float3 axes[15];
    axes[0] = normalize(cornerOf(a, 1) - cornerOf(a, 0));
    axes[1] = normalize(cornerOf(a, 3) - cornerOf(a, 0));
    axes[2] = normalize(cornerOf(a, 4) - cornerOf(a, 0));
    axes[3] = normalize(cornerOf(b, 1) - cornerOf(b, 0));
    axes[4] = normalize(cornerOf(b, 3) - cornerOf(b, 0));
    axes[5] = normalize(cornerOf(b, 4) - cornerOf(b, 0));
    uint axisCount = 6;
    [unroll] for(uint i = 0; i < 3; ++i)
        [unroll] for(uint j = 3; j < 6; ++j)
        {
            float3 axis = cross(axes[i], axes[j]);
            if(dot(axis, axis) > 1e-12)
                axes[axisCount++] = normalize(axis);
        }
    float minimum = 1e30;
    float3 minimumAxis = GroundNormal;
    float3 supportA = cornerOf(a, 0);
    float3 supportB = cornerOf(b, 0);
    [loop] for(uint index = 0; index < axisCount; ++index)
    {
        const float3 axis = axes[index];
        float minA = dot(cornerOf(a, 0), axis);
        float maxA = minA;
        float minB = dot(cornerOf(b, 0), axis);
        float maxB = minB;
        [unroll] for(uint corner = 1; corner < 8; ++corner)
        {
            float projectionA = dot(cornerOf(a, corner), axis);
            float projectionB = dot(cornerOf(b, corner), axis);
            minA = min(minA, projectionA);
            maxA = max(maxA, projectionA);
            minB = min(minB, projectionB);
            maxB = max(maxB, projectionB);
        }
        float penetration = min(maxA, maxB) - max(minA, minB);
        if(penetration < 0.0) return false;
        if(penetration < minimum)
        {
            minimum = penetration;
            minimumAxis = axis;
            supportA = cornerOf(a, 0);
            supportB = cornerOf(b, 0);
            [unroll] for(uint corner = 1; corner < 8; ++corner)
            {
                float3 pointA = cornerOf(a, corner);
                float3 pointB = cornerOf(b, corner);
                if(dot(pointA, axis) < dot(supportA, axis)) supportA = pointA;
                if(dot(pointB, axis) > dot(supportB, axis)) supportB = pointB;
            }
        }
    }
    float offset = dot(supportA - supportB, minimumAxis) * 0.5;
    collision.contactPoint = ((supportA - minimumAxis * offset) + (supportB + minimumAxis * offset)) * 0.5;
    collision.normal = minimumAxis;
    collision.penetration = minimum;
    return true;
}

float minimumY(Box box)
{
    float value = 1e30;
    [unroll] for(uint index = 0; index < 8; ++index)
        value = min(value, cornerOf(box, index).y);
    return value;
}

float3 lowestCorner(Box box)
{
    float3 selected = cornerOf(box, 0);
    [unroll] for(uint index = 1; index < 8; ++index)
    {
        float3 corner = cornerOf(box, index);
        if(corner.y < selected.y) selected = corner;
    }
    return selected;
}

void handleGround(inout Motion motion, Box box, float step)
{
    float3 contactPoint = 0;
    float lowest = 1e30;
    uint contacts = 0, nearContacts = 0;
    [unroll] for(uint index = 0; index < 8; ++index)
    {
        float3 corner = cornerOf(box, index);
        if(corner.y <= 0.0)
        {
            ++contacts;
            lowest = min(lowest, corner.y);
            contactPoint += corner;
        }
        if(abs(corner.y) < 0.01) ++nearContacts;
    }
    // After 50 frames without a domino contact/topple, three corners within
    // 0.01 of the ground qualify for extra damping. The latched 0x8 flag also
    // selects the ground-settled collision correction in handleInteraction.
    if(nearContacts > 2 && motion.lastContact != 0xffffffffu
       && Parameters.FrameIndex - motion.lastContact > 50u)
    {
        motion.flags |= 8u;
        motion.angularVelocity = 0;
        motion.linearVelocity *= float3(0.1, 0.0, 0.1);
        return;
    }
    if(contacts != 0)
    {
        motion.flags = (motion.flags & ~3u) | 1u;
        contactPoint /= contacts;
        float3x3 scale = transform3(motion);
        float3x3 inertiaInverse = inverse3(mul(transpose(scale), scale));
        float3 r = contactPoint - position(motion);
        float3 velocity = motion.linearVelocity - cross(motion.angularVelocity, r);
        if(length(velocity) < 0.1) velocity = 0;
        float denominator = 1.0 + dot(cross(mul(inertiaInverse, cross(r, GroundNormal)), r), GroundNormal);
        float impulse = -1.1 * dot(velocity, GroundNormal) / max(abs(denominator), 0.001);
        motion.linearVelocity += impulse * GroundNormal;
        float3 tangent = velocity - dot(velocity, GroundNormal) * GroundNormal;
        if(dot(tangent, tangent) > 1e-8)
            motion.linearVelocity -= normalize(tangent) * length(tangent);
        float3 gravityTorque = cross(r, Gravity);
        if(length(gravityTorque) < 0.1) gravityTorque = 0;
        motion.angularVelocity += mul(inertiaInverse, gravityTorque) * step;
        float3 center = position(motion);
        center.y += max(0.0, -lowest);
        setPosition(motion, center);
    }
    else
        motion.linearVelocity += Gravity * step;
    if(position(motion).y < 0.1 && length(motion.linearVelocity) < 0.01
       && length(motion.angularVelocity) < 0.01)
    {
        motion.linearVelocity = 0;
        motion.angularVelocity = 0;
        float3 center = position(motion);
        center.y = max(center.y, 0.1);
        setPosition(motion, center);
    }
}

uint wangHash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16);
    seed *= 9u;
    seed ^= seed >> 4;
    seed *= 0x27d4eb2du;
    return seed ^ (seed >> 15);
}

// PCG origin reference: https://www.pcg-random.org/index.html
uint pcg(inout uint seed)
{
    uint value = seed * 747796405u + 2891336453u;
    uint word = ((value >> ((value >> 28u) + 4u)) ^ value) * 277803737u;
    seed = value;
    return (word >> 22u) ^ word;
}

bool handleInteraction(inout Motion previous, Box boxA, inout Motion current, Box boxB)
{
    // Stop resolving this pair once its 'current' domino is more than 300
    // frames past first contact. This limits persistent contact churn in the
    // demonstration; adapt this policy when replacing the scene's physics.
    if(current.firstContact != 0xffffffffu && Parameters.FrameIndex - current.firstContact > 300u)
        return false;
    Collision collision;
    if(!detectCollision(boxA, boxB, collision)) return false;
    if(current.firstContact == 0xffffffffu) current.firstContact = Parameters.FrameIndex;
    current.flags = (current.flags & ~3u) | 2u;
    previous.flags = (previous.flags & ~3u) | 2u;
    float3 bottomA = lowestCorner(boxA);
    float3 bottomB = lowestCorner(boxB);
    if((current.flags & 8u) != 0)
    {
        setPosition(current, position(current) - collision.normal * collision.penetration * 0.5);
        current.linearVelocity -= collision.normal * collision.penetration * 0.5;
    }
    float3 groundB = transformPoint(current, float3(0, -0.5, 0));
    float distanceB = length(collision.contactPoint - groundB);
    if(distanceB > 1e-6 && length(current.angularVelocity) < 1.0)
        current.angularVelocity += clamp(-5.0 * cross(collision.normal, GroundNormal)
            * collision.penetration / distanceB, -1.0, 1.0);
    float3 direction = normalize(position(current) - position(previous));
    float correctionSign = dot(direction, collision.normal) < 0.0 ? 1.0 : -1.0;
    bottomB += correctionSign * collision.normal * collision.penetration * 0.5;
    if(bottomB.y < 0.0)
    {
        float3 center = position(current);
        center.y -= bottomB.y;
        setPosition(current, center);
        current.linearVelocity.y = 0;
    }
    setPosition(previous, position(previous) + collision.normal * collision.penetration * 0.5);
    previous.linearVelocity += collision.normal * collision.penetration * 0.5;
    float3 groundA = transformPoint(previous, float3(0, -0.5, 0));
    float distanceA = length(collision.contactPoint - groundA);
    if(distanceA > 1e-6 && length(previous.angularVelocity) < 1.0)
        previous.angularVelocity += clamp(5.0 * cross(collision.normal, GroundNormal)
            * collision.penetration / distanceA, -1.0, 1.0);
    bottomA -= correctionSign * collision.normal * collision.penetration * 0.5;
    if(bottomA.y < 0.0)
    {
        float3 center = position(previous);
        center.y -= bottomA.y;
        setPosition(previous, center);
        previous.linearVelocity.y = 0;
    }
    float3 rA = collision.contactPoint - position(previous);
    float3 rB = collision.contactPoint - position(current);
    float3 relative = current.linearVelocity + cross(current.angularVelocity, rB)
        - previous.linearVelocity - cross(previous.angularVelocity, rA);
    float relativeNormal = dot(relative, collision.normal);
    if(relativeNormal > 0.0) return false;
    float3x3 inverseA = inverse3(transform3(previous));
    float3x3 inverseB = inverse3(transform3(current));
    float angularB = dot(cross(mul(inverseB, cross(rB, collision.normal)), rB), collision.normal);
    float angularA = dot(cross(mul(inverseA, cross(rA, collision.normal)), rA), collision.normal);
    float magnitude = -1.1 * relativeNormal / max(2.0 + abs(angularB - angularA), 0.001);
    float3 impulse = magnitude * 0.2 * collision.normal;
    current.linearVelocity += impulse;
    previous.linearVelocity -= impulse;
    current.angularVelocity -= mul(inverseB, cross(rB, impulse));
    previous.angularVelocity += mul(inverseA, cross(rA, impulse));
    uint seed = wangHash(asuint(position(current).x));
    [unroll] for(uint axis = 0; axis < 3; ++axis)
    {
        current.linearVelocity[axis] *= 1.0 + (2.0 * float(pcg(seed)) / 4294967295.0 - 1.0) * 0.01;
        current.angularVelocity[axis] *= 1.0 + (2.0 * float(pcg(seed)) / 4294967295.0 - 1.0) * 0.01;
    }
    return true;
}

void rotateMotion(inout Motion motion, float step)
{
    float angle = length(motion.angularVelocity) * step;
    if(angle <= 0.0) return;
    float3 axis = normalize(motion.angularVelocity);
    float s = sin(angle), c = cos(angle), oc = 1.0 - c;
    float3x3 rotation = float3x3(
        oc * axis.x * axis.x + c, oc * axis.x * axis.y + axis.z * s, oc * axis.x * axis.z - axis.y * s,
        oc * axis.x * axis.y - axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z + axis.x * s,
        oc * axis.x * axis.z + axis.y * s, oc * axis.y * axis.z - axis.x * s, oc * axis.z * axis.z + c);
    float3x3 oriented = mul(rotation, transform3(motion));
    motion.row0.xyz = oriented[0];
    motion.row1.xyz = oriented[1];
    motion.row2.xyz = oriented[2];
}

bool computeMotion(uint index, inout Motion current, inout Motion previous, inout Motion next)
{
    current.flags &= ~3u;
    float4 original0 = current.row0, original1 = current.row1, original2 = current.row2;
    bool forced = (current.flags & 4u) != 0;
    if(forced)
    {
        float3 direction = normalize(transformPoint(current, float3(0, 0, 1)) - position(current));
        current.angularVelocity += direction * 5.0;
        current.flags &= ~4u;
    }
    Box box = boxOf(current);
    handleGround(current, box, Parameters.PhysicsStep);
    if(index > 0) handleInteraction(previous, boxOf(previous), current, box);
    if(index + 1 < Parameters.InstanceCount) handleInteraction(current, box, next, boxOf(next));
    setPosition(current, position(current) + current.linearVelocity * Parameters.PhysicsStep);
    float before = minimumY(boxOf(current));
    rotateMotion(current, Parameters.PhysicsStep);
    float after = minimumY(boxOf(current));
    if(after < 0.0 && before >= 0.0)
    {
        current.linearVelocity.y = max(current.linearVelocity.y, 0.0);
        float3 center = position(current);
        center.y -= after;
        setPosition(current, center);
    }
    if((current.flags & 2u) != 0 || forced) current.lastContact = Parameters.FrameIndex;
    // After 300 frames without a domino contact or forced topple, suppress
    // residual motion. Below both velocity thresholds, restore this substep's
    // incoming transform exactly. Once every substep leaves it unchanged,
    // Simulate stops marking the domino as modified and sparse AS writes cease
    // after any final partition reassignment.
    if(current.lastContact != 0xffffffffu && Parameters.FrameIndex - current.lastContact > 300u)
    {
        current.angularVelocity *= 0.01;
        current.linearVelocity *= 0.01;
        if(length(current.angularVelocity) < 0.02 && length(current.linearVelocity) < 0.02)
        {
            current.angularVelocity = 0;
            current.linearVelocity = 0;
            current.row0 = original0;
            current.row1 = original1;
            current.row2 = original2;
        }
    }
    float damping = 1.0 - Parameters.PhysicsStep;
    if((current.flags & 3u) != 0 || current.linearVelocity.y > 0.0)
        current.linearVelocity *= damping;
    current.angularVelocity *= damping;
    return any(current.row0 != original0) || any(current.row1 != original1)
        || any(current.row2 != original2);
}
