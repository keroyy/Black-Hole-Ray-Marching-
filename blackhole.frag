#version 330 core
#define Max_Steps 300    // 最大步数
#define Max_Dist 100.	 // 最大距离
#define Surf_Dist 0.01   //

in vec2 screenCoord;

out vec4 FragColor;

struct Camera {// 摄像机
    vec3 lower_left_corner; // 左下角
    vec3 horizontal; // 水平
    vec3 vertical; // 垂直
    vec3 origin; 
};
uniform Camera camera;
uniform mat4 view;
uniform mat4 rotate;
uniform samplerCube skybox;
uniform float time;

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

vec3 rotateVec3(vec3 v, vec3 axis, float theta){
    vec4 v1 = vec4(v, 1.0);
    float c = cos(theta),
        s = sin(theta), 
        p = 1 - cos(theta);
    vec3 a = axis;
    mat4 rotate = mat4(  // 任意轴旋转矩阵
        c+pow(a.x, 2)*p, a.x*a.y*p-a.z*s, a.x*a.z*p+a.y*s, 0,
        a.y*a.x*p+a.z*s, c+pow(a.y,2)*p, a.y*a.z*p-a.x*s, 0,
        a.z*a.x*p-a.y*s, a.z*a.y*p+a.x*s ,c+pow(a.z,2)*p, 0,
        0, 0, 0, 1
    );

    return vec3(rotate * v1);
}

float GetDist(vec3 p)
{
    Sphere s = CreateSphere(vec3(0, 0, -6), 0.2);

    float d = length(p-s.center)-s.radius;// P点到球面的距离
    
    return d;
}

// Signed Distance Field，中文名为有向距离场。SDF函数描述了一个图形的区域，我们习惯性地设置它的规则是点在图形内部则返回负值，点在图形外部返回正值
float eventHorizon(vec3 pPosition)
{
    return length(pPosition) - 1;
}

// 施瓦西度规 粒子运动轨迹计算
vec3 gravitationalLensing(float H2, vec3 pPosition)
{
    float r2 = dot(pPosition, pPosition);
    float r5 = pow(r2, 2.5);
    return -1.5 * H2 * pPosition / r5;
}

// TODO:test
vec3 RayMarching(Ray ray)
{
    vec3 color = vec3(0.);
    float t = 0.1;
    vec3 pos = vec3(0.);
    vec3 dir = ray.direction;
    vec3 h = cross(pos, dir);
    float h2 = dot(h, h);

    vec3 worldPos = vec3(inverse(view) * vec4(pos, 1.0));
    vec3 worldDir = vec3(inverse(view) * vec4(dir, 1.0));

    for(int i = 0; i < Max_Steps; i++)
    {
        //pos = GetRayLocation(ray, t);

        //ray.origin = pos;

        // 事件视界
        if (eventHorizon(worldPos) < 0.0)
        {
            return color;
        }

        // 引力透镜
        vec3 offset = gravitationalLensing(h2, worldPos);
        worldDir += offset;

        // 步进
        worldPos += worldDir;
    }
    // sample skybox
    //vec3 worldDir = vec3(inverse(view) * vec4(dir, 1.0));
    vec3 normalizeDir = normalize(worldDir.xyz);
    normalizeDir = rotateVec3(normalizeDir, vec3(0, 1, 0), time);
    color = vec3(texture(skybox, normalizeDir));
                     
    return color;    
}

vec3 RayMarch(Ray ray){
    vec3 color = vec3(0.);
    float d0 = 0.;
    for(int i = 0; i < Max_Steps; i++)
    {
        vec3 p = ray.origin + ray.direction * d0;
        float ds = GetDist(p);
        d0 += ds;
        if(d0 > Max_Dist) {
            // sample skybox
            vec3 worldDir = vec3(inverse(view) * vec4(ray.direction, 1.0));
            vec3 normalizeDir = normalize(worldDir.xyz);
            normalizeDir = rotateVec3(normalizeDir, vec3(0, 1, 0), time);
            color = vec3(texture(skybox, normalizeDir));
            break;
        } 
        if(d0 < Surf_Dist) {
            break;
        }        
    }
                     
    return color;     
}

vec3 RayTrace(Ray ray){
    vec3 color = vec3(0.0);
    float alpha = 1.0;
     
    Sphere s = CreateSphere(vec3(0, 0, -6), 0.1);
    if (SphereHit(s, ray)){
        return color;
    }
    
    // skybox color
    vec3 worldDir = vec3(inverse(view) * vec4(ray.direction, 1.0));
    vec3 normalizeDir = normalize(worldDir.xyz);
    normalizeDir = rotateVec3(normalizeDir, vec3(0, 1, 0), time);
    color = vec3(texture(skybox, normalizeDir));
    return color;
}

void main(){
    float u = screenCoord.x;
    float v = screenCoord.y;

    //vec3 dir = normalize(vec3(camera.lower_left_corner + u * camera.horizontal + v * camera.vertical - camera.origin));
    //dir = vec3(inverse(view) * vec4(dir, 1.0));
    //vec3 camPos = vec3(inverse(view) * vec4(camera.origin, 1.0));// world space

    Ray ray = CreateRay(camera.origin, camera.lower_left_corner + u * camera.horizontal + v * camera.vertical - camera.origin);
    //FragColor = vec4(RayTrace(ray), 1.0);
    FragColor = vec4(RayMarching(ray), 1.0);
}