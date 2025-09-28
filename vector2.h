//
// Created by markt on 28.09.2025
//
// vector2.h — small header-only 2D vector math utilities for games.
//

#ifndef VECTOR2_H
#define VECTOR2_H

#define EPSILON 1e-6f

#include <math.h>
#include <stdbool.h>

typedef struct {
    float x, y;
} vec2;

/**
 * @brief Component-wise addition of two vectors.
 *
 * @param a Pointer to the first vector (read-only).
 * @param b Pointer to the second vector (read-only).
 * @return Sum vector (a + b).
 */
static inline vec2 vec2_add(vec2* a, vec2* b)
{
    return (vec2){a->x + b->x, a->y + b->y};
}

/**
 * @brief Component-wise subtraction of two vectors.
 *
 * @param a Pointer to the minuend vector (read-only).
 * @param b Pointer to the subtrahend vector (read-only).
 * @return Difference vector (a - b).
 */
static inline vec2 vec2_sub(vec2* a, vec2* b)
{
    return (vec2){a->x - b->x, a->y - b->y};
}

/**
 * @brief Multiply a vector by a scalar.
 *
 * @param a Pointer to the input vector (read-only).
 * @param t Scalar multiplier.
 * @return Scaled vector (a * t).
 */
static inline vec2 vec2_mul(vec2* a, float t)
{
    return (vec2){ a->x * t, a->y * t };
}

/**
 * @brief Squared length (magnitude) of a vector.
 *        Avoids sqrt for performance-critical code.
 *
 * @param a Pointer to the input vector (read-only).
 * @return Squared length, i.e. x*x + y*y.
 */
static inline float vec2_length2(vec2* a)
{
    return a->x * a->x + a->y * a->y;
}

/**
 * @brief Euclidean length (magnitude) of a vector.
 *
 * @param a Pointer to the input vector (read-only).
 * @return Length, i.e. sqrt(x*x + y*y).
 */
static inline float vec2_length(vec2* a)
{
    return sqrt(vec2_length2(a));
}

/**
 * @brief Squared distance between two points (vectors).
 *        Useful to compare distances without sqrt.
 *
 * @param a Pointer to the first point (read-only).
 * @param b Pointer to the second point (read-only).
 * @return Squared distance.
 */
static inline float vec2_dist2(vec2* a, vec2* b)
{
    const float diff_x = a->x - b->x;
    const float diff_y = a->y - b->y;
    return diff_x * diff_x + diff_y * diff_y;
}

/**
 * @brief Euclidean distance between two points (vectors).
 *
 * @param a Pointer to the first point (read-only).
 * @param b Pointer to the second point (read-only).
 * @return Distance.
 */
static inline float vec2_dist(vec2* a, vec2* b)
{
    return sqrt(vec2_dist2(a, b));
}

/**
 * @brief Return a normalized (unit-length) copy of a vector.
 *
 * @param a Pointer to the input vector (read-only).
 * @return Unit vector in the direction of a.
 * @note If |a| == 0, returns (0,0) to avoid division by zero.
 */
static inline vec2 vec2_normalize(vec2* a)
{
    const float len = vec2_length(a);
    if (len == 0.0f) return (vec2){0.0f, 0.0f};
    return (vec2){a->x / len, a->y / len};
}

/**
 * @brief Dot product of two vectors.
 *
 * @param a Pointer to the first vector (read-only).
 * @param b Pointer to the second vector (read-only).
 * @return Scalar dot product (a.x*b.x + a.y*b.y).
 */
static inline float vec2_dot(vec2* a, vec2* b)
{
    return a->x * b->x + a->y * b->y;
}

/**
 * @brief 2D cross product (returns the z-component of a 3D cross).
 *
 * @param a Pointer to the first vector (read-only).
 * @param b Pointer to the second vector (read-only).
 * @return Pseudoscalar value a.x*b.y - a.y*b.x.
 */
static inline float vec2_cross(vec2* a, vec2* b)
{
    return a->x * b->y - a->y * b->x;
}

/**
 * @brief Unsigned angle between two vectors, in radians.
 *
 * @param a Pointer to the first vector (read-only).
 * @param b Pointer to the second vector (read-only).
 * @return Angle in [0, π]. Returns 0 if any vector has zero length.
 */
static inline float vec2_angle(vec2* a, vec2* b)
{
    const float dot = vec2_dot(a, b);
    // if (dot == 0.0f) return 0.0f; // orthogonal vectors should produce ~π/2

    const float len_a = vec2_length(a);
    const float len_b = vec2_length(b);

    if (len_a == 0.0f || len_b == 0.0f) return 0.0f;
    return acos(dot / (len_a * len_b));
}

/**
 * @brief Compare two vectors with tolerance.
 *
 * Uses a mixed strategy:
 *   - If both vectors are near zero (|v| < eps), they are considered equal.
 *   - Otherwise, compares each component with a relative tolerance.
 *
 * @param a   Pointer to the first vector (read-only).
 * @param b   Pointer to the second vector (read-only).
 * @param eps Tolerance (typically EPSILON).
 * @return true if considered equal under the above policy.
 */
