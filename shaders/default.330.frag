#version 330

uniform float uTime = 0;
uniform vec3 uEyePos = vec3(0,0,5);

uniform vec3 uLightPos  = vec3(-2,2,10);
uniform vec4 uLightAmbt = vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4 uLightDiff = vec4(0.8, 0.8, 0.8, 1.0);
uniform vec4 uLightSpec = vec4(1.0, 1.0, 1.0, 1.0);

uniform sampler2D uDiffMap;
uniform sampler2D uSpecMap;
uniform sampler2D uGlowMap;

in vec3 vVertex;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

out vec4 fColor;

void main () {
    vec3 lightDir = uLightPos - vVertex;
    vec3 eyeDir = uEyePos - vVertex;

    float diffFactor = max(0, dot(vNormal, normalize(lightDir)));
    float specFactor = max(0, dot(vNormal, normalize(eyeDir+lightDir)));

    vec4 diffTexCol = max(vec4(1.0,1.0,1.0,1.0), texture2D(uDiffMap, vTexCoord));
    vec4 specTexCol = max(vec4(1.0,1.0,1.0,0.5), texture2D(uSpecMap, vTexCoord));
    vec4 glowTexCol = max(vec4(0.0,0.0,0.0,1.0), texture2D(uGlowMap, vTexCoord));

    vec4 ambtColor = uLightAmbt * diffTexCol * vColor;
    vec4 diffColor = uLightDiff * diffTexCol * vColor;
    vec4 specColor = uLightSpec * specTexCol;
    float specPower = pow(2.0, 16.0*specTexCol.a);
    vec4 glowColor = glowTexCol * glowTexCol.a;

    fColor  = ambtColor;
    fColor += diffColor * diffFactor;
    fColor += specColor * pow(specFactor, specPower);
    fColor += glowColor;
    fColor.a = vColor.a * diffTexCol.a;

    //~ fColor.rgb = vNormal;
    //~ fColor.rgb = normalize(-vVertex);

    //~ fColor.rgb = fColor.rgb * 0.8 + 0.2 * mod(gl_FragCoord.x*gl_FragCoord.y,2);

    //~ float plasma = sin(vTexCoord.s*10+uTime);
    //~ plasma += sin(10*(vTexCoord.s*cos(uTime/2)+vTexCoord.t*sin(uTime/3))+uTime);
    //~ vec2 v = vec2(vTexCoord.s+0.5*cos(uTime/5),vTexCoord.t+0.5*sin(uTime/3));
    //~ plasma += sin(sqrt(100*dot(v,v)+1+uTime+gl_FragCoord.z));

    //~ plasma = sin(plasma)*0.5+0.5;

    //~ fColor.rgb = sin(plasma*vec3(0.3,0.5,0.2))*0.5+0.5;
    //~ fColor.rgb = vec3(sin(plasma)*0.5+0.5);

    //~ fColor.rgb = vec3(dot(fColor.rgb, vec3(0.3,0.5,0.2)));

    //~ fColor.rgb = fColor.rgb * 0.8 + plasma * 0.2;
}
