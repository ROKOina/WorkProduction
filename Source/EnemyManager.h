#pragma once

#include <vector>
#include "Enemy.h"
#include <set>

//エネミーマネジャー
class EnemyManager
{
private:
    EnemyManager(){}
    ~EnemyManager(){}

    //エネミー同士の衝突判定
    void CollsionEnemyVsEnemies();

public:
    //唯一のインスタンス取得
    static EnemyManager& Instance()
    {
        static EnemyManager instance;
        return instance;
    }

    //更新処理
    void Upadate(float elapsedTime);

    //描画処理
    void Render(ID3D11DeviceContext* dc, Shader* shader);

    void DrawDebugPrimitive();

    void DrawDebugGUI();

    //エネミー数取得
    int GetEnemyCount()const { return static_cast<int>(enemies.size()); }

    //エネミー取得
    Enemy* GetEnemy(int index) { return enemies.at(index); }

    //エネミー登録
    void Register(Enemy* enemy);

    //エネミー削除
    void Remove(Enemy* enemy);

    //エネミー全削除
    void Clear();

    //エネミー配置bool
    bool enemyLeftClickSet = false;


private:
    std::vector<Enemy*> enemies;
    std::set<Enemy*>removes;

    //ImGui
    bool VisibleDebugPrimitive = true;

};