static inline bool vec2_equal(vec2* a, vec2* b, const float eps)
{
    if (vec2_length(a) < eps && vec2_length(b) < eps) {
        return true;
    }

    return fabsf(a->x - b->x) <= eps * fmaxf(fabsf(a->x), fabsf(b->x)) &&
           fabsf(a->y - b->y) <= eps * fmaxf(fabsf(a->y), fabsf(b->y));
}

/**
 * @brief Component-wise minimum of two vectors.
 *
 * @param a Pointer to the first vector (read-only).
 * @param b Pointer to the second vector (read-only).
 * @return Vector composed of min(a.x,b.x), min(a.y,b.y).
 */
static inline vec2 vec2_min(vec2* a, vec2* b)
{
    return (vec2){
        (a->x < b->x) ? a->x : b->x,
        (a->y < b->y) ? a->y : b->y
    };
}

/**
 * @brief Component-wise maximum of two vectors.
 *
 * @param a Pointer to the first vector (read-only).
 * @param b Pointer to the second vector (read-only).
 * @return Vector composed of max(a.x,b.x), max(a.y,b.y).
 */
static inline vec2 vec2_max(vec2* a, vec2* b)
{
    return (vec2){
        (a->x > b->x) ? a->x : b->x,
        (a->y > b->y) ? a->y : b->y
    };
}

/**
 * @brief Component-wise absolute value.
 *
 * @param a Pointer to the input vector (read-only).
 * @return Vector with |x| and |y|.
 * @note For floating-point inputs, use fabsf for each component.
 */
static inline vec2 vec2_abs(vec2* a)
{
    return (vec2){
        abs(a->x),
        abs(a->y)
    };
}

/**
 * @brief 90° CCW perpendicular vector.
 *
 * @param a Pointer to the input vector (read-only).
 * @return (-y, x).
 */
static inline vec2 vec2_perp(vec2* a)
{
    return (vec2){-a->y, a->x};
}

/**
 * @brief Projection of vector a onto vector b.
 *
 * Computes proj_b(a) = ((a·b)/|b|^2) * b.
 *
 * @param a      Pointer to the input vector (read-only).
 * @param onto_b Pointer to the vector to project onto (read-only).
 * @return Projection vector.
 */
static inline vec2 vec2_project(vec2* a, vec2* onto_b)
{
    const float scalar = vec2_dot(a, onto_b) / vec2_length2(onto_b);
    return (vec2){onto_b->x * scalar, onto_b->y * scalar};
}

/**
 * @brief Rejection of vector a from vector b (component orthogonal to b).
 *
 * Computes reject_b(a) = a - proj_b(a).
 *
 * @param a      Pointer to the input vector (read-only).
 * @param from_b Pointer to the reference vector b (read-only).
 * @return Orthogonal component of a with respect to b.
 */
static inline vec2 vec2_reject(vec2* a, vec2* from_b)
{
    const vec2 projection = vec2_project(a, from_b);
    return (vec2){a->x - projection.x, a->y - projection.y};
}

/**
 * @brief Reflection of vector a about normal n.
 *
 * Computes a' = a - 2 * (a·n̂) * n̂, where n̂ is the normalized n.
 *
 * @param a Pointer to the incident vector (read-only).
 * @param n Pointer to the surface normal (read-only; will be normalized internally).
 * @return Reflected vector.
 * @note If n is zero-length, result equals a.
 */
static inline vec2 vec2_reflect(vec2* a, vec2* n)
{
    vec2 safe_n = vec2_normalize(n);
    const float dot = vec2_dot(a, &safe_n);
    return (vec2){
        a->x - 2.0f * dot * safe_n.x,
        a->y - 2.0f * dot * safe_n.y
    };
}

/**
 * @brief Rotate a vector about the origin by a given angle in radians.
 *
 * @param a       Pointer to the input vector (read-only).
 * @param radians Rotation angle in radians (CCW-positive).
 * @return Rotated vector.
 */
static inline vec2 vec2_rotate(vec2* a, float radians)
{
    const float cos_radians = cosf(radians);
    const float sin_radians = sinf(radians);
    return (vec2){
        a->x * cos_radians - a->y * sin_radians,
        a->x * sin_radians + a->y * cos_radians
    };
}

/**
 * @brief Rotate a point around an arbitrary pivot by a given angle (radians).
 *
 * @param v       Pointer to the point to rotate (read-only).
 * @param pivot   Pointer to the pivot point (read-only).
 * @param radians Rotation angle in radians (CCW-positive).
 * @return Rotated point.
 */
static inline vec2 vec2_rotate_around(const vec2* v, const vec2* pivot, float radians)
{
    vec2 dv = (vec2){ v->x - pivot->x, v->y - pivot->y };
    const vec2 r  = vec2_rotate(&dv, radians);
    return (vec2){ r.x + pivot->x, r.y + pivot->y };
}

/**
 * @brief 90° counter-clockwise rotation.
 *
 * @param v Pointer to the input vector (read-only).
 * @return Vector rotated by +90°: (-y, x).
 */
static inline vec2 vec2_rot90_ccw(const vec2* v)
{
    return (vec2){ -v->y, v->x };
}

/**
 * @brief 90° clockwise rotation.
 *
 * @param v Pointer to the input vector (read-only).
 * @return Vector rotated by -90°: (y, -x).
 */
static inline vec2 vec2_rot90_cw(const vec2* v)
{
    return (vec2){  v->y, -v->x };
}

#endif // VECTOR2_H
