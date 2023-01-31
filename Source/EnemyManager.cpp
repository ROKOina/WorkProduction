#include <imgui.h>
#include "EnemyManager.h"
#include "Collision.h"
#include "EnemySlime.h"

//更新処理
void EnemyManager::Upadate(float elapsedTime)
{
    for (Enemy* enemy : enemies)
    {
        enemy->Update(elapsedTime);

    }

    //破棄処理(更新処理が終わった後に一気に削除
    for (Enemy* enemy : removes)
    {
        std::vector<Enemy*>::iterator it =
            std::find(enemies.begin(), enemies.end(), enemy);
        if (it != enemies.end())
        {
            enemies.erase(it);
        }
        //敵の破棄処理
        delete enemy;
    }
    //破棄リストをクリア
    removes.clear();

    //敵同士の当たり判定
    CollsionEnemyVsEnemies();
}

//エネミー同士の衝突判定
void EnemyManager::CollsionEnemyVsEnemies()
{

    int enemyCount = GetEnemyCount();
    for (int i = 0; i < enemyCount; ++i)
    {
        for (int e = 0; e < enemyCount; ++e)//2体のエネミーを回すため2重for文
        {
            if (i == e)continue;    //同じエネミーならコンテニュー
            Enemy* enemyI = GetEnemy(i);
            Enemy* enemyE = GetEnemy(e);    //当たり判定をするエネミー2体取得
                
                    //衝突処理
            DirectX::XMFLOAT3 outPosition;
            //if (Collision::IntersectSphereVsSphere(
            //    enemyI->GetPosition(), enemyI->GetRadius(),
            //    enemyE->GetPosition(), enemyE->GetRadius(),
            //    outPosition))
            //{
            //    //押し出し後の位置設定
            //    enemyE->SetPosition(outPosition);
            //}

            if (Collision::IntersectCylinder(
                enemyI->GetPosition(), enemyI->GetRadius(), enemyI->GetHeight(),
                enemyE->GetPosition(), enemyE->GetRadius(), enemyE->GetHeight(),
                outPosition
            ))
            {
                //押し出し後の位置設定
                enemyE->SetPosition(outPosition);
            }

        }
    }
}

//描画処理
void EnemyManager::Render(ID3D11DeviceContext* dc, Shader* shader)
{
    for (Enemy* enemy : enemies)
    {
        enemy->Render(dc, shader);
    }
}

void EnemyManager::DrawDebugPrimitive()
{
    if (!VisibleDebugPrimitive)return;

    for (Enemy* enemy : enemies)
    {
        enemy->DrawDebugPrimitive();
    }
}

void EnemyManager::DrawDebugGUI()
{
    if (ImGui::CollapsingHeader("EnemyManager", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("enemyLeftClickSet", &enemyLeftClickSet);

        if (ImGui::Button("Add"))
        {
            EnemySlime* enemy = new EnemySlime();
            enemy->SetPosition({ (float)(rand() % 100 - 50) / 10.0f,0,(float)(rand() % 100 - 50) / 10.0f });
            Register(enemy);
        }
        if (ImGui::Button("Clear"))
        {
            for (Enemy* enemy : enemies)
            {
                Remove(enemy);
            }
        }
        ImGui::Checkbox("VisibleDebugPrimitiveEnemy", &VisibleDebugPrimitive);

        //for (Enemy* enemy : enemies)
        //{
        //    if (ImGui::CollapsingHeader("a", ImGuiTreeNodeFlags_OpenOnArrow))
        //    {
        //        DirectX::XMFLOAT3 p = enemy->GetPosition();
        //        ImGui::InputFloat3("Position", &p.x);
        //        enemy->SetPosition(p);
        //    }

        //}
        
        for (int i=0;i<enemies.size();++i)
        {
            char number[] = { (char)i ,'\0'};
            if(ImGui::TreeNode(number))
            {
                enemies.at(i)->GuiEnemy();
                
                ImGui::TreePop();
            }
        }

    }

}

//エネミー登録
void EnemyManager::Register(Enemy* enemy)
{
    enemies.emplace_back(enemy);
}

//エネミー削除
void EnemyManager::Remove(Enemy* enemy)
{
    //破棄リストに追加
    removes.insert(enemy);
}

//エネミー全削除
void EnemyManager::Clear()
{
    for (Enemy* enemy : enemies)
    {
        delete enemy;
    }
}
