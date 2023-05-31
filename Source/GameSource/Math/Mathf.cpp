#include "Mathf.h"
#include <stdlib.h>

//線形補完
float Mathf::Lerp(float a, float b, float t)
{
    return a * (1.0f - t) + (b * t);
}

//指定範囲のランダム値を計算する
float Mathf::RandomRange(float min, float max)
{
    float Range = max - min;

    return (float)(rand() % (int)(Range * 100)) / 100 + min;
}