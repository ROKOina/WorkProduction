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

	//sprite描画用
	virtual void Render2D(float elapsedTime){}

	//音解放
	virtual void AudioRelease() {}

	// ゲームオブジェクト設定
	void SetGameObject(std::shared_ptr<GameObject> obj) { this->gameObject_ = obj; }

	// ゲームオブジェクト取得
	std::shared_ptr<GameObject> GetGameObject() { return gameObject_.lock(); }

	const bool GetEnabled() const { return isEnabled_; }
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }

private:
	std::weak_ptr<GameObject>	gameObject_;

protected:
	//有効か
	bool isEnabled_ = true;
};