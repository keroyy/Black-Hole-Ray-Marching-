#version 330 core
#define Max_Steps 300    // �����
#define Max_Dist 100.	 // ������
#define Surf_Dist 0.01   //

in vec2 screenCoord;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 rotate;
uniform samplerCube skybox;
uniform float time;

// ����
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
    mat4 rotate = mat4(  // ��������ת����
        c+pow(a.x, 2)*p, a.x*a.y*p-a.z*s, a.x*a.z*p+a.y*s, 0,
        a.y*a.x*p+a.z*s, c+pow(a.y,2)*p, a.y*a.z*p-a.x*s, 0,
        a.z*a.x*p-a.y*s, a.z*a.y*p+a.x*s ,c+pow(a.z,2)*p, 0,
        0, 0, 0, 1
    );

    return vec3(rotate * v1);
}

// �¼��ӽ� 
// Signed Distance Field��������Ϊ������볡��SDF����������һ��ͼ�ε���������ϰ���Ե��������Ĺ����ǵ���ͼ���ڲ��򷵻ظ�ֵ������ͼ���ⲿ������ֵ
float eventHorizon(vec3 pos)
{
    return length(pos) - 1; // ʷ�����뾶
}

// ʩ�����ȹ� �����˶��켣����(ʷ�����뾶Ϊ1�������)
vec3 gravitationalLensing(float H2, vec3 pos)
{
    float r2 = dot(pos, pos);
    float r5 = pow(r2, 2.5);
    return -1.5 * H2 * pos / r5;
}

// ������
vec3 accrectionDisk(vec3 pos)
{
    const float MIN_WIDTH = 2.6; // ��������͸��ЧӦ����ɫ��Ե��ʵ���¼��ӽ��һ��ͶӰ�����İ뾶�պ���2.6����ʷ�����뾶

    float r = length(pos);

    float accretionDiskWidth = 11;

    vec3 disk = vec3(accretionDiskWidth, 0.2, accretionDiskWidth); // ����һ���ܱ��������
    if (length(pos / disk) > 1)
    {
        return vec3(0, 0, 0);
    }
    float temperature = max(0, 1 - length(pos / disk));
    temperature *= (r - MIN_WIDTH) / (accretionDiskWidth - MIN_WIDTH);
    // ����ת��Ϊ������ϵ
    float t = radians(atan(pos.z, pos.x)); // ��
    float p = asin(pos.y / r); // ��
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

    vec3 h = cross(pos, dir); // �������
    float h2 = dot(h, h);

    for(int i = 0; i < Max_Steps; i++)
    {
        //pos = GetRayLocation(ray, t);

        //ray.origin = pos;

        // �¼��ӽ�
        if (eventHorizon(pos) < 0.0)
        {
            return color;
        }

        // ������
        color += accrectionDisk(pos);

        // ����͸��
        vec3 offset = gravitationalLensing(h2, pos);
        dir += offset;

        // ����
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