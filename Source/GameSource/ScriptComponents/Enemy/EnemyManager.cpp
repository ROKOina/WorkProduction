#include "EnemyManager.h"

#include "Components/System/GameObject.h"
#include "Components/TransformCom.h"
#include "EnemyCom.h"
#include "EnemyNearCom.h"
#include "EnemyFarCom.h"

// 更新処理
void EnemyManager::Update(float elapsedTime)
{
    
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
            //送信者の場合は飛ばす
            if (telegram.sender == e.enemy.lock()->GetComponent<EnemyCom>()->GetID())continue;

            if (nearCount >= nearEnemyLevel.inRadiusCount)
                break;

            //近接敵にキャスト
            std::shared_ptr<EnemyNearCom> nearEnemy = std::static_pointer_cast<EnemyNearCom>(e.enemy.lock()->GetComponent<EnemyCom>());

            if(nearEnemy->GetIsNearFlag())
                nearCount++;

            //DirectX::XMFLOAT3 enemyPos = nearEnemy.enemy.lock()->transform_->GetWorldPosition();
            ////プレイヤーとの距離を算出
            //float length = DirectX::XMVectorGetX(
            //    DirectX::XMVector3Length(
            //        DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&enemyPos))));
            ////接近しているなら
            //if (length <= nearEnemyLevel.radius)
            //    nearCount++;
        }
        //近くにいる敵が決められている数より小さい時
        if (nearCount < nearEnemyLevel.inRadiusCount)
        {
            //接近許可を送る
            SendMessaging(static_cast<int>(AI_ID::AI_INDEX), telegram.sender, MESSAGE_TYPE::MsgGiveNearRight);
            return true;
        }
        break;
    }
    }
    return false;
}

//IDから敵をゲット
std::shared_ptr<GameObject> EnemyManager::GetEnemyFromId(int id)
{
    for (EnemyData& e : nearEnemies_)
    {
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
