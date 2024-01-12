#include "PlayerCom.h"
#include "Input/Input.h"
#include <imgui.h>
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"
#include "Components\AnimationCom.h"
#include "Components\AnimatorCom.h"
#include "Components\RendererCom.h"
#include "Components\ColliderCom.h"
#include "Components\MovementCom.h"

#include "../Enemy/EnemyCom.h"
#include "../Weapon\WeaponCom.h"
#include "../CharacterStatusCom.h"

#include "Graphics\Shaders\PostEffect.h"
#include  "GameSource/Math/easing.h"
#include "../Score.h"

// 開始処理
void PlayerCom::Start()
{
    //カメラをプレイヤーにフォーカスする
    std::shared_ptr<GameObject> cameraObj = GameObjectManager::Instance().Find("Camera");

    //プレイヤーのワールドポジションを取得
    DirectX::XMFLOAT3 wp = GetGameObject()->transform_->GetWorldPosition();
    wp.z -= 10;
    wp.y += 6;
    cameraObj->transform_->SetLocalPosition(wp);

    //当たり大きさ
    GetGameObject()->GetComponent<CapsuleColliderCom>()->SetRadius(0.17f);

    //武器ステータス初期化
    //Candy
    std::shared_ptr<WeaponCom> weapon = GetGameObject()->GetChildFind("Candy")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_01, 5, 15, 0.8f, 0.2f, 1.5f);
    weapon->SetAttackStatus(BIGSWORD_DASH, 2, 100, 0.9f, 0.1f);
    //ジャンプ攻撃
    weapon->SetAttackStatus(JUMP_ATTACK_UPPER, 4, 20, 0.0f, 1.0f, 1.5f, ATTACK_SPECIAL_TYPE::JUMP_START | ATTACK_SPECIAL_TYPE::UNSTOP);
    weapon->SetAttackStatus(JUMP_ATTACK_01, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
    weapon->SetAttackStatus(JUMP_ATTACK_DOWN_DO, 8, 1, 0.0f, -1.0f);

    //CandyCircle
    weapon = GetGameObject()->GetChildFind("CandyCircle")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_02, 5, 15, 0.3f, 0.7f);
    weapon->SetAttackStatus(BIGSWORD_COM2_02, 3, 10, 1.0f, 0.0f);
    weapon->SetAttackStatus(JUMP_ATTACK_02, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);

    //CandyStick
    weapon = GetGameObject()->GetChildFind("CandyStick")->GetComponent<WeaponCom>();
    weapon->SetAttackStatus(BIGSWORD_COM1_03, 5, 15, 0.9f, 0.1f, 2.0f);
    weapon->SetAttackStatus(JUMP_ATTACK_03, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);
    weapon->SetAttackStatus(JUMP_ATTACK_06, 3, 20, 0.0f, 1.0f, 1.0f, ATTACK_SPECIAL_TYPE::JUMP_NOW);

    std::shared_ptr<PlayerCom> myCom = GetGameObject()->GetComponent<PlayerCom>();
    //攻撃管理を初期化
    attackPlayer_ = std::make_shared<AttackPlayer>(myCom);
    //移動管理を初期化
    movePlayer_ = std::make_shared<MovePlayer>(myCom);
    //ジャスト回避管理を初期化
    justAvoidPlayer_ = std::make_shared<JustAvoidPlayer>(myCom);

    //アニメーション初期化
    AnimationInitialize();

    //ステータス設定
    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    status->SetMaxHP(8);
    status->SetHP(8);

    //ダメージビネット
    vignetteDamageTimer_ = -1;
    vignetteDamageTime_ = 0;
    vignetteDamagePower_ = 0;
    //瀕死時
    vignetteHPPower_[0].power = 0.25f;
    vignetteHPPower_[0].speed = 4;
    vignetteHPPower_[0].powerOffset = 0.3f;
    vignetteHPPower_[1].power = 0.18f;
    vignetteHPPower_[1].speed = 3;
    vignetteHPPower_[1].powerOffset = 0.3f;
    vignetteHPPower_[2].power = 0.0f;
    vignetteHPPower_[2].speed = 2;
    vignetteHPPower_[2].powerOffset = 0.3f;
    vignetteLowHP_ = 0;

    //UI初期化
    startUI_ = false;
    hpSprite_[0] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonutsOne.png");
    hpSprite_[1] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts2.png");
    hpSprite_[2] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts3.png");
    hpSprite_[3] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts4.png");
    hpSprite_[4] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts5.png");
    hpSprite_[5] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts6.png");
    hpSprite_[6] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts7.png");
    hpSprite_[7] = std::make_unique<Sprite>("./Data/Sprite/GameUI/Player/faceDonuts/faceDonuts8.png");

    hpDonutsPos_[0] = { 48.3f,3.1f };
    hpDonutsPos_[1] = { 6.0f,44.0f };
    hpDonutsPos_[2] = { 5.9f,107.3f };
    hpDonutsPos_[3] = { 49.0f,146.5f };
    hpDonutsPos_[4] = { 110.9f,145.3f };
    hpDonutsPos_[5] = { 153.9f,101.4f };
    hpDonutsPos_[6] = { 153.9f,42.9f };
    hpDonutsPos_[7] = { 112.2f,7.3f };

    buttonPos_ = { 1270.0f,769.0f };
    offsetButtonPos_[0] = { -45.0f,0.0f };
    offsetButtonPos_[1] = { 0.0f,-45.0f };
    offsetButtonPos_[2] = { 0.0f,45.0f };
    offsetButtonPos_[3] = { 45.0f,0.0f };
    offsetButtonPos_[4] = { 48.0f,-116.0f };

    //文字配置
    spriteRightPost_[0] = false;
    spriteRightPost_[1] = false;
    spriteRightPost_[2] = true;
    spriteRightPost_[3] = true;
    spriteRightPost_[4] = false;


    //アニメーションイベントSE初期化
    animSE.emplace_back("SEAttack", "Data/Audio/Player/attackNormal.wav");
    animSE.emplace_back("SEAttack", "Data/Audio/Player/yaVoice.wav");

}

