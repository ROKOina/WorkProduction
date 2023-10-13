#include "EnemyManager.h"

#include "Components/System/GameObject.h"
#include "EnemyCom.h"

// XVˆ—
void EnemyManager::Update(float elapsedTime)
{
    
}

// “G“o˜^
void EnemyManager::Register(std::shared_ptr<GameObject> enemy)
{
    //“Gƒf[ƒ^‚ğì¬
    EnemyData e;
    e.index = currentIndex;
    e.enemy = enemy;
    //“o˜^
    enemies.emplace_back(e);
    currentIndex++;
}

