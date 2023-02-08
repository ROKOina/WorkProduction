#pragma once

//
// モデルのアニメーションを見れるクラス
//

class ModelWatch
{
public:

    ModelWatch() {}
    ~ModelWatch() {}

    void Update(float elapsedTime);
    void Render(float elapsedTime);
};