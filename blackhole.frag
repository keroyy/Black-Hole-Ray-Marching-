#version 330 core
#define Max_Steps 300    // 最大步数

in vec2 screenCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 rotate;
uniform samplerCube skybox;
uniform sampler2D colorMap;
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

// Convert from Cartesian to spherical coord (rho, phi, theta) 笛卡尔坐标系 -> 球面坐标系
// https://en.wikipedia.org/wiki/Spherical_coordinate_system
vec3 toSpherical(vec3 p) {
  float rho = sqrt((p.x * p.x) + (p.y * p.y) + (p.z * p.z));
  float theta = atan(p.z, p.x);
  float phi = asin(p.y / rho);
  return vec3(rho, theta, phi);
}

///----
/// Simplex 3D Noise
/// by Ian McEwan, Ashima Arts
vec4 permute(vec4 x) { return mod(((x * 34.0) + 1.0) * x, 289.0); }
vec4 taylorInvSqrt(vec4 r) { return 1.79284291400159 - 0.85373472095314 * r; }

float snoise(vec3 v) {
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  // x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

  // Permutations
  i = mod(i, 289.0);
  vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y +
                           vec4(0.0, i1.y, i2.y, 1.0)) +
                   i.x + vec4(0.0, i1.x, i2.x, 1.0));

  // Gradients
  // ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0 / 7.0; // N=7
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_); // mod(j,N)

  vec4 x = x_ * ns.x + ns.yyyy;
  vec4 y = y_ * ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4(x.xy, y.xy);
  vec4 b1 = vec4(x.zw, y.zw);

  vec4 s0 = floor(b0) * 2.0 + 1.0;
  vec4 s1 = floor(b1) * 2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
  vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

  vec3 p0 = vec3(a0.xy, h.x);
  vec3 p1 = vec3(a0.zw, h.y);
  vec3 p2 = vec3(a1.xy, h.z);
  vec3 p3 = vec3(a1.zw, h.w);

  // Normalise gradients
  vec4 norm =
      taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // Mix final noise value
  vec4 m =
      max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
  m = m * m;
  return 42.0 *
         dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
///----

// 吸积盘
void accrectionDisk(vec3 pos, inout vec3 color)
{
    float innerRadius = 2.6; // 由于引力透镜效应，黑色边缘其实是事件视界的一个投影，它的半径刚好是2.6倍的史瓦西半径
    float outerRadius = 11.0;

    float adiskParticle = 1.0; // 1.0
    float adiskHeight = 0.5; // 0.2
    float adiskLit = 0.1; // 0.01
    float adiskDensityV = 2.0; // 1.0
    float adiskDensityH = 3.0; // 1.0
    float adiskNoiseScale = 0.8; // 1.0
    float adiskNoiseLOD = 5.0; // 5.0
    float adiskSpeed = 10;

    vec3 disk = vec3(outerRadius, adiskHeight, outerRadius); // 视作一个很扁的椭球形

    // Density linearly decreases as the distance to the blackhole center
    // 密度随着到黑洞中心的距离线性降低
    // increases.
    float density = max(0.0, 1.0 - length(pos / disk));
    if (density < 0.001) {
        return;
    }

    density *= pow(1.0 - abs(pos.y) / adiskHeight, adiskDensityV);

    // cSet particale density to 0 when radius is below the inner most stable
    // 当半径低于内部最稳定时，将粒子密度设置为0
    // circular orbit.
    density *= smoothstep(innerRadius, innerRadius * 1.1, length(pos));

    // Avoid the shader computation when density is very small.
    // 当密度非常小时，避免着色器计算
    if (density < 0.001) {
        return;
    }
    
    vec3 sphericalCoord = toSpherical(pos);

    // Scale the theta and phi so that the particales appear to be at the correct
    // 缩放 θ 和 φ ，使粒子看起来在正确的位置
    // scale visually.
    sphericalCoord.y *= 2.0;
    sphericalCoord.z *= 4.0;

    density *= 1.0 / pow(sphericalCoord.x, adiskDensityH);
    density *= 16000.0;

    if (adiskParticle < 0.5) {
        color += vec3(0.0, 1.0, 0.0) * density * 0.02;
        return;
    }

    float noise = 1.0;
    for (int i = 0; i < int(adiskNoiseLOD); i++) {
        noise *= 0.5 * snoise(sphericalCoord * pow(i, 2) * adiskNoiseScale) + 0.5;
        if (i % 2 == 0) {
            sphericalCoord.y += time * adiskSpeed;
        } else {
            sphericalCoord.y -= time * adiskSpeed;
        }
    }

    vec3 dustColor = texture(colorMap, vec2(sphericalCoord.x / outerRadius, 0.5)).rgb;

    color += density * adiskLit * dustColor * abs(noise);
}

// 光线步进
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
        
        // 事件视界
        if (eventHorizon(pos) < 0.0)
        {
            return color;
        }

        // 吸积盘
        accrectionDisk(pos, color);

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