// 更新処理
void PlayerCom::Update(float elapsedTime)
{
    //ゲーム開始フラグ
    if (!GameObjectManager::Instance().GetIsSceneGameStart())
        return;

    std::shared_ptr<CharacterStatusCom> status = GetGameObject()->GetComponent<CharacterStatusCom>();
    //ダメージ時以外
    if (!status->GetDamageAnimation())
    {
        //移動
        movePlayer_->Update(elapsedTime);

        //ジャスト回避
        justAvoidPlayer_->Update(elapsedTime);

        //攻撃
        attackPlayer_->Update(elapsedTime);
    }

    //ダメージ
    if (status->GetFrameDamage())
    {
        //アニメーター
        std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
        animator->ResetParameterList();
        animator->SetTriggerOn("damageFront");
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        attackPlayer_->AttackFlagEnd();
        movePlayer_->DashEndFlag(true);

        //ビネットスタート
        VignetteStart(0.7f, 1.0f);

        //ダメージボイス
        damageSE_->Play(false);
    }

    //カプセル当たり判定設定
    {
        std::shared_ptr<CapsuleColliderCom> capsule = GetGameObject()->GetComponent<CapsuleColliderCom>();
        //足元から頭までカプセルを設定
        capsule->SetPosition1({ 0,0,0 });
        Model::Node* headNode = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode("Head");
        DirectX::XMMATRIX PWorld = DirectX::XMLoadFloat4x4(&headNode->parent->worldTransform);
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&headNode->translate), PWorld));
        DirectX::XMFLOAT3 playerPos = GetGameObject()->transform_->GetWorldPosition();
        capsule->SetPosition2({ pos.x - playerPos.x,pos.y - playerPos.y,pos.z - playerPos.z });
    }

    //壁判定
    {
        DirectX::XMFLOAT3 kabePlus = GameObjectManager::Instance().Find("kabePlus")->transform_->GetWorldPosition();
        DirectX::XMFLOAT3 kabeMinas = GameObjectManager::Instance().Find("kabeMinas")->transform_->GetWorldPosition();

        DirectX::XMFLOAT3 pos = GetGameObject()->transform_->GetWorldPosition();
        if (pos.x > kabePlus.x)
            pos.x = kabePlus.x;
        if (pos.x < kabeMinas.x)
            pos.x = kabeMinas.x;
        if (pos.z > kabePlus.z)
            pos.z = kabePlus.z;
        if (pos.z < kabeMinas.z)
            pos.z = kabeMinas.z;

        GetGameObject()->transform_->SetWorldPosition(pos);
    }

    //ブラー
    {
        if (blurTimer_ < blurTime_)
        {
            blurTimer_ += elapsedTime;
            ShaderParameter3D& sp = Graphics::Instance().shaderParameter3D_;

            //時間比率
            float ratio = sin(DirectX::XM_PI * (blurTimer_ / blurTime_));

            //パワー代入
            sp.radialBlur.power = blurPower_ * ratio;

            //ポジション設定
            if (!blurBoneName_.empty()) //ボーン設定されている場合
            {
                Model::Node* n = GetGameObject()->GetComponent<RendererCom>()->GetModel()->FindNode(blurBoneName_.c_str());
                DirectX::XMFLOAT3 worldPos ={ n->worldTransform._41,n->worldTransform._42,n->worldTransform._43 };
                DirectX::XMFLOAT3 screenPos = {};
                //スクリーン座標にする
                std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
                screenPos = Graphics::Instance().WorldToScreenPos(worldPos, camera);
                sp.radialBlur.blurPos = { screenPos.x,screenPos.y };
            }
            else
            {
                if (!blurPosObj_.expired()) //オブジェクト指定されている場合
                {
                    DirectX::XMFLOAT3 worldPos = blurPosObj_.lock()->transform_->GetWorldPosition();
                    DirectX::XMFLOAT3 screenPos = {};
                    //スクリーン座標にする
                    std::shared_ptr<CameraCom> camera = GameObjectManager::Instance().Find("Camera")->GetComponent<CameraCom>();
                    screenPos = Graphics::Instance().WorldToScreenPos(worldPos, camera);
                    sp.radialBlur.blurPos = { screenPos.x,screenPos.y };
                }
            }

            //終了化
            if (blurTimer_ >= blurTime_)
            {
                sp.radialBlur.power = 0;
            }
        }

    }

    //ビネット
    {
        //ダメージ時
        if (vignetteDamageTimer_ > 0)
        {
            Vignette& vignette = Graphics::Instance().shaderParameter3D_.vignette;
            vignetteDamageTimer_ -= elapsedTime;

            float ratio = vignetteDamageTimer_ / vignetteDamageTime_;
            float powerRatio = Expo::easeOut(ratio, 0, 1, 1);
            vignette.power = vignetteDamagePower_ * powerRatio;

            if (vignetteDamageTimer_ <= 0)
            {
                vignette.enabled = -1;
            }

            if (status->GetHP() <= 3)
            {
                float timePower = sin(vignetteLowHP_) * 0.5f + 0.5f;
                timePower = vignetteHPPower_[status->GetHP() - 1].power + vignetteHPPower_[status->GetHP() - 1].powerOffset * timePower;

                if (timePower > vignette.power)
                {
                    vignetteDamageTimer_ = -1;
                }
            }
        }
        //瀕死時   
        else if(status->GetHP() <= 3 && status->GetHP() != 0) 
        {
            Vignette& vignette = Graphics::Instance().shaderParameter3D_.vignette;

            //波でビネット
            vignetteLowHP_ += vignetteHPPower_[status->GetHP() - 1].speed * elapsedTime;  //速さ

            float timePower = sin(vignetteLowHP_) * 0.5f + 0.5f;
            timePower = vignetteHPPower_[status->GetHP() - 1].power + vignetteHPPower_[status->GetHP() - 1].powerOffset * timePower;

            vignette.enabled = 1;
            vignette.power = timePower;
        }
    }

    //UI更新
    {
        if (isHpDirection_)
        {
            hpGravity_ += 20 * elapsedTime;
            hpDir_.x += dirVelo_.x * 200 * elapsedTime;
            hpDir_.y += dirVelo_.y * 200 * elapsedTime + hpGravity_;
            hpDir_.z -= elapsedTime * 2;
            if (hpDir_.z < 0)isHpDirection_ = false;
        }

        //ヒット数字演出
        if (hitComboNumPos_.z > hitComboSize_)
        {
            hitComboNumPos_.z -= hitComboDirSpeed_*elapsedTime;
            if (hitComboNumPos_.z <= hitComboSize_)
                hitComboNumPos_.z = hitComboSize_;
        }

        //ヒット継続タイマー
        if (comboTimer_ > 0 
            && justAvoidPlayer_->GetJustAvoidKey() == JustAvoidPlayer::JUST_AVOID_KEY::NULL_KEY)
        {
            comboTimer_ -= elapsedTime;
            if (comboTimer_ <= 0)
            {
                comboTimer_ = 0;
                hitComboCount_ = 0;
            }
        }
    }

    //アニメーションイベントでSE
    PlayAnimationSE();
}

