#pragma once

#include "GameObject.h"

//コンポーネント
class Component
{
public:
    Component(){}
    virtual ~Component(){}

	// 名前取得
	virtual const char* GetName() const = 0;

	// 開始処理
	virtual void Start() {}

	// 更新処理
	virtual void Update(float elapsedTime) {}

	// GUI描画
	virtual void OnGUI() {}

	// ゲームオブジェクト設定
	void SetGameObject(std::shared_ptr<GameObject> obj) { this->gameObject = obj; }

	// ゲームオブジェクト取得
	std::shared_ptr<GameObject> GetGameObject() { return gameObject.lock(); }

private:
	std::weak_ptr<GameObject>	gameObject;

};