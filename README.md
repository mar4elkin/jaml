# JAML - just another math lib
Educational project: a tiny header-only 2D vector math helper (vector2.h) and a minimal Win32 GDI viewer (viewer_win32.c) to visualize vectors, coordinate axes, and small demos (presets).

## Examples
### Base vectors
```c
vec2 a = (vec2){ 3.0f, 4.0f };
vec2 b = (vec2){ -1.5f, 2.0f };
```

### add / sub / mul
```c
vec2 add = vec2_add(&a, &b);                // (4.500, 6.000)
vec2 sub = vec2_sub(&a, &b);                // (4.500, 2.000)
vec2 mul = vec2_mul(&a, 2.0f);              // (6.000, 8.000)
```

### length2 / length
```c
vec2_length2(&a); // 25.000
vec2_length(&a);  // 5.000
```

### dist2 / dist
```c
vec2_dist2(&a, &b); // 24.250
vec2_dist(&a, &b);  // ~4.924
```

### normalize
```c
vec2_normalize(&a); // (0.600, 0.800)
```

### dot / cross
```c
vec2_dot(&a, &b); // 3.500
vec2_cross(&a, &b); // 12.000
```

### angle (radians / degrees)
```c
float ang = vec2_angle(&a, &b);                 // ~1.287003 rad
float deg = ang * (180.0f / 3.14159265358979f); // ~73.740°
```

### equal (tolerant compare with EPSILON)
```c
vec2 a_close = (vec2){ a.x + 1e-7f, a.y };
vec2_equal(&a, &a, EPSILON);       // true
vec2_equal(&a, &a_close, EPSILON); // true
```

### min / max (component-wise)
```c
vec2 vmin = vec2_min(&a, &b); // (-1.500, 2.000)
vec2 vmax = vec2_max(&a, &b); // ( 3.000, 4.000)
```

### abs (component-wise)
```c
vec2 v  = (vec2){ -2.0f, 5.0f };
vec2 av = vec2_abs(&v); // (2.000, 5.000)
```

### perp (90° CCW)
```c
vec2 p = vec2_perp(&a); // (-4.000, 3.000)
```

### project / reject
```c
vec2 proj = vec2_project(&a, &b); // (-0.840, 1.120)
vec2 rej  = vec2_reject(&a, &b);  // ( 3.840, 2.880)
```

### reflect (incident about normal)
```c
vec2 i = (vec2){ 3.0f, -2.0f };
vec2 nrm = (vec2){ 0.0f, 1.0f };
vec2 r = vec2_reflect(&i, &nrm); // (3.000, 2.000)
```

### rotate (around origin)
```c
vec2 v0 = (vec2){ 2.0f, 0.0f };
float rad = 3.14159265358979f / 4.0f; // 45°
vec2 vr = vec2_rotate(&v0, rad);      // (~1.414, ~1.414)
```

### rotate_around (about pivot)
```c
vec2 pt    = (vec2){ 2.0f, 1.0f };
vec2 pivot = (vec2){ 1.0f, 0.0f };
float r90  = 3.14159265358979f / 2.0f; // 90°
vec2 out   = vec2_rotate_around(&pt, &pivot, r90); // (0.000, 1.000)
```

### rot90 helpers
```c
vec2 u = (vec2){ 5.0f, 2.0f };
vec2 u_ccw = vec2_rot90_ccw(&u); // (-2.000, 5.000)
vec2 u_cw  = vec2_rot90_cw(&u);  // ( 2.000,-5.000)
```

## Visualizations
<img width="990" height="793" alt="empty" src="https://github.com/user-attachments/assets/14ca4798-55b4-416d-8726-009ac9db6263" />
preset empty

<img width="990" height="793" alt="rotations" src="https://github.com/user-attachments/assets/4d54269c-b605-4099-84de-7b63a8e23ce0" />
preset rotations

<img width="990" height="793" alt="reflections" src="https://github.com/user-attachments/assets/14d353bc-619b-4034-9cc1-72f2c11d85dd" />
preset reflections

<img width="990" height="793" alt="projection" src="https://github.com/user-attachments/assets/9ac5cab4-75a2-40ec-ba61-7a2f05032c51" />
preset projection

<img width="990" height="793" alt="basics" src="https://github.com/user-attachments/assets/f14f489c-972d-42c6-b4cf-e58c63634ca9" />
preset basics

## Types & Constants
- vec2 — { float x, y; }.
- EPSILON — small positive constant used for tolerance in comparisons.

## Basic Ops (component-wise)
- vec2 vec2_add(vec2* a, vec2* b) → (a.x+b.x, a.y+b.y)
- vec2 vec2_sub(vec2* a, vec2* b) → (a.x-b.x, a.y-b.y)
- vec2 vec2_mul(vec2* a, float s) → (a.x*s, a.y*s)

## Length & Distance
- float vec2_length2(vec2* a) → x² + y² (no sqrt, fast)
- float vec2_length(vec2* a) → sqrt(x² + y²)
- float vec2_dist2(vec2* a, vec2* b) → (ax-bx)² + (ay-by)²
- float vec2_dist(vec2* a, vec2* b) → sqrt(dist2)

## Normalization
- vec2 vec2_normalize(vec2* a) → unit vector a/|a|; returns (0,0) if |a|==0.

## Products
- float vec2_dot(vec2* a, vec2* b) → ax*bx + ay*by
- float vec2_cross(vec2* a, vec2* b) → ax*by - ay*bx

## Angle
- float vec2_angle(vec2* a, vec2* b)

## Equality with Tolerance
- bool vec2_equal(vec2* a, vec2* b, float eps)

## Component-wise Helpers
- vec2 vec2_min(vec2* a, vec2* b) → (min(ax,bx), min(ay,by))
- vec2 vec2_max(vec2* a, vec2* b) → (max(ax,bx), max(ay,by))
- vec2 vec2_abs(vec2* a) → (|ax|, |ay|)
- vec2 vec2_perp(vec2* a) → 90° CCW perpendicular (-y, x)

## Projection / Rejection / Reflection
- vec2 vec2_project(vec2* a, vec2* b)
- vec2 vec2_reject(vec2* a, vec2* b) → a - proj_b(a)
- vec2 vec2_reflect(vec2* a, vec2* n)

## Rotation
- vec2 vec2_rotate(vec2* a, float radians)
- vec2 vec2_rotate_around(const vec2* v, const vec2* pivot, float radians)
- vec2 vec2_rot90_ccw(const vec2* v) → +90° (-y, x)
- vec2 vec2_rot90_cw(const vec2* v) → −90° (y, -x)
