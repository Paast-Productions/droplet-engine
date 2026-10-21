#include "EasingFunctions.hpp"
#include <corecrt_math.h>

namespace Droplet::Math::Easing
{
    constexpr auto PI = 3.14159265358979323846f;

    constexpr float c1 = 1.70158f;
    constexpr float c2 = c1 * 1.525f;
    constexpr float c3 = c1 + 1;
    constexpr float c4 = (2 * PI) / 3;
    constexpr float c5 = 2 * PI / 4.5f;

    constexpr float n1 = 7.5625f;
    constexpr float d1 = 2.75f;


    float EaseInSine(float fraction)
    {
        return 1 - cosf((fraction * PI) * 0.5f);
    }

    float EaseOutSine(float fraction)
    {
        return sinf((fraction * PI) * 0.5f);
    }

    float EaseInOutSine(float fraction)
    {
        return -(cosf(fraction * PI) - 1) * 0.5f;
    }

    float EaseInQuad(float fraction)
    {
        return fraction * fraction;
    }

    float EaseOutQuad(float fraction)
    {
        return 1 - (1 - fraction) * (1 - fraction);
    }

    float EaseInOutQuad(float fraction)
    {
        if (fraction < 0.5f)
            return 2 * fraction * fraction;
        return 1 - powf(-2 * fraction + 2, 2) * 0.5f;
    }

    float EaseInCubic(float fraction)
    {
        return powf(fraction, 3);
    }

    float EaseOutCubic(float fraction)
    {
        return 1 - powf(1 - fraction, 3);
    }

    float EaseInOutCubic(float fraction)
    {
        if (fraction < 0.5f)
            return 4 * powf(fraction, 3);
        return 1 - powf(-2 * fraction + 2, 3) * 0.5f;
    }

    float EaseInQuart(float fraction)
    {
        return fraction * fraction * fraction * fraction;
    }

    float EaseOutQuart(float fraction)
    {
        return 1 - powf(1 - fraction, 4);
    }

    float EaseInOutQuart(float fraction)
    {
        if (fraction < 0.5f)
            return 8 * fraction * fraction * fraction * fraction;
        return 1 - powf(-2 * fraction + 2, 4) * 0.5f;
    }

    float EaseInQuint(float fraction)
    {
        return fraction * fraction * fraction * fraction * fraction;
    }

    float EaseOutQuint(float fraction)
    {
        return 1 - powf(1 - fraction, 5);
    }

    float EaseInOutQuint(float fraction)
    {
        if (fraction < 0.5f)
            return 16 * fraction * fraction * fraction * fraction * fraction;
        return 1 - powf(-2 * fraction + 2, 5) * 0.5f;
    }

    float EaseInExpo(float fraction)
    {
        if (fraction <= 0)
            return 0;
        return powf(2, 10 * fraction - 10);
    }

    float EaseOutExpo(float fraction)
    {
        if (fraction >= 1)
            return 1;
        return 1 - pow(2, -10 * fraction);
    }

    float EaseInOutExpo(float fraction)
    {
        if (fraction <= 0)
            return 0;
        if (fraction >= 1)
            return 1;
        if (fraction < 0.5f)
            return powf(2, 20 * fraction - 10) * 0.5f;
        return (2 - powf(2, -20 * fraction + 10)) * 0.5f;
    }

    float EaseInCirc(float fraction)
    {
        if (fraction >= 1)
            return 1;
        if (fraction <= 0)
            return 0;
        return 1 - sqrtf(1 - powf(fraction, 2));
    }

    float EaseOutCirc(float fraction)
    {
        if (fraction >= 1)
            return 1;
        if (fraction <= 0)
            return 0;
        return sqrtf(1 - powf(fraction - 1, 2));
    }

    float EaseInOutCirc(float fraction)
    {
        if (fraction >= 1)
            return 1;
        if (fraction <= 0)
            return 0;
        if (fraction < 0.5f)
            return (1 - sqrtf(1 - powf(2 * fraction, 2))) * 0.5f;
        return (sqrtf(1 - powf(-2 * fraction + 2, 2)) + 1) * 0.5f;
    }

    float EaseInBack(float fraction)
    {
        return c3 * fraction * fraction * fraction - c1 * fraction * fraction;
    }

    float EaseOutBack(float fraction)
    {
        return 1 + c3 * powf(fraction - 1, 3) + c1 * powf(fraction - 1, 2);
    }

    float EaseInOutBack(float fraction)
    {
        if (fraction <= 0)
            return 0;
        if (fraction >= 1)
            return 1;
        if (fraction < 0.5f)
            return (powf(2 * fraction, 2) * ((c2 + 1) * 2 * fraction - c2)) * 0.5f;
        return (powf(2 * fraction - 2, 2) * ((c2 + 1) * (fraction * 2 - 2) + c2) + 2) * 0.5f;
    }

    float EaseInElastic(float fraction)
    {
        if (fraction <= 0)
            return 0;
        else if (fraction >= 1)
            return 1;
        return -powf(2, 10 * fraction - 10) * sinf((fraction * 10 - 10.75f) * c4);
    }

    float EaseOutElastic(float fraction)
    {
        if (fraction <= 0)
            return 0;
        else if (fraction >= 1)
            return 1;
        return -powf(2, 10 * fraction - 10) * sinf((fraction * 10 - 10.75f) * c4);
    }

    float EaseInOutElastic(float fraction)
    {
        if (fraction <= 0)
            return 0;
        else if (fraction >= 1)
            return 1;
        else if (fraction < 0.5f)
            return (-(powf(2, 20 * fraction - 10) * sinf((20 * fraction - 11.125f) * c5)) * 0.5f);
        return ((powf(2, -20 * fraction + 10) * sinf((20 * fraction - 11.125f) * c5)) * 0.5f + 1.0f);
    }

    float EaseInBounce(float fraction)
    {
        return 1 - EaseOutBounce(1 - fraction);
    }

    float EaseOutBounce(float fraction)
    {
        float tempFrac = fraction;

        if (tempFrac < 1 / d1) 
            return n1 * tempFrac * tempFrac;
        else if (tempFrac < 2 / d1) 
        {
            tempFrac -= 1.5f / d1;
            return n1 * (tempFrac) * tempFrac + 0.75f;
        }
        else if (tempFrac < 2.5f / d1) 
        {
            tempFrac -= 2.25f / d1;
            return n1 * (tempFrac) * tempFrac + 0.9375f;
        }
        tempFrac -= 2.625f / d1;
        return n1 * (tempFrac / d1) * tempFrac + 0.984375f;
    }

    float EaseInOutBounce(float fraction)
    {
        if (fraction < 0.5f)
            return (1 - EaseOutBounce(1 - 2 * fraction)) * 0.5f;
        return (1 + EaseOutBounce(2 * fraction - 1)) * 0.5f;
    }

}
