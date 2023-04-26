#version 330 core
out vec4 FragColor;

in vec2 screenCoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
//uniform float exposure;
uniform float gamma = 2.2;
uniform float tone = 1.0;
uniform float bloomStrength = 0.1;

///----
/// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
vec3 aces(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{             
    vec3 hdrColor = texture(scene, screenCoord).rgb; 
    vec3 bloomColor = texture(bloomBlur, screenCoord).rgb;
    if(bloom)
        hdrColor = hdrColor * tone + bloomColor * bloomStrength; // additive blending

    // tone mapping
    //float exposure = 0.6;
    //vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    
    // ACES filmic tone mapping
    vec3 result = aces(hdrColor);

    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}