#version 330 core

precision highp float;
precision highp sampler2D;

in vec2 uv;
out vec4 out_color;

// Camera and scene parameters
const vec3 e = vec3(0.0, 0.0, 1.0);
const vec3 t = vec3(0.0, 1.0, 0.0);
const vec3 u = vec3(1.0, 0.0, 0.0);
const vec3 w = normalize(e - t);
const vec3 v = cross(w, u);
const float dof = 1.0;
const mat4 M_view = mat4(vec4(u,0), vec4(v,0), vec4(w, 0), vec4(e, 1));

const vec3 light_pos = vec3(0.0, 1.0, 1.0);

// Sphere parameters
const vec3 sphere_1_pos = vec3(0.0, 0.0, -2.0);
const vec3 sphere_2_pos = vec3(0.0, -101.0, -2.0);
const float sphere_1_radius = 1.0;
const float sphere_2_radius = 100.0;
// const vec3 sphere_1_color = vec3(0.0, 1.0, 0.0);
// const vec3 sphere_2_color = vec3(1.0, 0.0, 0.0);
// const float sphere_1_reflectance = 0.8;
// const float sphere_2_reflectance = 0.2;

// Uniforms
// uniform vec2 u_resolution;
// uniform vec2 u_mouse;
// uniform float u_time;

// Ray-sphere intersection function
bool intersectRaySphere(vec3 rayOrigin, vec3 rayDir, vec3 spherePos, float sphereRadius, out float t)
{
    vec3 oc = rayOrigin - spherePos;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
        return false;

    float sqrtDisc = sqrt(discriminant);
    float t0 = (-b - sqrtDisc) / (2.0 * a);
    float t1 = (-b + sqrtDisc) / (2.0 * a);

    t = (t0 < t1) ? t0 : t1;
    if (t < 0.0) return false;
    return true;
}

vec3 rayTrace(vec3 o, vec3 d)
{
    vec3 color = vec3(0.0);
    // float diffuse = 0.8;

    // Sphere 1
    float t2;
    if (intersectRaySphere(o, d, sphere_2_pos, sphere_2_radius, t2))
    {
        vec3 hit_pos = o + t2*d;
        vec3 normal = normalize(hit_pos - sphere_2_pos);
        // vec3 lightDir = normalize(light_pos - hit_pos);
        // float diffuse = max(0.0, dot(normal, lightDir));
        color = 0.5*(normal + vec3(1,1,1));
    }
    
    float t1;
    if (intersectRaySphere(o, d, sphere_1_pos, sphere_1_radius, t1))
    {
        vec3 hit_pos = o + t1*d;
        vec3 normal = normalize(hit_pos - sphere_1_pos);
        // vec3 lightDir = normalize(light_pos - hit_pos);
        // float diffuse = max(0.0, dot(normal, lightDir));
        color = 0.5*(normal + vec3(1,1,1));
    }

    return color;
}

void main()
{
    // Normalized pixel coordinates
    // vec2 uv = (gl_FragCoord.xy - 0.5 * u_resolution) / u_resolution.y;

    vec2 uv_norm = (2.0*uv - 1.0);

    // Ray direction
    // vec3 ray_dir =  vec3(M_view * vec4(uv_norm, -dof, 1));
    vec3 ray_dir =  vec3(uv_norm, -dof);

    // Trace the ray
    vec3 color = rayTrace(e, ray_dir);

    // Gamma correction
    color = pow(color, vec3(0.4545));


    // Output the color
    out_color = vec4(color, 1.0);
    // out_color = vec4(ray_dir, 1.0);
}