// GUI描画
void PlayerCom::OnGUI()
{
    int currentAnimIndex = GetGameObject()->GetComponent<AnimationCom>()->GetCurrentAnimationIndex();

    ImGui::InputInt("currentAnimIndex", &currentAnimIndex);

    //入力情報を取得
    GamePad& gamePad = Input::Instance().GetGamePad();
    float ax = gamePad.GetAxisLX();
    float ay = gamePad.GetAxisLY();
    ImGui::DragFloat("x", &ax);
    ImGui::DragFloat("y", &ay);

    int stats = (int)playerStatus_;
    ImGui::InputInt("status", &stats);

    if(ImGui::TreeNode("Attack"))
    {
        attackPlayer_->OnGui();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("JustAvo"))
    {
        justAvoidPlayer_->OnGui();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Move"))
    {
        movePlayer_->OnGui();
        ImGui::TreePop();
    }

    ImGui::DragInt("combo", &hitComboCount_);
    ImGui::DragFloat("hitComboDirSpeed_", &hitComboDirSpeed_,0.1f);
    ImGui::DragFloat4("pos", &hitComboNumPos_.x);
    ImGui::DragFloat4("hitComboNumC_", &hitComboNumC_.x);

    ImGui::DragFloat3("CCpos", &comboPos_.x);
    ImGui::DragFloat2("comboMaskSize_", &comboMaskOffsetSize_.x,0.01f);

    ////ボタン配置用
    //ImGui::DragFloat2("buttonPos_", &buttonPos_.x);
    //for (int i = 0; i < 5; ++i)
    //{
    //    std::string s = "offsetPos_" + std::to_string(i);
    //    ImGui::DragFloat2(s.c_str(), &offsetButtonPos_[i].x);
    //}
}

void PlayerCom::Render2D(float elapsedTime)
{
    attackPlayer_->Render2D(elapsedTime);
    justAvoidPlayer_->Render2D(elapsedTime);

    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

    //ボタン
    {
        //特殊攻撃の切り替え
        std::shared_ptr<MovementCom> move = GetGameObject()->GetComponent<MovementCom>();
        Sprite* Y = YSprite_.get();
        Sprite* YString = attackJumpStringSprite_.get();
        if (attackPlayer_->GetComboSquareCount() > 0
            && playerStatus_ != PLAYER_STATUS::ATTACK_DASH
            && playerStatus_ != PLAYER_STATUS::ATTACK_JUMP_FALL
            && move->OnGround())
        {
            Y = YTuyoSprite_.get();
            YString = attackRangeStringSprite_.get();
        }

        //ジャスト攻撃の切り替え
        Sprite* X = XSprite_.get();
        Sprite* XString = attackStringSprite_.get();

        //ジャスト回避成功時
        if (justAvoidPlayer_->GetIsJustJudge())
        {
            YString = sniperStringSprite_.get();
            XString = slowStringSprite_.get();
        }

        Sprite* s[5] =
        {
            X,
            Y,
            ASprite_.get(),
            BSprite_.get(),
            RTSprite_.get()
        };
        Sprite* sString[5] =
        {
            XString,
            YString,
            jumpStringSprite_.get(),
            nullptr,
            dashStringSprite_.get()
        };
        //スナイパージャスト回避中は表示を制限
        if (justAvoidPlayer_->GetJustAvoidKey() == JustAvoidPlayer::JUST_AVOID_KEY::TRIANGLE)
        {
            for (int i = 0; i < 5; ++i)
            {
                //Y以外はnullに
                if (i == 1)continue;
                sString[i] = nullptr;
            }
        }

        for (int i = 0; i < 5; ++i)
        {
            float size = 50 + buttonSize_[i];
            s[i]->Render(dc, buttonPos_.x + offsetButtonPos_[i].x - size / 2.0f, buttonPos_.y + offsetButtonPos_[i].y - size / 2.0f
                , size, size
                , 0, 0, static_cast<float>(s[i]->GetTextureWidth()), static_cast<float>(s[i]->GetTextureHeight())
                , 0, 1, 1, 1, 1);

            //文字
            if (sString[i])
            {
                float stringRatio = 0.1f;   //比率で大きさを決める
                float xSize = static_cast<float>(sString[i]->GetTextureWidth()) * stringRatio;
                float ySize = static_cast<float>(sString[i]->GetTextureHeight()) * stringRatio;

                float x = buttonPos_.x + offsetButtonPos_[i].x - xSize / 2.0f;
                if (spriteRightPost_[i])x += xSize / 2 + 50;
                else x -= xSize / 2 + 50;

                sString[i]->Render(dc, x, buttonPos_.y + offsetButtonPos_[i].y - ySize / 2.0f
                    , xSize, ySize
                    , 0, 0, static_cast<float>(sString[i]->GetTextureWidth()), static_cast<float>(sString[i]->GetTextureHeight())
                    , 0, 1, 1, 1, 1);
            }
        }
    }


    //ヒットコンボカウント
    {
        //数字
        DirectX::XMFLOAT2 size = {
            static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f * hitComboNumPos_.z ,
            static_cast<float>(numSprite_->GetTextureHeight()) * hitComboNumPos_.z
        };
        //１桁
        numSprite_->Render(dc, hitComboNumPos_.x - (size.x * 0.8f), hitComboNumPos_.y - (size.y * 0.5f), size.x, size.y
            , 170.0f * hitComboCount_, 0, static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f, static_cast<float>(numSprite_->GetTextureHeight())
            , 0, hitComboNumC_.x, hitComboNumC_.y, hitComboNumC_.z, hitComboNumC_.w);
        //２桁
        if (hitComboCount_ >= 10)
        {
            int keta = hitComboCount_ / 10 % 10;
            numSprite_->Render(dc, hitComboNumPos_.x - hitComboNumPos_.w - (size.x * 0.8f), hitComboNumPos_.y - (size.y * 0.5f), size.x, size.y
                , 170.0f * keta, 0, static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f, static_cast<float>(numSprite_->GetTextureHeight())
                , 0, hitComboNumC_.x, hitComboNumC_.y, hitComboNumC_.z, hitComboNumC_.w);
        }
        //３桁
        if (hitComboCount_ >= 100)
        {
            int keta = hitComboCount_ / 100 % 10;
            numSprite_->Render(dc, hitComboNumPos_.x - hitComboNumPos_.w * 2 - (size.x * 0.8f), hitComboNumPos_.y - (size.y * 0.5f), size.x, size.y
                , 170.0f * keta, 0, static_cast<float>(numSprite_->GetTextureWidth()) * 0.1f, static_cast<float>(numSprite_->GetTextureHeight())
                , 0, hitComboNumC_.x, hitComboNumC_.y, hitComboNumC_.z, hitComboNumC_.w);
        }
    }

}

void PlayerCom::AudioRelease()
{
    damageSE_->AudioRelease();
    for (auto& se : animSE)
    {
        se.SE->AudioRelease();
    }
    attackPlayer_->AudioRelease();
    movePlayer_->AudioRelease();
    justAvoidPlayer_->AudioRelease();
}


void PlayerCom::MaskRender(PostEffect* postEff, std::shared_ptr<CameraCom> maskCamera)
{
    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

    //仮処理
    if (!startUI_)
    {
        startUI_ = true;
        std::shared_ptr<FbxModelResource> res = GameObjectManager::Instance().Find("picoMask")->GetComponent<RendererCom>()->GetModel()->GetResourceShared();
        for (auto& shape : res->GetMeshesEdit()[res->GetShapeIndex()].shapeData)
        {
            shape.rate = 0;
        }
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[5].rate = 1;
        res->GetMeshesEdit()[res->GetShapeIndex()].shapeData[9].rate = 1;
        GameObjectManager::Instance().Find("picoMask")->GetComponent<AnimationCom>()
            ->PlayAnimation(ANIMATION_PLAYER::IDEL_2, true);

    }

    //皿
    saraSprite_->Render(dc, -101.8f, -62.6f, static_cast<float>(saraSprite_->GetTextureWidth()), static_cast<float>(saraSprite_->GetTextureHeight())
        , 0, 0, static_cast<float>(saraSprite_->GetTextureWidth()), static_cast<float>(saraSprite_->GetTextureHeight())
        , 0, 1, 1, 1, 1);

    //HP
    {
        int hpIndex = GetGameObject()->GetComponent<CharacterStatusCom>()->GetHP() - 1;
        if (hpIndex > 0) {
            hpSprite_[hpIndex]->Render(dc, 1.9f, -9.1f, static_cast<float>(hpSprite_[hpIndex]->GetTextureWidth()) * 0.3f, static_cast<float>(hpSprite_[hpIndex]->GetTextureHeight()) * 0.3f
                , 0, 0, static_cast<float>(hpSprite_[hpIndex]->GetTextureWidth()), static_cast<float>(hpSprite_[hpIndex]->GetTextureHeight())
                , 0, 1, 1, 1, 1);
        }
        if (hpIndex >= 0) {
            hpSprite_[0]->Render(dc, hpDonutsPos_[hpIndex].x, hpDonutsPos_[hpIndex].y, static_cast<float>(hpSprite_[0]->GetTextureWidth()) * 0.3f, static_cast<float>(hpSprite_[0]->GetTextureHeight()) * 0.3f
                , 0, 0, static_cast<float>(hpSprite_[0]->GetTextureWidth()), static_cast<float>(hpSprite_[0]->GetTextureHeight())
                , 0, 1, 1, 1, 1);
        }
        //演出
        static int oldHp_ = hpIndex;
        if (oldHp_ != hpIndex)
        {
            isHpDirection_ = true;
            hpDir_.x = hpDonutsPos_[oldHp_].x;
            hpDir_.y = hpDonutsPos_[oldHp_].y;
            hpDir_.z = 1;
            hpGravity_ = 0;
            oldHp_ = hpIndex;

            //画像の重心（下の方に置く）を算出
            static DirectX::XMVECTOR Point = { 1.9f + hpSprite_[1]->GetTextureWidth() / 2 * 0.3f
            ,-9.1f + hpSprite_[1]->GetTextureHeight() * 0.3f };
            //重心からのベクトルを移動方向にする
            DirectX::XMVECTOR HpPoint = { hpDir_.x + hpSprite_[0]->GetTextureWidth() / 2 * 0.3f
                ,hpDir_.y + hpSprite_[0]->GetTextureHeight() / 2 * 0.3f };
            DirectX::XMStoreFloat2(&dirVelo_, DirectX::XMVector2Normalize(DirectX::XMVectorSubtract(HpPoint, Point)));
        }
        if (isHpDirection_)
        {
            hpSprite_[0]->Render(dc, hpDir_.x, hpDir_.y, static_cast<float>(hpSprite_[0]->GetTextureWidth()) * 0.3f, static_cast<float>(hpSprite_[0]->GetTextureHeight()) * 0.3f
                , 0, 0, static_cast<float>(hpSprite_[0]->GetTextureWidth()), static_cast<float>(hpSprite_[0]->GetTextureHeight())
                , 0, 1, 1, 1, hpDir_.z);
        }
    }

    //ワイプ枠外
    faceFrameUI_->Render(dc, 49.5f, 41.4f, static_cast<float>(faceFrameUI_->GetTextureWidth()) * 0.552f, static_cast<float>(faceFrameUI_->GetTextureHeight()) * 0.552f
        , 0, 0, static_cast<float>(faceFrameUI_->GetTextureWidth()), static_cast<float>(faceFrameUI_->GetTextureHeight())
        , 0, 0, 1, 1, 1);

    //ワイプ
    {
        //マスクする側描画
        postEff->CacheMaskBuffer(maskCamera);

        //ワイプ背景
        faceFrameUI_->Render(dc, 49.5f + 9, 41.4f + 9, static_cast<float>(faceFrameUI_->GetTextureWidth()) * 0.48f, static_cast<float>(faceFrameUI_->GetTextureHeight()) * 0.48f
            , 0, 0, static_cast<float>(faceFrameUI_->GetTextureWidth()), static_cast<float>(faceFrameUI_->GetTextureHeight())
            , 0, 1, 1, 1, 1);

        //マスクされる側描画
        postEff->StartBeMaskBuffer();


        //マスクオブジェ描画
        GameObjectManager::Instance().RenderMask();


        //マスク処理終了処理
        postEff->RestoreMaskBuffer({ -154 ,-72 }, { 0.3f,0.3f });

        postEff->DrawMask();
    }

    DirectX::XMFLOAT2 comboSize = { static_cast<float>(comboBackSprite_->GetTextureWidth()) * comboPos_.z ,static_cast<float>(comboBackSprite_->GetTextureHeight()) * comboPos_.z };

    //コンボ背景
    comboBackSprite_->Render(dc, comboPos_.x, comboPos_.y
        , comboSize.x, comboSize.y
        , 0, 0, static_cast<float>(comboBackSprite_->GetTextureWidth()), static_cast<float>(comboBackSprite_->GetTextureHeight())
        , 0, 1, 1, 1, 1);

    //コンボ文字
    {
        //マスクする側描画
        postEff->CacheMaskBuffer(maskCamera);

        //コンボマスク用画像
        comboMaskSprite_->Render(dc, comboPos_.x + comboSize.x, (comboPos_.y + comboSize.y)* comboMaskOffsetSize_.x
            , comboSize.x, comboSize.y * comboMaskOffsetSize_.y* (comboTimer_/ comboTime_)
            , 0, 0, static_cast<float>(comboMaskSprite_->GetTextureWidth()), static_cast<float>(comboMaskSprite_->GetTextureHeight())
            , 180, 1, 1, 1, 0.001f);

        //マスクされる側描画
        postEff->StartBeMaskBuffer();

        //コンボ文字
        comboSprite_->Render(dc, comboPos_.x, comboPos_.y
            , comboSize.x, comboSize.y
            , 0, 0, static_cast<float>(comboSprite_->GetTextureWidth()), static_cast<float>(comboSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);

        //マスク処理終了処理
        postEff->RestoreMaskBuffer();

        postEff->DrawMask();
    }
}

//アニメーション初期化設定
void PlayerCom::AnimationInitialize()
{
    //アニメーター
    std::shared_ptr<AnimatorCom> animator = GetGameObject()->GetComponent<AnimatorCom>();
    //初期のアニメーション
    animator->SetFirstTransition(ANIMATION_PLAYER::IDEL_2);
    animator->SetLoopAnimation(ANIMATION_PLAYER::IDEL_2, true);

    //アニメーションパラメーター追加
    //トリガーで判定
    {
        animator->AddTriggerParameter("idle");
        animator->AddTriggerParameter("jump");
        animator->AddTriggerParameter("jumpFall");
        animator->AddTriggerParameter("dash");
        animator->AddTriggerParameter("dashBack");
        animator->AddTriggerParameter("runTurn");
        animator->AddTriggerParameter("runStop");

        animator->AddTriggerParameter("damageFront");

        animator->AddTriggerParameter("square");    //□
        animator->AddTriggerParameter("squareIdle");
        animator->AddTriggerParameter("squareDash");
        animator->AddTriggerParameter("squareJump");

        animator->AddTriggerParameter("triangle");  //△
        animator->AddTriggerParameter("triangleDash");
        animator->AddTriggerParameter("triangleJump");
        animator->AddTriggerParameter("triangleJumpDown");

        //ジャスト時
        animator->AddTriggerParameter("squareJust");
        animator->AddTriggerParameter("squareJustSky");
        animator->AddTriggerParameter("triangleJust");
        //ジャスト回避
        animator->AddTriggerParameter("justBack");
        animator->AddTriggerParameter("justFront");
        animator->AddTriggerParameter("justLeft");
        animator->AddTriggerParameter("justRight");
    }

    //フロートで判定
    {
        animator->AddFloatParameter("moveSpeed");
    }

    //アニメーション遷移とパラメーター設定を決める
    {
        //idle
        animator->AddAnimatorTransition(IDEL_2, WALK_RUNRUN_2, false, 0.5f);
        animator->SetFloatTransition(IDEL_2, WALK_RUNRUN_2,
            "moveSpeed", 0.1f, PARAMETER_JUDGE::GREATER);

        //walk
        animator->AddAnimatorTransition(WALK_RUNRUN_2, IDEL_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, IDEL_2,
            "moveSpeed", 0.1f, PARAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(WALK_RUNRUN_2, true);

        animator->AddAnimatorTransition(WALK_RUNRUN_2, RUN_HARD_2);
        animator->SetFloatTransition(WALK_RUNRUN_2, RUN_HARD_2,
            "moveSpeed", movePlayer_->GetMoveParam(MovePlayer::MOVE_PARAM::WALK).moveMaxSpeed + 1, PARAMETER_JUDGE::GREATER);

        //run
        animator->AddAnimatorTransition(RUN_HARD_2, WALK_RUNRUN_2);
        animator->SetFloatTransition(RUN_HARD_2, WALK_RUNRUN_2,
            "moveSpeed", movePlayer_->GetMoveParam(MovePlayer::MOVE_PARAM::WALK).moveMaxSpeed + 1, PARAMETER_JUDGE::LESS);
        animator->SetLoopAnimation(RUN_HARD_2, true);

        //run切り替えし
        animator->AddAnimatorTransition(RUN_HARD_2, RUN_TURN_FORWARD, false, 0.0f);
        animator->SetTriggerTransition(RUN_HARD_2, RUN_TURN_FORWARD, "runTurn");
        animator->AddAnimatorTransition(RUN_TURN_FORWARD, RUN_HARD_2, true);

        //run止まり
        animator->AddAnimatorTransition(RUN_STOP);
        animator->SetTriggerTransition(RUN_STOP, "runStop");
        animator->AddAnimatorTransition(RUN_STOP, IDEL_2, true);

        //jump
        animator->AddAnimatorTransition(JUMP_IN);
        animator->SetTriggerTransition(JUMP_IN, "jump");
        animator->AddAnimatorTransition(JUMP_IN, IDEL_2);
        animator->SetTriggerTransition(JUMP_IN, IDEL_2, "idle");

        //jumpFall
        animator->AddAnimatorTransition(JUMP_FALL, false, 1.0f);
        animator->SetTriggerTransition(JUMP_FALL, "jumpFall");
        animator->AddAnimatorTransition(JUMP_FALL, IDEL_2, false, 0.5f);
        animator->SetTriggerTransition(JUMP_FALL, IDEL_2, "idle");
        animator->SetLoopAnimation(JUMP_FALL, true);

        //dash
        animator->AddAnimatorTransition(DASH_ANIM);
        animator->SetTriggerTransition(DASH_ANIM, "dash");
        animator->AddAnimatorTransition(DASH_ANIM, IDEL_2, true);

        //dash_back
        animator->AddAnimatorTransition(DASH_BACK);
        animator->SetTriggerTransition(DASH_BACK, "dashBack");
        animator->AddAnimatorTransition(DASH_BACK, IDEL_2);
        animator->SetTriggerTransition(DASH_BACK, IDEL_2, "idle");

        //ジャスト回避回避
        {
            //back
            animator->AddAnimatorTransition(DODGE_BACK);
            animator->SetTriggerTransition(DODGE_BACK, "justBack");
            animator->AddAnimatorTransition(DODGE_BACK, IDEL_2, true);
            //front
            animator->AddAnimatorTransition(DODGE_FRONT);
            animator->SetTriggerTransition(DODGE_FRONT, "justFront");
            animator->AddAnimatorTransition(DODGE_FRONT, IDEL_2, true,0.5f);
            //left
            animator->AddAnimatorTransition(DODGE_LEFT);
            animator->SetTriggerTransition(DODGE_LEFT, "justLeft");
            animator->AddAnimatorTransition(DODGE_LEFT, IDEL_2, true);
            //right
            animator->AddAnimatorTransition(DODGE_RIGHT);
            animator->SetTriggerTransition(DODGE_RIGHT, "justRight");
            animator->AddAnimatorTransition(DODGE_RIGHT, IDEL_2, true);
        }

        {   //中距離コンボ
            //コンボ2
            animator->AddAnimatorTransition(BIGSWORD_DASH);
            animator->SetTriggerTransition(BIGSWORD_DASH, "triangleDash");
            animator->AddAnimatorTransition(BIGSWORD_DASH, IDEL_2, true, 3.5f);

            //コンボ3
            animator->AddAnimatorTransition(BIGSWORD_DASH, BIGSWORD_COM2_02);
            animator->SetTriggerTransition(BIGSWORD_DASH, BIGSWORD_COM2_02, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM2_02, IDEL_2, true, 3.5f);
        }

        //攻撃
        {  
            //□
            //combo01
            animator->AddAnimatorTransition(BIGSWORD_COM1_01);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, "squareIdle");
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, IDEL_2, true, 3.5f);

            //combo2
            //□ (前□）
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, BIGSWORD_COM1_02, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, IDEL_2, true, 3.5f);

            //□ (前△）
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, BIGSWORD_COM1_02);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_01, BIGSWORD_COM1_02, "square");

            //△
            animator->AddAnimatorTransition(BIGSWORD_COM1_01, TRIANGLE_ATTACK_01, false, 1.0f);
            animator->SetTriggerTransition(BIGSWORD_COM1_01, TRIANGLE_ATTACK_01, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, IDEL_2, true, 3.5f);

            //combo3
            //□ (前□)
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, BIGSWORD_COM1_03, "square");
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, IDEL_2, true, 3.5f);

            //□ (前△)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, BIGSWORD_COM1_03);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_02, BIGSWORD_COM1_03, "square");

            //△ (前□)
            animator->AddAnimatorTransition(BIGSWORD_COM1_02, TRIANGLE_ATTACK_02, false, 1.0f);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, TRIANGLE_ATTACK_02, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, IDEL_2, true, 3.5f);

            //△ (前△)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_01, TRIANGLE_ATTACK_02, false, 1.0f);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_01, TRIANGLE_ATTACK_02, "triangle");

            //最後△
            //△ (前□)
            animator->AddAnimatorTransition(BIGSWORD_COM1_03, TRIANGLE_ATTACK_03);
            animator->SetTriggerTransition(BIGSWORD_COM1_03, TRIANGLE_ATTACK_03, "triangle");
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_03, IDEL_2, true, 3.5f);

            //△ (前△)
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_02, TRIANGLE_ATTACK_03);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_02, TRIANGLE_ATTACK_03, "triangle");


            //ジャスト回避時□
            animator->AddAnimatorTransition(BIGSWORD_COM1_02);
            animator->SetTriggerTransition(BIGSWORD_COM1_02, "squareJust");

            //ジャスト回避時△
            animator->AddAnimatorTransition(TRIANGLE_ATTACK_PUSH);
            animator->SetTriggerTransition(TRIANGLE_ATTACK_PUSH, "triangleJust");
        }

        //ジャンプコンボ
        {   //J△□□
            //combo01
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER);
            animator->SetTriggerTransition(JUMP_ATTACK_UPPER, "triangleJump");
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER, IDEL_2, true, 3.5f);

            //combo02
            animator->AddAnimatorTransition(JUMP_ATTACK_UPPER, JUMP_ATTACK_01);
            animator->SetTriggerTransition(JUMP_ATTACK_UPPER, JUMP_ATTACK_01, "square");
        }

        //ジャンプ中、下強攻撃
        {   //J△
            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_START);
            animator->SetTriggerTransition(JUMP_ATTACK_DOWN_START, "triangleJumpDown");
            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_START, JUMP_ATTACK_DOWN_DO, true);

            animator->SetLoopAnimation(JUMP_ATTACK_DOWN_DO, true);

            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_DO, JUMP_ATTACK_DOWN_END);
            animator->SetTriggerTransition(JUMP_ATTACK_DOWN_DO, JUMP_ATTACK_DOWN_END, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_DOWN_END, IDEL_2, true);
        }

        //ジャンプ攻撃
        {
            //ジャスト空中回避時□
            animator->AddAnimatorTransition(JUMP_ATTACK_06);
            animator->SetTriggerTransition(JUMP_ATTACK_06, "squareJustSky");

            animator->AddAnimatorTransition(JUMP_ATTACK_01);
            animator->SetTriggerTransition(JUMP_ATTACK_01, "squareJump");
            animator->AddAnimatorTransition(JUMP_ATTACK_01, IDEL_2);
            animator->SetTriggerTransition(JUMP_ATTACK_01, IDEL_2, "idle");

            //ジャストからのコンボ
            animator->AddAnimatorTransition(JUMP_ATTACK_06, JUMP_ATTACK_01);
            animator->SetTriggerTransition(JUMP_ATTACK_06, JUMP_ATTACK_01, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_06, IDEL_2,true);
            animator->SetTriggerTransition(JUMP_ATTACK_06, IDEL_2, "idle");


            animator->AddAnimatorTransition(JUMP_ATTACK_01, JUMP_ATTACK_02);
            animator->SetTriggerTransition(JUMP_ATTACK_01, JUMP_ATTACK_02, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_02, IDEL_2);
            animator->SetTriggerTransition(JUMP_ATTACK_02, IDEL_2, "idle");

            animator->AddAnimatorTransition(JUMP_ATTACK_02, JUMP_ATTACK_03);
            animator->SetTriggerTransition(JUMP_ATTACK_02, JUMP_ATTACK_03, "square");
            animator->AddAnimatorTransition(JUMP_ATTACK_03, IDEL_2);
            animator->SetTriggerTransition(JUMP_ATTACK_03, IDEL_2, "idle");

        }

        //ダメージ
        {
            animator->AddAnimatorTransition(DAMAGE_FRONT);
            animator->SetTriggerTransition(DAMAGE_FRONT, "damageFront");
            animator->AddAnimatorTransition(DAMAGE_FRONT, IDEL_2, true);
        }
    }
}

