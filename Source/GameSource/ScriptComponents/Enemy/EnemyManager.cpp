#include "EnemyManager.h"

#include <imgui.h>

#include "Components/System/GameObject.h"
#include "Components/TransformCom.h"
#include "EnemyCom.h"
#include "EnemyNearCom.h"
#include "EnemyFarCom.h"

// 更新処理
void EnemyManager::Update(float elapsedTime)
{
    
}

//GUI
void EnemyManager::OnGui()
{
    //近接的に接近フラグカウント
    int nearFlagCount = GetCurrentNearFlagCount();
    ImGui::InputInt("nearFlagCount", &nearFlagCount);

    //近接的に経路探査フラグカウント
    int nearPathCount = GetCurrentNearPathCount();
    ImGui::InputInt("nearPathCount", &nearPathCount);
}

// 敵登録
void EnemyManager::Register(std::shared_ptr<GameObject> enemy, EnemyKind enemyKind)
{
    //敵データを作成
    EnemyData e;
    e.enemy = enemy;
    e.enemy.lock()->GetComponent<EnemyCom>()->SetID(AI_ID::ENEMY_INDEX + currentIndex_);

    //登録
    switch (enemyKind)
    {
    case EnemyManager::EnemyKind::NEAR_ENEMY:
        nearEnemies_.emplace_back(e);
        break;
    case EnemyManager::EnemyKind::FAR_ENEMY:
        farEnemies_.emplace_back(e);
        break;
    default:
        break;
    }
   
    currentIndex_++;
}

//近接敵の攻撃カウント取得
int EnemyManager::GetCurrentNearAttackCount()
{
    int attackCount = 0;
    for (auto& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyCom>()->GetIsAttackFlag())
            attackCount++;
    }

    return attackCount;
}

//近接敵の接近カウント取得
int EnemyManager::GetCurrentNearFlagCount()
{
    int nearFlagCount = 0;
    for (auto& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsNearFlag())
            nearFlagCount++;
    }

    return nearFlagCount;
}

//近接敵の経路探査カウント取得
int EnemyManager::GetCurrentNearPathCount()
{
    int nearPathCount = 0;
    for (auto& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyNearCom>()->GetIsPathFlag())
            nearPathCount++;
    }

    return nearPathCount;
}


/////   AI関係   /////

// メッセージ受信したときの処理
bool EnemyManager::OnMessage(const Telegram& telegram)
{
    switch (telegram.msg)
    {
    case MESSAGE_TYPE::MsgAskNearRight: //プレイヤーへの接近を要求
    {
        DirectX::XMFLOAT3 playerPos= player_.lock()->transform_->GetWorldPosition();
        int nearCount = 0;  //近くにいる敵の数
        for (EnemyData& e : nearEnemies_)
        {
            if (e.enemy.expired())continue;
            //送信者の場合は飛ばす
            if (telegram.sender == e.enemy.lock()->GetComponent<EnemyCom>()->GetID())continue;

            if (nearCount >= nearEnemyLevel_.inRadiusCount)
                break;

            //近接敵にキャスト
            std::shared_ptr<EnemyNearCom> nearEnemy = e.enemy.lock()->GetComponent<EnemyNearCom>();

            if(nearEnemy->GetIsNearFlag())
                nearCount++;
        }
        //近くにいる敵が決められている数より小さい時
        if (nearCount < nearEnemyLevel_.inRadiusCount)
        {
            //接近許可を送る
            SendMessaging(static_cast<int>(AI_ID::AI_INDEX), telegram.sender, MESSAGE_TYPE::MsgGiveNearRight);
            return true;
        }
        break;
    }
    case MESSAGE_TYPE::MsgAskAttackRight: //プレイヤーへの攻撃を要求
    {
        std::shared_ptr<GameObject> enemy = GetEnemyFromId(telegram.sender);
        int attackCount = 0;  //攻撃中の敵カウント
        //近接か遠隔か判断
        //近接敵にキャスト
        std::shared_ptr<EnemyNearCom> nearEnemy = enemy->GetComponent<EnemyNearCom>();
        //近接敵処理
        if (nearEnemy)
        {
            attackCount = GetCurrentNearAttackCount();

            //同時攻撃可能数時
            if (attackCount < nearEnemyLevel_.togetherAttackCount)
            {
                //攻撃許可を送る
                SendMessaging(static_cast<int>(AI_ID::AI_INDEX), telegram.sender, MESSAGE_TYPE::MsgGiveAttackRight);
                return true;
            }
            break;
        }
        //遠隔的処理
        else
        {

        }

    }
    }
    return false;
}

//IDから敵をゲット
std::shared_ptr<GameObject> EnemyManager::GetEnemyFromId(int id)
{
    for (EnemyData& e : nearEnemies_)
    {
        if (e.enemy.expired())continue;
        if (e.enemy.lock()->GetComponent<EnemyCom>()->GetID() == id)
            return e.enemy.lock();
    }
    for (EnemyData& e : farEnemies_)
    {
        if (e.enemy.lock()->GetComponent<EnemyCom>()->GetID() == id)
            return e.enemy.lock();
    }

    return nullptr;
}

//空の敵を削除
void EnemyManager::EraseExpiredEnemy()
{
    for (int i = 0; i < nearEnemies_.size();)
    {
        if (nearEnemies_[i].enemy.expired())
            nearEnemies_.erase(nearEnemies_.begin() + i);
        else
            i++;
    }
}



// メッセージ送信関数
void EnemyManager::SendMessaging(int sender, int receiver, MESSAGE_TYPE msg)
{
    if (receiver == static_cast<int>(AI_ID::AI_INDEX))
    {// MetaAI宛の時
        //メッセージデータを作成
        Telegram telegram(sender, receiver, msg);
        // メッセージ送信
        OnMessage(telegram);
    }
    else
    {// エネミーが受信者のとき、どのエネミーに送信するか
        // 受信者のポインタを取得
        std::weak_ptr<GameObject> receiveEnemy = GetEnemyFromId(receiver);
        //レシーバー居ないとき関数を終了する
        if(!receiveEnemy.lock()->GetComponent<EnemyCom>()) return;
        //メッセージデータを作成
        Telegram telegram(sender, receiver, msg);
        //ディレイ無しメッセージ（即時配送メッセージ）
        receiveEnemy.lock()->GetComponent<EnemyCom>()->OnMessage(telegram);
    }
}
