////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __LIGHTINFO_HPP__
#define __LIGHTINFO_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>

////////////////////////////////////////////////////////////////////////////////

struct LightInfo {
    enum LightType {
        Point,
        Spot,
        Directional,
    };

    LightType type;
    glm::vec4 ambtColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 position;
    glm::vec3 halfVector;
    glm::vec3 spotDirection;
    float spotExponent;
    float spotConeInner;
    float spotConeOuter;
    float spotConeInnerCos;
    float spotConeOuterCos;
    float attenuation[3];
};

////////////////////////////////////////////////////////////////////////////////

#endif // __LIGHTINFO_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

