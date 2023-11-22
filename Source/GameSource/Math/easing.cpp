#include "easing.h"
#include <DirectXMath.h>

//==============================================================================
//
//		Easing.cpp
//
//      Robert Penner's Easing Functions
//      URL http://robertpenner.com/easing/
//      上記サイトのC++をクリックし、PennerEasingをクリックすると、各種easing関数を
//      入手することができる
//
//==============================================================================

#if 20
//******************************************************************************
// TODO:20 イージング関数
//------------------------------------------------------------------------------
/*
課題）
    今までは、パーティクルに0.94fや0.98fをかけ続ける事でアルファを減らしたり、スケールを
    小さくしてきた。ここで、イージング関数というものを取り入れて、より思い通りの曲線で数値が
    変位するようにしたい。

    インターネットで「イージング関数」もしくは「easing」等で検索して、使い方を調べてみなさい。

ヒント）
    イージング関数の見本がダウンロードできるサイトを掲載しておく。
    なお、こういった解説サイトはいつ消えるかわからないので、必要なページは各自でバックアップを
    取っておいた方が良い。

    Robert Penner's Easing Functions
    https://github.com/jesusgollonet/ofpennereasing
    PennerEasingをクリックして、必要な.cppと.hを入手できる。


    下記サイトでは各イージング関数の動きを視覚的に確認できる。

    Easing Functions Cheat Sheet
    https://easings.net/
*/
//******************************************************************************
#endif

//例）
// Sine
float Sine::easeIn(float t, float b, float c, float d) {
    return static_cast<float>(-c * cosf(t / d * (DirectX::XM_PI / 2)) + c + b);
}

float Sine::easeOut(float t, float b, float c, float d) {
    return static_cast<float>(c * sinf(t / d * (DirectX::XM_PI / 2)) + b);
}

float Sine::easeInOut(float t, float b, float c, float d) {
    return static_cast<float>(-c / 2 * (cosf(DirectX::XM_PI * t / d) - 1) + b);
}

// Quad
float Quad::easeIn(float t, float b, float c, float d) {
    return c * (t /= d) * t + b;
}
float Quad::easeOut(float t, float b, float c, float d) {
    return -c * (t /= d) * (t - 2) + b;
}

float Quad::easeInOut(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1) return ((c / 2) * (t * t)) + b;
    return -c / 2 * (((t - 2) * (--t)) - 1) + b;
    /*
    originally return -c/2 * (((t-2)*(--t)) - 1) + b;

    I've had to swap (--t)*(t-2) due to diffence in behaviour in
    pre-increment operators between java and c++, after hours
    of joy
    */
}

// Cubic
float Cubic::easeIn(float t, float b, float c, float d) {
    return c * (t /= d) * t * t + b;
}
float Cubic::easeOut(float t, float b, float c, float d) {
    return c * ((t = t / d - 1) * t * t + 1) + b;
}

float Cubic::easeInOut(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1) return c / 2 * t * t * t + b;
    return c / 2 * ((t -= 2) * t * t + 2) + b;
}

// Quart
float Quart::easeIn(float t, float b, float c, float d) {
    return c * (t /= d) * t * t * t + b;
}
float Quart::easeOut(float t, float b, float c, float d) {
    return -c * ((t = t / d - 1) * t * t * t - 1) + b;
}

float Quart::easeInOut(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1) return c / 2 * t * t * t * t + b;
    return -c / 2 * ((t -= 2) * t * t * t - 2) + b;
}

// Quint
float Quint::easeIn(float t, float b, float c, float d) {
    return c * (t /= d) * t * t * t * t + b;
}
float Quint::easeOut(float t, float b, float c, float d) {
    return c * ((t = t / d - 1) * t * t * t * t + 1) + b;
}

float Quint::easeInOut(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1) return c / 2 * t * t * t * t * t + b;
    return c / 2 * ((t -= 2) * t * t * t * t + 2) + b;
}

// Expo
float Expo::easeIn(float t, float b, float c, float d) {
    return static_cast<float>((t == 0) ? b : c * pow(2, 10 * (t / d - 1)) + b);
}
float Expo::easeOut(float t, float b, float c, float d) {
    return static_cast<float>((t == d) ? b + c : c * (-pow(2, -10 * t / d) + 1) + b);
}

