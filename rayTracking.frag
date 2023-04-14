#version 330 core

in vec2 screenCoord;

out vec4 FragColor;

uniform samplerCube skybox;
uniform float time;
uniform float aspect;

// 光线
struct Ray{
    vec3 origin;
    vec3 direction;
};

Ray CreateRay(vec3 o, vec3 d){
    Ray ray;
    ray.origin = o;
    ray.direction = d;

    return ray;
};

vec3 GetRayLocation(Ray ray, float t){
    return ray.origin + t * ray.direction;
};

// 摄像机
struct Camera {
    vec3 lower_left_corner; // 左下角
    vec3 horizontal; // 水平
    vec3 vertical; // 垂直
    vec3 origin; 
};

Camera InitCamera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect){
    Camera camera;
    camera.origin = lookfrom;

    float theta = radians(vfov);
    float half_height = tan(theta/2);
    float half_width = aspect * half_height;

    vec3 u, v, w;
    w = normalize(lookfrom - lookat);
    u = normalize(cross(vup, w));
    v = cross(w, u);

    camera.lower_left_corner = camera.origin - half_width*u - half_height*v - w;

    camera.horizontal = 2 * half_height * u;
    camera.vertical = 2 * half_width * v;
    return camera;
};

// 球体
struct Sphere{
    vec3 center;
    float radius;
}; 

Sphere CreateSphere(vec3 center, float radius){
	Sphere sphere;

	sphere.center = center;
	sphere.radius = radius;

	return sphere;
}

bool SphereHit(Sphere sphere, Ray ray){
	vec3 oc = ray.origin - sphere.center;
	
	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(oc, ray.direction);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;

	float delta = b * b - 4 * a * c;

	return delta > 0.0;
}

vec4 quadFromAxisAngle(vec3 axis, float angle) {
    vec4 qr;
    float half_angle = (angle * 0.5) * 3.14159 / 180.0;
    qr.x = axis.x * sin(half_angle);
    qr.y = axis.y * sin(half_angle);
    qr.z = axis.z * sin(half_angle);
    qr.w = cos(half_angle);
    return qr;
}

vec4 quadConj(vec4 q) { 
    return vec4(-q.x, -q.y, -q.z, q.w); 
}

vec4 quat_mult(vec4 q1, vec4 q2) {
    vec4 qr;
    qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
    qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
    qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
    qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
    return qr;
}

vec3 rotateVector(vec3 position, vec3 axis, float angle) {
    vec4 qr = quadFromAxisAngle(axis, angle);
    vec4 qr_conj = quadConj(qr);
    vec4 q_pos = vec4(position.x, position.y, position.z, 0);

    vec4 q_tmp = quat_mult(qr, q_pos);
    qr = quat_mult(q_tmp, qr_conj);

    return vec3(qr.x, qr.y, qr.z);
}

mat3 lookAt(vec3 origin, vec3 target, float roll) {
  vec3 rr = vec3(sin(roll), cos(roll), 0.0);
  vec3 ww = normalize(target - origin);
  vec3 uu = normalize(cross(ww, rr));
  vec3 vv = normalize(cross(uu, ww));

  return mat3(uu, vv, ww);
}

mat4 LookAt(vec3 eye, vec3 up)
{
    mat4 t2 = mat4(1,0,0,0,0,1,0,0,0,0,1,0,-eye.x,-eye.y,-eye.z,1);
    vec3 w = normalize(eye);
    vec3 u = normalize(cross(up,eye));
    vec3 v = cross(w,u);
    mat4 t1 = mat4(u.x,v.x,w.x,0,u.y,v.y,w.y,0,u.z,v.z,w.z,0,0,0,0,1);
    return t1*t2;
}

vec3 RayTrace(Ray ray){
    vec3 color = vec3(1.0);
    float alpha = 1.0;
     
    Sphere s = CreateSphere(vec3(0, 0, -1), 0.1);
    if (SphereHit(s, ray)){
        return color;
    }

    mat4 view = LookAt(vec3(0,0,6), vec3(0,0,0));
    
    // skybox color
    vec3 normalizeDir = normalize(ray.direction);
    //vec3 Dir = rotateVector(normalizeDir, vec3(0.0, 1.0, 0.0), time);
    vec4 Dir = inverse(view) * vec4(normalizeDir.xyz,1);
    Dir = normalize(Dir);
    color = vec3(texture(skybox, normalizeDir.xyz));
    return color;
}

void main(){
    float u = screenCoord.x;
    float v = screenCoord.y;

    Camera camera = InitCamera(vec3(0,0,0), vec3(0,0,-1), vec3(0,1,0), 45, aspect);

    Ray ray = CreateRay(camera.origin, camera.lower_left_corner + u * camera.horizontal + v * camera.vertical - camera.origin);

    FragColor = vec4(RayTrace(ray), 1.0);
}