#pragma once

//https://easings.net/

namespace Droplet::Math::Easing
{
    enum EasingFunction
    {
        Func_None,
        Func_Linear,
        Func_EaseInSine,
        Func_EaseOutSine,
        Func_EaseInOutSine,
        Func_EaseInQuad,
        Func_EaseOutQuad,
        Func_EaseInOutQuad,
        Func_EaseInCubic,
        Func_EaseOutCubic,
        Func_EaseInOutCubic,
        Func_EaseInQuart,
        Func_EaseOutQuart,
        Func_EaseInOutQuart,
        Func_EaseInQuint,
        Func_EaseOutQuint,
        Func_EaseInOutQuint,
        Func_EaseInExpo,
        Func_EaseOutExpo,
        Func_EaseInOutExpo,
        Func_EaseInCirc,
        Func_EaseOutCirc,
        Func_EaseInOutCirc,
        Func_EaseInBack,
        Func_EaseOutBack,
        Func_EaseInOutBack,
        Func_EaseInElastic,
        Func_EaseOutElastic,
        Func_EaseInOutElastic,
        Func_EaseInBounce,
        Func_EaseOutBounce,
        Func_EaseInOutBounce,
    };

    float EaseInSine(float fraction);

    float EaseOutSine(float fraction);

    float EaseInOutSine(float fraction);

    float EaseInQuad(float fraction);

    float EaseOutQuad(float fraction);

    float EaseInOutQuad(float fraction);

    float EaseInCubic(float fraction);

    float EaseOutCubic(float fraction);

    float EaseInOutCubic(float fraction);

    float EaseInQuart(float fraction);

    float EaseOutQuart(float fraction);

    float EaseInOutQuart(float fraction);

    float EaseInQuint(float fraction);

    float EaseOutQuint(float fraction);

    float EaseInOutQuint(float fraction);

    float EaseInExpo(float fraction);

    float EaseOutExpo(float fraction);

    float EaseInOutExpo(float fraction);

    float EaseInCirc(float fraction);

    float EaseOutCirc(float fraction);

    float EaseInOutCirc(float fraction);

    float EaseInBack(float fraction);

    float EaseOutBack(float fraction);

    float EaseInOutBack(float fraction);

    float EaseInElastic(float fraction);

    float EaseOutElastic(float fraction);

    float EaseInOutElastic(float fraction);

    float EaseInBounce(float fraction);

    float EaseOutBounce(float fraction);

    float EaseInOutBounce(float fraction);
}
