#pragma once
#include "util/VecBase.hpp"

inline void HSV_set(float h, float s, float v, float& r, float& g, float& b)
{
        h *= 6.0f;

        int i = int(h);
        float f = h-i;

        float p = v*(1-s);
        float q = v*(1-(s*f));
        float t = v*(1-(s*(1-f)));

        switch (i) {
        case 0: r=v; g=t; b=p; break;
        case 1: r=q; g=v; b=p; break;
        case 2: r=p; g=v; b=t; break;
        case 3: r=p; g=q; b=v; break;
        case 4: r=t; g=p; b=v; break;
        default: r=v; g=p; b=q; break;
        }
}

inline
Vec3 HSV(float h, float s, float v)
{
        float r,g,b;
        HSV_set(h, s, v, r, g, b);
		return Vec3(r,g,b);
}
