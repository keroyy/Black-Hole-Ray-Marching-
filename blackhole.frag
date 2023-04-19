#version 330 core
#define Max_Steps 300    // 最大步数
#define Max_Dist 100.	 // 最大距离
#define Surf_Dist 0.01   //

in vec2 screenCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform mat4 view;
uniform mat4 projection;
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

// 事件视界 
// Signed Distance Field，中文名为有向距离场。SDF函数描述了一个图形的区域，我们习惯性地设置它的规则是点在图形内部则返回负值，点在图形外部返回正值
float eventHorizon(vec3 pos)
{
    return length(pos) - 1; // 史瓦西半径
}

// 施瓦西度规 粒子运动轨迹计算(史瓦西半径为1的情况下)
vec3 gravitationalLensing(float H2, vec3 pos)
{
    float r2 = dot(pos, pos);
    float r5 = pow(r2, 2.5);
    return -1.5 * H2 * pos / r5;
}

// 吸积盘
vec3 accrectionDisk(vec3 pos)
{
    const float MIN_WIDTH = 2.6; // 由于引力透镜效应，黑色边缘其实是事件视界的一个投影，它的半径刚好是2.6倍的史瓦西半径

    float r = length(pos);

    float accretionDiskWidth = 11;

    vec3 disk = vec3(accretionDiskWidth, 0.2, accretionDiskWidth); // 视作一个很扁的椭球形
    if (length(pos / disk) > 1)
    {
        return vec3(0, 0, 0);
    }
    float temperature = max(0, 1 - length(pos / disk));
    temperature *= (r - MIN_WIDTH) / (accretionDiskWidth - MIN_WIDTH);
    // 坐标转换为球极坐标系
    float t = radians(atan(pos.z, pos.x)); // θ
    float p = asin(pos.y / r); // φ
    vec3 sphericalCoord = vec3(r, t, p);

    vec3 color = vec3(0, 1, 0);
    return temperature * color;

}

// TODO:test
vec3 RayMarching(Ray ray)
{
    vec3 color = vec3(0.);
    float STEP_SIZE = 0.1;
    vec3 pos = ray.origin;
    vec3 dir = ray.direction * STEP_SIZE;

    vec3 h = cross(pos, dir); // 面积常数
    float h2 = dot(h, h);

    for(int i = 0; i < Max_Steps; i++)
    {
        //pos = GetRayLocation(ray, t);

        //ray.origin = pos;

        // 事件视界
        if (eventHorizon(pos) < 0.0)
        {
            return color;
        }

        // 吸积盘
        color += accrectionDisk(pos);

        // 引力透镜
        vec3 offset = gravitationalLensing(h2, pos);
        dir += offset;

        // 步进
        pos += dir;
    }
    // sample skybox
    dir = rotateVec3(dir, vec3(0, 1, 0), time);
    color += vec3(texture(skybox, dir));
                     
    return color;    
}

void main(){
    float u = screenCoord.x * 2.0f - 1.0f;
    float v = screenCoord.y * 2.0f - 1.0f;

    vec3 dir = normalize(vec3(inverse(view) * inverse(projection) * vec4(u, v, 0, 1)));

    Ray ray = CreateRay(cameraPos, dir);

    FragColor = vec4(RayMarching(ray), 1.0);
}