float Expo::easeInOut(float t, float b, float c, float d) {
    if (t == 0) return b;
    if (t == d) return b + c;
    if ((t /= d / 2) < 1) return static_cast<float>(c / 2 * pow(2, 10 * (t - 1)) + b);
    return static_cast<float>(c / 2 * (-pow(2, -10 * --t) + 2) + b);
}

// Circ
float Circ::easeIn(float t, float b, float c, float d) {
    return static_cast<float>(-c * (sqrt(1 - (t /= d) * t) - 1) + b);
}
float Circ::easeOut(float t, float b, float c, float d) {
    return static_cast<float>(c * sqrt(1 - (t = t / d - 1) * t) + b);
}

float Circ::easeInOut(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1) return static_cast<float>(-c / 2 * (sqrt(1 - t * t) - 1) + b);
    return static_cast<float>(c / 2 * (sqrt(1 - t * (t -= 2)) + 1) + b);
}

// Back
float Back::easeIn(float t, float b, float c, float d) {
    float s = 1.70158f;
    float postFix = t /= d;
    return c * (postFix)*t * ((s + 1) * t - s) + b;
}
float Back::easeOut(float t, float b, float c, float d) {
    float s = 1.70158f;
    return c * ((t = t / d - 1) * t * ((s + 1) * t + s) + 1) + b;
}

float Back::easeInOut(float t, float b, float c, float d) {
    float s = 1.70158f;
    if ((t /= d / 2) < 1) return c / 2 * (t * t * (((s *= (1.525f)) + 1) * t - s)) + b;
    float postFix = t -= 2;
    return c / 2 * ((postFix)*t * (((s *= (1.525f)) + 1) * t + s) + 2) + b;
}


// Elastic
float Elastic::easeIn(float t, float b, float c, float d) {
    if (t == 0) return b;  if ((t /= d) == 1) return b + c;
    float p = d * .3f;
    float a = c;
    float s = p / 4;
    float postFix = static_cast<float>(a * pow(2, 10 * (t -= 1))); // this is a fix, again, with post-increment operators
    return static_cast<float>(-(postFix * sin((t * d - s) * (2 * PI) / p)) + b);
}

float Elastic::easeOut(float t, float b, float c, float d) {
    if (t == 0) return b;  if ((t /= d) == 1) return b + c;
    float p = d * .3f;
    float a = c;
    float s = p / 4;
    return static_cast<float>((a * pow(2, -10 * t) * sin((t * d - s) * (2 * PI) / p) + c + b));
}

float Elastic::easeInOut(float t, float b, float c, float d) {
    if (t == 0) return b;  if ((t /= d / 2) == 2) return b + c;
    float p = d * (.3f * 1.5f);
    float a = c;
    float s = p / 4;

    if (t < 1) {
        float postFix = static_cast<float>(a * pow(2, 10 * (t -= 1))); // postIncrement is evil
        return static_cast<float>(-.5f * (postFix * sin((t * d - s) * (2 * PI) / p)) + b);
    }
    float postFix = static_cast<float>(a * pow(2, -10 * (t -= 1))); // postIncrement is evil
    return static_cast<float>(postFix * sin((t * d - s) * (2 * PI) / p) * .5f + c + b);
}

// Bounce
float Bounce::easeIn(float t, float b, float c, float d) {
    return c - easeOut(d - t, 0, c, d) + b;
}
float Bounce::easeOut(float t, float b, float c, float d) {
    if ((t /= d) < (1 / 2.75f)) {
        return c * (7.5625f * t * t) + b;
    }
    else if (t < (2 / 2.75f)) {
        float postFix = t -= (1.5f / 2.75f);
        return c * (7.5625f * (postFix)*t + .75f) + b;
    }
    else if (t < (2.5 / 2.75)) {
        float postFix = t -= (2.25f / 2.75f);
        return c * (7.5625f * (postFix)*t + .9375f) + b;
    }
    else {
        float postFix = t -= (2.625f / 2.75f);
        return c * (7.5625f * (postFix)*t + .984375f) + b;
    }
}

float Bounce::easeInOut(float t, float b, float c, float d) {
    if (t < d / 2) return easeIn(t * 2, 0, c, d) * .5f + b;
    else return easeOut(t * 2 - d, 0, c, d) * .5f + c * .5f + b;
}