//ラジアルブラーの設定
void PlayerCom::BlurStartPlayer(float power, float time, std::string boneName, DirectX::XMFLOAT2 pos, std::shared_ptr<GameObject> posObj)
{
    ShaderParameter3D& sp = Graphics::Instance().shaderParameter3D_;

    blurPosObj_.lock().reset();
    if (posObj)
    {
        blurPosObj_ = posObj;
    }

    blurBoneName_ = boneName;
    sp.radialBlur.blurPos = pos;
    blurPower_ = power;
    blurTime_ = time;
    blurTimer_ = 0;
}

void PlayerCom::VignetteStart(float power, float time)
{
    Vignette& vignette = Graphics::Instance().shaderParameter3D_.vignette;
    vignette.enabled = 1;
    vignetteDamageTime_ = time;
    vignetteDamageTimer_ = time;
    vignetteDamagePower_ = power;
}

void PlayerCom::AddHitCount()
{
    hitComboCount_++;
    hitComboNumPos_.z = hitComboDirSize_;
    comboTimer_ = comboTime_;

    //スコア
    Score::Instance().AddScore(123456);
    int maxCombo = Score::Instance().maxCombo_;
    if (maxCombo < hitComboCount_)
        Score::Instance().maxCombo_ = hitComboCount_;
}

void PlayerCom::PlayAnimationSE()
{
    std::shared_ptr<AnimationCom> anim = GetGameObject()->GetComponent<AnimationCom>();

    for (auto& se : animSE)
    {
        if (se.isPlay && se.saveAnimIndex > 0)
        {
            if (anim->GetCurrentAnimationIndex() != se.saveAnimIndex)
            {
                se.saveAnimIndex = -1;
                se.isPlay = false;
            }
            continue;
        }
        if (anim->GetCurrentAnimationEvent(se.animEventName.c_str(), DirectX::XMFLOAT3()))
        {
            se.isPlay = true;
            se.saveAnimIndex = anim->GetCurrentAnimationIndex();
            se.SE->Stop();
            se.SE->Play(false, se.volumeSE);
        }
    }
}
