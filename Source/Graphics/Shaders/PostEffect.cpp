#include "PostEffect.h"

#include "Graphics\Graphics.h"
#include <imgui.h>

#include "Components\System\GameObject.h"
#include "Components\CameraCom.h"
#include "Components\TransformCom.h"

//�K�E�X�u���[
void CalcGaussianFilter(DirectX::XMFLOAT4* array, int karnel_size, float sigma) {
    float sum = 0.0f;
    int id = 0;
    for (int y = -karnel_size / 2; y <= karnel_size / 2; y++) {
        for (int x = -karnel_size / 2; x <= karnel_size / 2; x++) {
            array[id].x = (float)x;
            array[id].y = (float)y;
            float a = 1.0f / (2.0f * PI_ANGLE * sigma * sigma);
            float b = expf(-(x * x + y * y) / (2.0f * sigma * sigma));
            array[id].z = a * b;
            sum += array[id].z;
            id++;
        }
    }
    // �S�̂ō��v�P�ɂȂ�悤�ɐ��K��
    for (int i = 0; i < karnel_size * karnel_size; i++)
        array[i].z /= sum;
}

PostEffect::PostEffect(UINT width, UINT height)
{
    lightDirMask_ = { 0,0,1,1 };

    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();
    ID3D11Device* device = Graphics::Instance().GetDevice();
    //�o�b�t�@�쐬
    dx11State->createConstantBuffer(device, sizeof(ShaderParameter3D::colorGradingData), colorGradingBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(ShaderParameter3D::bloomBlur), bloomBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(ShaderParameter3D::bloomLuminance), bloomExBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(ShaderParameter3D::sunAtmosphere), sunBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(ShaderParameter3D::radialBlur), radialBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(ShaderParameter3D::vignette), vignetteBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(CbScene), sceneBuffer_.GetAddressOf());
    dx11State->createConstantBuffer(device, sizeof(SkymapData), skymapBuffer_.GetAddressOf());

    //�s�N�Z���V�F�[�_�[�Z�b�g
    colorGrading_ = std::make_unique<ShaderPost>("ColorGrading");
    bloomExtract_ = std::make_unique<ShaderPost>("BloomExtract");
    bloomKawaseFilter_ = std::make_unique<ShaderPost>("BloomKawase");
    bloomBlur_ = std::make_unique<ShaderPost>("GaussianBlur");
    sun_ = std::make_unique<ShaderPost>("SunAtmosphere");
    radial_ = std::make_unique<ShaderPost>("RadialBlur");
    mask_ = std::make_unique<ShaderPost>("MaskPost");

    {
        //�X�J�C�}�b�v
        D3D11_TEXTURE2D_DESC texture2d_desc{};
        texture2d_desc.Width = 1280;
        texture2d_desc.Height = 720;
        texture2d_desc.MipLevels = 1;
        texture2d_desc.ArraySize = 1;
        texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        texture2d_desc.SampleDesc.Count = 1;
        texture2d_desc.SampleDesc.Quality = 0;
        texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
        texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        texture2d_desc.CPUAccessFlags = 0;
        texture2d_desc.MiscFlags = 0;

        dx11State->load_texture_from_file(device, "Data/Sprite/sky.dds", skymapSrv_.GetAddressOf(), &texture2d_desc);

        dx11State->createVsFromCso(device, "Shader\\FullScreenVS.cso", skyVertex_.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
        dx11State->createPsFromCso(device, "Shader\\Skymap.cso", skyPixel_.ReleaseAndGetAddressOf());

        postEffect_ = std::make_unique<ShaderPost>("PostEffect");
        postRender_ = std::make_unique<ShaderPost>("RenderPost");
    }

    //�e�N�X�`��������
    drawTexture_ = std::make_unique<TextureFormat>();

    //�����_�[�^�[�Q�b�g�ݒ�
    //�P�x���o�p
    renderPost_[0] = std::make_unique<PostRenderTarget>(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
    //�u���[�p
    float string = 0.7f;
    UINT wB = width;
    UINT hB = height;
    for (int i = 0; i < BlurCount; ++i)
    {
        renderPost_[i + 1] = std::make_unique<PostRenderTarget>(device, wB, hB, DXGI_FORMAT_R16G16B16A16_FLOAT);
        wB = static_cast<UINT>(wB*string);
        hB = static_cast<UINT>(hB*string);
    }

    //���z
    renderPostSun_ = std::make_unique<PostRenderTarget>(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);

    //���ː��u���[
    renderPostRadial_ = std::make_unique<PostRenderTarget>(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);

    //�t���X�N���[���p
    renderPostFull_ = std::make_unique<PostRenderTarget>(device,
        static_cast<UINT>(Graphics::Instance().GetScreenWidth()),
        static_cast<UINT>(Graphics::Instance().GetScreenHeight()), DXGI_FORMAT_R16G16B16A16_FLOAT);

    //2DMask
    for (int i = 0; i < 3; ++i)
    {
        renderPost2D_[i] = std::make_unique<PostRenderTarget>(device,
            static_cast<UINT>(Graphics::Instance().GetScreenWidth()),
            static_cast<UINT>(Graphics::Instance().GetScreenHeight()), DXGI_FORMAT_R16G16B16A16_FLOAT);
    }
    depthPost2D_ = std::make_unique<PostDepthStencil>(device,
        static_cast<UINT>(Graphics::Instance().GetScreenWidth()),
        static_cast<UINT>(Graphics::Instance().GetScreenHeight()));
}

//�`��
void PostEffect::Render(std::shared_ptr<CameraCom> camera)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    //�o�b�t�@���
    graphics.CacheRenderTargets();



#pragma region ���z
    if (sun_->IsEnabled())
    {
        //�`����ύX
        ID3D11RenderTargetView* rtv = renderPostSun_->renderTargetView.Get();
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(rtv, color);
        dc->OMSetRenderTargets(1, &rtv, nullptr);
        D3D11_VIEWPORT	viewport{};
        viewport.Width = static_cast<float>(renderPostSun_->width);
        viewport.Height = static_cast<float>(renderPostSun_->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &viewport);

        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        PostRenderTarget* ps = graphics.GetPostEffectModelRenderTarget().get();
        drawTexture_->SetShaderResourceView(
            ps->diffuseMap, ps->width, ps->height);

        drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
            0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
            0,
            1, 1, 1, 1);

        //�[�x��񑗂�
        PostDepthStencil* ds = graphics.GetPostEffectModelDepthStencilView().get();
        dc->PSSetShaderResources(1, 1, ds->diffuseMap.GetAddressOf());

        //�L���ɂ���
        graphics.shaderParameter3D_.sunAtmosphere.enabled = 1;

        //���W����
        graphics.shaderParameter3D_.sunAtmosphere.view = camera->GetView();

        DirectX::XMStoreFloat4x4(&graphics.shaderParameter3D_.sunAtmosphere.inverseProjection,
            DirectX::XMMatrixInverse(nullptr,
               DirectX::XMLoadFloat4x4(&camera->GetProjection())));

        DirectX::XMStoreFloat4x4(&graphics.shaderParameter3D_.sunAtmosphere.inverseViewProjection,
            DirectX::XMMatrixInverse(nullptr,
                DirectX::XMLoadFloat4x4(&camera->GetView())
                * DirectX::XMLoadFloat4x4(&camera->GetProjection())));

        //�V�[�����
        CbScene cb = {};
        cb.lightColor = graphics.shaderParameter3D_.lightColor;
        cb.lightDirection = graphics.shaderParameter3D_.lightDirection;
        DirectX::XMFLOAT3 cP = camera->GetGameObject()->transform_->GetWorldPosition();
        cb.viewPosition = { cP.x,cP.y,cP.z,1 };
        DirectX::XMStoreFloat4x4(&cb.viewProjection,
                DirectX::XMLoadFloat4x4(&camera->GetView())
                * DirectX::XMLoadFloat4x4(&camera->GetProjection()));


        dc->UpdateSubresource(sunBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.sunAtmosphere, 0, 0);
        dc->UpdateSubresource(sceneBuffer_.Get(), 0, NULL, &cb, 0, 0);
        dc->PSSetConstantBuffers(0, 1, sunBuffer_.GetAddressOf());
        dc->PSSetConstantBuffers(1, 1, sceneBuffer_.GetAddressOf());


        sun_->Draw(drawTexture_.get());
    }
    else
    {
        //�`����ύX
        ID3D11RenderTargetView* rtv = renderPostSun_->renderTargetView.Get();
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(rtv, color);
        dc->OMSetRenderTargets(1, &rtv, nullptr);
        D3D11_VIEWPORT	viewport{};
        viewport.Width = static_cast<float>(renderPostSun_->width);
        viewport.Height = static_cast<float>(renderPostSun_->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &viewport);

        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        PostRenderTarget* ps = graphics.GetPostEffectModelRenderTarget().get();
        drawTexture_->SetShaderResourceView(
            ps->diffuseMap, ps->width, ps->height);

        drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
            0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
            0,
            1, 1, 1, 1);

        //�[�x��񑗂�
        PostDepthStencil* ds = graphics.GetPostEffectModelDepthStencilView().get();
        dc->PSSetShaderResources(1, 1, ds->diffuseMap.GetAddressOf());

        //�L���ɂ���
        graphics.shaderParameter3D_.sunAtmosphere.enabled = -1;
        dc->UpdateSubresource(sunBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.sunAtmosphere, 0, 0);
        dc->PSSetConstantBuffers(0, 1, sunBuffer_.GetAddressOf());

        sun_->Draw(drawTexture_.get());
    }
#pragma endregion

#pragma region ���ː��u���[
    if (radial_->IsEnabled())
    {
        //�`����ύX
        ID3D11RenderTargetView* rtv = renderPostRadial_->renderTargetView.Get();
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(rtv, color);
        dc->OMSetRenderTargets(1, &rtv, nullptr);
        D3D11_VIEWPORT	viewport{};
        viewport.Width = static_cast<float>(renderPostRadial_->width);
        viewport.Height = static_cast<float>(renderPostRadial_->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &viewport);

        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        drawTexture_->SetShaderResourceView(
            renderPostSun_->diffuseMap, renderPostSun_->width, renderPostSun_->height);

        drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
            0, 0, static_cast<float>(renderPostSun_->width), static_cast<float>(renderPostSun_->height),
            0,
            1, 1, 1, 1);

        //�L���ɂ���
        graphics.shaderParameter3D_.radialBlur.enabled = 1;


        dc->UpdateSubresource(radialBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.radialBlur, 0, 0);
        dc->PSSetConstantBuffers(0, 1, radialBuffer_.GetAddressOf());


        radial_->Draw(drawTexture_.get());
    }
    else
    {
        //�`����ύX
        ID3D11RenderTargetView* rtv = renderPostRadial_->renderTargetView.Get();
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(rtv, color);
        dc->OMSetRenderTargets(1, &rtv, nullptr);
        D3D11_VIEWPORT	viewport{};
        viewport.Width = static_cast<float>(renderPostRadial_->width);
        viewport.Height = static_cast<float>(renderPostRadial_->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &viewport);

        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        drawTexture_->SetShaderResourceView(
            renderPostSun_->diffuseMap, renderPostSun_->width, renderPostSun_->height);

        drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
            0, 0, static_cast<float>(renderPostSun_->width), static_cast<float>(renderPostSun_->height),
            0,
            1, 1, 1, 1);

        //�L���ɂ���
        graphics.shaderParameter3D_.radialBlur.enabled = -1;


        //�L���ɂ���
        dc->UpdateSubresource(radialBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.radialBlur, 0, 0);
        dc->PSSetConstantBuffers(0, 1, radialBuffer_.GetAddressOf());

        radial_->Draw(drawTexture_.get());
    }
#pragma endregion


#pragma region �u���[��
    if(bloomExtract_->IsEnabled() && !bloomKawaseFilter_->IsEnabled())
    {
        //���P�x���o�p�o�b�t�@
        {
            //�`����ύX
            ID3D11RenderTargetView* rtv = renderPost_[0]->renderTargetView.Get();
            FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            dc->ClearRenderTargetView(rtv, color);
            dc->OMSetRenderTargets(1, &rtv, nullptr);
            D3D11_VIEWPORT	viewport{};
            viewport.Width = static_cast<float>(renderPost_[0]->width);
            viewport.Height = static_cast<float>(renderPost_[0]->height);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            dc->RSSetViewports(1, &viewport);

            //�V�F�[�_�[���\�[�X�r���[�ݒ�
            PostRenderTarget* ps = graphics.GetPostEffectModelRenderTarget().get();
            drawTexture_->SetShaderResourceView(
                ps->diffuseMap, ps->width, ps->height);

            drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
                0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
                0,
                1, 1, 1, 1);


            dc->UpdateSubresource(bloomExBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.bloomLuminance, 0, 0);
            dc->PSSetConstantBuffers(0, 1, bloomExBuffer_.GetAddressOf());


            //���P�x���o
            bloomExtract_->Draw(drawTexture_.get());
        }

        //�򂵃o�b�t�@�X�V
        {
            BloomBlur bloomBlur = {};
            CalcGaussianFilter(bloomBlur.Weight, KARNEL_SIZE, 50.0f);//�K�E�X�t�B���^�[�쐬 
            //�R���X�^���g�o�b�t�@�X�V
            bloomBlur.KarnelSize = KARNEL_SIZE;
            bloomBlur.texel.x = 1.0f / renderPost_[3]->width;
            bloomBlur.texel.y = 1.0f / renderPost_[3]->height;
            dc->UpdateSubresource(bloomBuffer_.Get(), 0, NULL, &bloomBlur, 0, 0);
            dc->PSSetConstantBuffers(7, 1, bloomBuffer_.GetAddressOf());        
        }

        //���P�x�`��Ώۂ��
        {
            //�`����ύX
            ID3D11RenderTargetView* rtv = renderPost_[3]->renderTargetView.Get();
            FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
            dc->ClearRenderTargetView(rtv, color);
            dc->OMSetRenderTargets(1, &rtv, nullptr);
            D3D11_VIEWPORT	viewport{};
            viewport.Width = static_cast<float>(renderPost_[3]->width);
            viewport.Height = static_cast<float>(renderPost_[3]->height);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            dc->RSSetViewports(1, &viewport);

            //�V�F�[�_�[���\�[�X�r���[�ݒ�
            drawTexture_->SetShaderResourceView(
                renderPost_[0]->diffuseMap,
                renderPost_[0]->width, renderPost_[0]->height);

            drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
                0, 0, static_cast<float>(renderPost_[0]->width), static_cast<float>(renderPost_[0]->height),
                0,
                1, 1, 1, 1);

            //��
            bloomBlur_->Draw(drawTexture_.get());
        }
    }
#pragma endregion

#pragma region �쐣��?�u���[��
    {
        if (bloomKawaseFilter_->IsEnabled())
        {

            //�u���[���̃o�b�t�@�Ɣ�邽�߉��
            ID3D11ShaderResourceView* srvNull[] = { NULL };
            dc->PSSetShaderResources(0, 1, srvNull);

            //���P�x���o�p�o�b�t�@
            {
                //�`����ύX
                ID3D11RenderTargetView* rtv = renderPost_[0]->renderTargetView.Get();
                FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
                dc->ClearRenderTargetView(rtv, color);
                dc->OMSetRenderTargets(1, &rtv, nullptr);
                D3D11_VIEWPORT	viewport{};
                viewport.Width = static_cast<float>(renderPost_[0]->width);
                viewport.Height = static_cast<float>(renderPost_[0]->height);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                dc->RSSetViewports(1, &viewport);

                //�V�F�[�_�[���\�[�X�r���[�ݒ�
                PostRenderTarget* ps = graphics.GetPostEffectModelRenderTarget().get();
                drawTexture_->SetShaderResourceView(
                    ps->diffuseMap, ps->width, ps->height);

                drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
                    0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
                    0,
                    1, 1, 1, 1);


                dc->UpdateSubresource(bloomExBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.bloomLuminance, 0, 0);
                dc->PSSetConstantBuffers(0, 1, bloomExBuffer_.GetAddressOf());


                //���P�x���o
                bloomExtract_->Draw(drawTexture_.get());
            }

            //�_�E���T���v�����O���āA�򂵂𕡐��񂷂�
            for (int i = 0; i < BlurCount; ++i)
            {
                int index = i + 1;
                //�򂵃o�b�t�@�X�V
                {
                    BloomBlur bloomBlur = {};
                    CalcGaussianFilter(bloomBlur.Weight, KARNEL_SIZE, 50.0f);//�K�E�X�t�B���^�[�쐬 
                    //�R���X�^���g�o�b�t�@�X�V
                    bloomBlur.KarnelSize = KARNEL_SIZE;
                    bloomBlur.texel.x = 1.0f / renderPost_[index]->width;
                    bloomBlur.texel.y = 1.0f / renderPost_[index]->height;
                    dc->UpdateSubresource(bloomBuffer_.Get(), 0, NULL, &bloomBlur, 0, 0);
                    dc->PSSetConstantBuffers(7, 1, bloomBuffer_.GetAddressOf());
                }

                //���P�x�`��Ώۂ��
                {
                    //�`����ύX
                    ID3D11RenderTargetView* rtv = renderPost_[index]->renderTargetView.Get();
                    FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
                    dc->ClearRenderTargetView(rtv, color);
                    dc->OMSetRenderTargets(1, &rtv, nullptr);
                    D3D11_VIEWPORT	viewport{};
                    viewport.Width = static_cast<float>(renderPost_[index]->width);
                    viewport.Height = static_cast<float>(renderPost_[index]->height);
                    viewport.MinDepth = 0.0f;
                    viewport.MaxDepth = 1.0f;
                    dc->RSSetViewports(1, &viewport);

                    //�V�F�[�_�[���\�[�X�r���[�ݒ�
                    drawTexture_->SetShaderResourceView(
                        renderPost_[index - 1]->diffuseMap,
                        renderPost_[index - 1]->width, renderPost_[index - 1]->height);

                    drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
                        0, 0, static_cast<float>(renderPost_[index - 1]->width), static_cast<float>(renderPost_[index - 1]->height),
                        0,
                        1, 1, 1, 1);

                    //��
                    bloomBlur_->Draw(drawTexture_.get());
                }
            }

            //�򂵂̕��ς��v�Z
            {
                //�`����ύX
                ID3D11RenderTargetView* rtv = renderPost_[0]->renderTargetView.Get();
                FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
                dc->ClearRenderTargetView(rtv, color);
                dc->OMSetRenderTargets(1, &rtv, nullptr);
                D3D11_VIEWPORT	viewport{};
                viewport.Width = static_cast<float>(renderPost_[0]->width);
                viewport.Height = static_cast<float>(renderPost_[0]->height);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                dc->RSSetViewports(1, &viewport);

                //�V�F�[�_�[���\�[�X�r���[�ݒ�
                drawTexture_->SetShaderResourceView(
                    renderPost_[1]->diffuseMap,
                    static_cast<int>(viewport.Width), static_cast<int>(viewport.Height));

                for (int i = 0; i < BlurCount-1; ++i)
                {
                    dc->PSSetShaderResources(
                        i + 1, 1,
                        renderPost_[i+2]->diffuseMap.GetAddressOf()
                    );

                }

                drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
                    0, 0, static_cast<float>(renderPost_[0]->width), static_cast<float>(renderPost_[0]->height),
                    0,
                    1, 1, 1, 1);

                //���ω�
                bloomKawaseFilter_->Draw(drawTexture_.get());
            }
        }
    }
#pragma endregion


#pragma region �|�X�g�G�t�F�N�g�K�p
    {
        //�`����ύX
        ID3D11RenderTargetView* rtv = renderPostFull_->renderTargetView.Get();

        PostRenderTarget* ps = graphics.GetPostEffectModelRenderTarget().get();
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(rtv, color);
        dc->OMSetRenderTargets(1, &rtv, nullptr);
        D3D11_VIEWPORT	viewport{};
        viewport.Width = static_cast<float>(ps->width);
        viewport.Height = static_cast<float>(ps->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &viewport);

        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        drawTexture_->SetShaderResourceView(
            renderPostRadial_->diffuseMap, ps->width, ps->height);

        drawTexture_->Update(0, 0, graphics.GetScreenWidth(), graphics.GetScreenHeight(),
            0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
            0,
            1, 1, 1, 1);

        //�쐣�u���[��
        if (bloomKawaseFilter_->IsEnabled()) {
            dc->PSSetShaderResources(
                1, 1,
                renderPost_[0]->diffuseMap.GetAddressOf()
            );
        }
        else
        {
            //�u���[��
            dc->PSSetShaderResources(
                1, 1,
                renderPost_[3]->diffuseMap.GetAddressOf()
            );
        }

        //�r�l�b�g�o�b�t�@�X�V
        {
            dc->UpdateSubresource(vignetteBuffer_.Get(), 0, NULL, &graphics.shaderParameter3D_.vignette, 0, 0);
            dc->PSSetConstantBuffers(0, 1, vignetteBuffer_.GetAddressOf());
        }


        postEffect_->Draw(drawTexture_.get());
    }
#pragma endregion

    //�o�b�t�@�߂�
    graphics.RestoreRenderTargets();

    //�|�X�g�G�t�F�N�g�`��
    {            
        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        PostRenderTarget* ps = graphics.GetPostEffectModelRenderTarget().get();
        drawTexture_->SetShaderResourceView(
            renderPostFull_->diffuseMap, static_cast<int>(ps->width), static_cast<int>(ps->height));

        drawTexture_->Update(0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
            0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
            0,
            1, 1, 1, 1);

        postRender_->Draw(drawTexture_.get());

#pragma region �J���[�O���[�f�B���O
        if (colorGrading_->IsEnabled())
        {
            //�V�F�[�_�[���\�[�X�r���[�ݒ�
            drawTexture_->SetShaderResourceView(
                renderPostFull_->diffuseMap, static_cast<int>(ps->width), static_cast<int>(ps->height));

            drawTexture_->Update(0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
                0, 0, static_cast<float>(ps->width), static_cast<float>(ps->height),
                0,
                1, 1, 1, 1);

            ID3D11Buffer* constantBuffers[] =
            {
                colorGradingBuffer_.Get()
            };
            dc->PSSetConstantBuffers(5, ARRAYSIZE(constantBuffers), constantBuffers);

            dc->UpdateSubresource(colorGradingBuffer_.Get(), 0, 0, &graphics.shaderParameter3D_.colorGradingData, 0, 0);

            colorGrading_->Draw(drawTexture_.get());
        }
#pragma endregion

    }

    //���
    ID3D11ShaderResourceView* srvNull[BloomCount] = { nullptr };
    dc->PSSetShaderResources(0, ARRAYSIZE(srvNull), srvNull);

    FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    //dc->ClearRenderTargetView(renderPost_[1]->renderTargetView.Get(), color);
    //dc->ClearRenderTargetView(renderPost_[3]->renderTargetView.Get(), color);
}

//ImGui
void PostEffect::ImGuiRender()
{

    Graphics& graphics = Graphics::Instance();

    if (ImGui::Begin("3DShaderParamerter", nullptr, ImGuiWindowFlags_None))
    {
        if (ImGui::TreeNode("Sun"))
        {
            DirectX::XMFLOAT4 lVec = graphics.shaderParameter3D_.lightDirection;
            if (ImGui::SliderFloat4("lightDirection", &lVec.x, -1, 1))
            {
                lVec.w = 0;
                DirectX::XMStoreFloat4(&graphics.shaderParameter3D_.lightDirection, DirectX::XMVector4Normalize(DirectX::XMLoadFloat4(&lVec)));
            }
            DirectX::XMFLOAT4 lCor = graphics.shaderParameter3D_.lightColor;
            if (ImGui::SliderFloat4("lightColor", &lCor.x, 0, 1))
            {
                lCor.w = 0;
                DirectX::XMStoreFloat4(&graphics.shaderParameter3D_.lightColor, DirectX::XMLoadFloat4(&lCor));
            }
            //���z�ƃ~�X�g
            if (ImGui::TreeNode("SunAtmosphere"))
            {
                bool enabled = sun_->IsEnabled();
                if (ImGui::Checkbox("enabled", &enabled))
                {
                    sun_->SetEnabled(enabled);
                }

                ImGui::SliderFloat4("mistColor", &graphics.shaderParameter3D_.sunAtmosphere.mistColor.x, 0, 1);
                ImGui::SliderFloat2("mistDensity", &graphics.shaderParameter3D_.sunAtmosphere.mistDensity.x, -1, 1);
                ImGui::DragFloat2("mist_heightFalloff", &graphics.shaderParameter3D_.sunAtmosphere.mist_heightFalloff.x, 1.0f);
                ImGui::DragFloat2("heightMistOffset", &graphics.shaderParameter3D_.sunAtmosphere.heightMistOffset.x, 1.0f);

                ImGui::DragFloat("mistCutoffDistance", &graphics.shaderParameter3D_.sunAtmosphere.mistCutoffDistance, 0.01f);

                ImGui::DragFloat("mistFlowSpeed", &graphics.shaderParameter3D_.sunAtmosphere.mistFlowSpeed, 0.1f);
                ImGui::DragFloat("mistFlowNoiseScaleFactor", &graphics.shaderParameter3D_.sunAtmosphere.mistFlowNoiseScaleFactor, 0.001f);
                ImGui::DragFloat("mistFlowDensityLowerLimit", &graphics.shaderParameter3D_.sunAtmosphere.mistFlowDensityLowerLimit, 0.01f);

                ImGui::DragFloat("distanceToSun", &graphics.shaderParameter3D_.sunAtmosphere.distanceToSun, 0.1f);
                ImGui::DragFloat("sunHighlightExponentialFactor", &graphics.shaderParameter3D_.sunAtmosphere.sunHighlightExponentialFactor, 0.1f);
                ImGui::DragFloat("sunHighlightIntensity", &graphics.shaderParameter3D_.sunAtmosphere.sunHighlightIntensity, 0.1f);


                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        //���ː��u���[
        if (ImGui::TreeNode("Radial"))
        {
            bool enabled = radial_->IsEnabled();
            if (ImGui::Checkbox("enabled", &enabled))
            {
                radial_->SetEnabled(enabled);
            }

            ImGui::DragFloat2("blurPos", &graphics.shaderParameter3D_.radialBlur.blurPos.x, 0.1f);
            ImGui::DragFloat("power", &graphics.shaderParameter3D_.radialBlur.power, 0.1f);

            ImGui::TreePop();
        }

        //�r�l�b�g
        if (ImGui::TreeNode("Vignette"))
        {
            bool enabled = false;
            if (graphics.shaderParameter3D_.vignette.enabled > 0.1f)enabled = true;
            if (ImGui::Checkbox("enabled", &enabled))
            {
                if (enabled)
                    graphics.shaderParameter3D_.vignette.enabled = 1;
                else
                    graphics.shaderParameter3D_.vignette.enabled = -1;
            }

            ImGui::SliderFloat("power", &graphics.shaderParameter3D_.vignette.power, 0, 1);

            ImGui::TreePop();
        }


        //�|�X�g�G�t�F�N�g
        //�J���[�O���[�f�B���O
        if (ImGui::TreeNode("ColorGrading"))
        {
            bool enabled = colorGrading_->IsEnabled();
            if(ImGui::Checkbox("enabled", &enabled))
            {
                colorGrading_->SetEnabled(enabled);
            }
            ImGui::Separator();
            ImGui::DragFloat("brightness", &graphics.shaderParameter3D_.colorGradingData.brightness, 0.01f);
            ImGui::DragFloat("contrast", &graphics.shaderParameter3D_.colorGradingData.contrast, 0.01f);
            ImGui::DragFloat("saturation", &graphics.shaderParameter3D_.colorGradingData.saturation, 0.01f);
            ImGui::DragFloat4("filter", &graphics.shaderParameter3D_.colorGradingData.filter.x, 0.01f);

            ImGui::TreePop();
        }

        //�u���[��
        if (ImGui::TreeNode("Bloom"))
        {
            bool enabled = bloomExtract_->IsEnabled();
            if(ImGui::Checkbox("enabled", &enabled))
            {
                bloomExtract_->SetEnabled(enabled);
            }
            bool enabledKawase = bloomKawaseFilter_->IsEnabled();
            if(ImGui::Checkbox("enabledKawase", &enabledKawase))
            {
                bloomKawaseFilter_->SetEnabled(enabledKawase);
            }
            ImGui::Separator();
            ImGui::SliderFloat("intensity", &graphics.shaderParameter3D_.bloomLuminance.intensity, 0, 100);
            ImGui::SliderFloat("threshold", &graphics.shaderParameter3D_.bloomLuminance.threshold, 0, 1);

           
            ImGui::TreePop();
        }

        //skymap
        if (ImGui::TreeNode("Skymap"))
        {
            ImGui::Checkbox("enabled", &enabledSkyMap_);

            ImGui::TreePop();
        }

        //�e
        if (ImGui::TreeNode("shadow"))
        {

            ImGui::ColorEdit4("shadowColor", &graphics.shaderParameter3D_.shadowMapData.shadowColor.x);
            ImGui::DragFloat("bias", &graphics.shaderParameter3D_.shadowMapData.shadowBias, 0.1f);
            ImGui::DragFloat("rect", &graphics.shaderParameter3D_.shadowMapData.shadowRect, 0.1f);
            if (ImGui::TreeNode("shadowRenderTargets"))
            {
                ImGui::Image(graphics.shaderParameter3D_.shadowMapData.shadowmapDepthStencil->diffuseMap.Get(), { 256, 256 }, { 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 });


            ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("RenderTargets"))
        {
            ImGui::Text("Color");
            ImGui::Image(drawTexture_->GetShaderResourceView().Get(), { 256, 256 }, { 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 });
            for (int i = 0; i < BloomCount; ++i)
            {
                std::string s = "Post" + std::to_string(i);
                ImGui::Text(s.c_str());
                ImGui::Image(renderPost_[i]->diffuseMap.Get(), { 256, 256 }, { 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 });
            }

            ImGui::TreePop();
        }


    }
    ImGui::End();
}

//�X�J�C�}�b�v�`��
void PostEffect::SkymapRender(std::shared_ptr<CameraCom> camera)
{
    if (!enabledSkyMap_)return;

    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();
    dc->OMSetDepthStencilState(dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::SKYMAP).Get(), 0);
    dc->RSSetState(dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::SKYMAP).Get());
    ID3D11SamplerState* samplerStates[] =
    {
        dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).Get(),
        dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_CLAMP).Get(),
        dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_BORDER_POINT).Get(),
    };
    dc->PSSetSamplers(0, ARRAYSIZE(samplerStates), samplerStates);


    dc->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    dc->IASetInputLayout(nullptr);

    dc->VSSetShader(skyVertex_.Get(), 0, 0);
    dc->PSSetShader(skyPixel_.Get(), 0, 0);

    //�R���X�^���g�o�b�t�@
    SkymapData skymapData = {};

    DirectX::XMStoreFloat4x4(&skymapData.invMat,
        DirectX::XMMatrixInverse(nullptr, 
            DirectX::XMLoadFloat4x4(&camera->GetView())
            * DirectX::XMLoadFloat4x4(&camera->GetProjection())));

    dc->UpdateSubresource(skymapBuffer_.Get(), 0, NULL, &skymapData, 0, 0);
    ID3D11Buffer* constantBuffers[] =
    {
        skymapBuffer_.Get()
    };
    dc->PSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);

    //�V�F�[�_�[���\�[�X�r���[
    ID3D11ShaderResourceView* srvs[] =
    {
        skymapSrv_.Get(),
    };
    dc->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

    dc->Draw(4, 0);

    //�I������
    {
        dc->VSSetShader(nullptr, nullptr, 0);
        dc->PSSetShader(nullptr, nullptr, 0);
        dc->IASetInputLayout(nullptr);
    }

}

void PostEffect::CacheMaskBuffer(std::shared_ptr<CameraCom> camera)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    for (int i = 0; i < 3; ++i)
    {
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(renderPost2D_[i]->renderTargetView.Get(), color);
    }

    graphics.CacheRenderTargets();

    //�`����ύX
    ID3D11RenderTargetView* rtv = renderPost2D_[0]->renderTargetView.Get();
    ID3D11DepthStencilView* dsv = depthPost2D_->depthStencilView.Get();

    FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    dc->ClearRenderTargetView(rtv, color);    
    dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    dc->OMSetRenderTargets(1, &rtv, dsv);
    D3D11_VIEWPORT	viewport{};
    viewport.Width = static_cast<float>(renderPost2D_[0]->width);
    viewport.Height = static_cast<float>(renderPost2D_[0]->height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    dc->RSSetViewports(1, &viewport);


    // �`�揈��
    ShaderParameter3D& rc = graphics.shaderParameter3D_;
    //�J�����p�����[�^�[�ݒ�
    rc.view = camera->GetView();
    rc.projection = camera->GetProjection();
    DirectX::XMFLOAT3 cameraPos = camera->GetGameObject()->transform_->GetWorldPosition();
    rc.viewPosition = { cameraPos.x,cameraPos.y,cameraPos.z,1 };

    saveLightDir_ = rc.lightDirection;
    rc.lightDirection = lightDirMask_;
}

void PostEffect::StartBeMaskBuffer()
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ID3D11DepthStencilView* dsv = depthPost2D_->depthStencilView.Get();

    //�`����ύX
    ID3D11RenderTargetView* rtv = renderPost2D_[1]->renderTargetView.Get();
    FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    dc->ClearRenderTargetView(rtv, color);	
    dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    dc->OMSetRenderTargets(1, &rtv, dsv);
    D3D11_VIEWPORT	viewport{};
    viewport.Width = static_cast<float>(renderPost2D_[1]->width);
    viewport.Height = static_cast<float>(renderPost2D_[1]->height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    dc->RSSetViewports(1, &viewport);
}

void PostEffect::RestoreMaskBuffer(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size)
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    //�}�X�N����鑤�𓮂�����悤�ɃX�v���C�g�ɂ���Srv�ɂ���
    {
        //�`����ύX
        ID3D11RenderTargetView* rtv = renderPost2D_[2]->renderTargetView.Get();
        ID3D11DepthStencilView* dsv = depthPost2D_->depthStencilView.Get();
        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(rtv, color);
        dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        dc->OMSetRenderTargets(1, &rtv, dsv);
        D3D11_VIEWPORT	viewport{};
        viewport.Width = static_cast<float>(renderPost2D_[2]->width);
        viewport.Height = static_cast<float>(renderPost2D_[2]->height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        dc->RSSetViewports(1, &viewport);

        //�}�X�N����鑤��sprite�œ�������悤��
        maskSprite_->SetShaderResourceView(renderPost2D_[1]->diffuseMap.Get(), 
            static_cast<float>(renderPost2D_[1]->width), 
            static_cast<float>(renderPost2D_[1]->height));
       
        maskSprite_->Render(dc, pos.x, pos.y,
            maskSprite_->GetTextureWidth() * size.x, maskSprite_->GetTextureHeight() * size.y
            , 0, 0, 
            static_cast<float>(maskSprite_->GetTextureWidth()), 
            static_cast<float>(maskSprite_->GetTextureHeight())
            , 0, 1, 1, 1, 1);
    }


    graphics.RestoreRenderTargets();

}

void PostEffect::DrawMask()
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    //�}�X�N����
    D3D11_VIEWPORT	viewport{};
    viewport.Width = static_cast<float>(renderPost2D_[0]->width);
    viewport.Height = static_cast<float>(renderPost2D_[0]->height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    dc->RSSetViewports(1, &viewport);

    //�}�X�N�����邩�i�ݒ莞�m�F�p�j
    if (!isMask_)
    {
        //�V�F�[�_�[���\�[�X�r���[�ݒ�
        drawTexture_->SetShaderResourceView(
            renderPost2D_[2]->diffuseMap, renderPost2D_[2]->width, renderPost2D_[2]->height);

        drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
            0, 0, static_cast<float>(renderPost2D_[2]->width), static_cast<float>(renderPost2D_[2]->height),
            0,
            1, 1, 1, 1);

        FLOAT color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        dc->ClearRenderTargetView(renderPost2D_[1]->renderTargetView.Get(), color);

        dc->PSSetShaderResources(1, 1, renderPost2D_[1]->diffuseMap.GetAddressOf());

        mask_->Draw(drawTexture_.get());
    }

    //�V�F�[�_�[���\�[�X�r���[�ݒ�
    drawTexture_->SetShaderResourceView(
        renderPost2D_[0]->diffuseMap, renderPost2D_[0]->width, renderPost2D_[0]->height);

    drawTexture_->Update(0, 0, viewport.Width, viewport.Height,
        0, 0, static_cast<float>(renderPost2D_[0]->width), static_cast<float>(renderPost2D_[0]->height),
        0,
        1, 1, 1, 1);

    dc->PSSetShaderResources(1, 1, renderPost2D_[2]->diffuseMap.GetAddressOf());

    mask_->Draw(drawTexture_.get());


    //���
    ID3D11ShaderResourceView* srvs[] = { nullptr };
    dc->PSSetShaderResources(1, ARRAYSIZE(srvs), srvs);
    //���z�����߂�
    ShaderParameter3D& rc = graphics.shaderParameter3D_;
    rc.lightDirection = saveLightDir_;
}

void PostEffect::DrawMaskGui()
{
    if (ImGui::Begin("Mask", nullptr, ImGuiWindowFlags_None))
    {
        ImGui::Checkbox("isMask", &isMask_);

        if (ImGui::DragFloat4("lightDir", &lightDirMask_.x, 0.01f))
        {
            DirectX::XMFLOAT3 dir = { lightDirMask_.x,lightDirMask_.y,lightDirMask_.z };
            DirectX::XMStoreFloat3(&dir,
                DirectX::XMVector3Normalize(
                    DirectX::XMLoadFloat3(&dir)));
            lightDirMask_.x = dir.x;
            lightDirMask_.y = dir.y;
            lightDirMask_.z = dir.z;
        }

        if (ImGui::TreeNode("RenderTargets"))
        {
            for (int i = 0; i < 2; ++i)
            {
                std::string s = "Post2D" + std::to_string(i);
                ImGui::Text(s.c_str());
                ImGui::Image(renderPost2D_[i]->diffuseMap.Get(), { 256, 256 }, { 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 });
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}


//�|�X�g�G�t�F�N�g�\����
PostEffect::ShaderPost::ShaderPost(const char* PS)
{
    ID3D11Device* device = Graphics::Instance().GetDevice();
    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

    //���_�V�F�[�_�[
    {
        // ���̓��C�A�E�g
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        dx11State->createVsFromCso(device, "Shader\\ShaderPostVS.cso", vertexShaderPost.GetAddressOf(),
            inputLayoutPost.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
    }

    //�s�N�Z���V�F�[�_�[
    {
        char filename[256];
        sprintf_s(filename, 256, "Shader/%s.cso", PS);
        dx11State->createPsFromCso(device, filename, pixelShaderPost.GetAddressOf());
    }
}

void PostEffect::ShaderPost::Draw(TextureFormat* renderTexture)
{
    ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

    //������
    {
        dc->VSSetShader(vertexShaderPost.Get(), nullptr, 0);
        dc->PSSetShader(pixelShaderPost.Get(), nullptr, 0);
        dc->IASetInputLayout(inputLayoutPost.Get());

        dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        dc->OMSetBlendState(
            dx11State->GetBlendState(Dx11StateLib::BLEND_STATE_TYPE::ALPHA).Get(),
            blend_factor, 0xFFFFFFFF);
        dc->OMSetDepthStencilState(
            dx11State->GetDepthStencilState(Dx11StateLib::DEPTHSTENCIL_STATE_TYPE::DEPTH_ON_3D).Get(),
            0);
        dc->RSSetState(
            dx11State->GetRasterizerState(Dx11StateLib::RASTERIZER_TYPE::FRONTCOUNTER_FALSE_CULLBACK).Get()
        );
        ID3D11SamplerState* samplerStates[] =
        {
            dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).Get(),
            dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_CLAMP).Get(),
            dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_BORDER_POINT).Get(),
            dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP_ANISO).Get(),
            dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_BORDER_LINER).Get(),
            dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP_POINT).Get(),
        };
        dc->PSSetSamplers(0, ARRAYSIZE(samplerStates), samplerStates);
    }

    //�`��
    {
        UINT stride = sizeof(TextureFormat::Vertex);
        UINT offset = 0;
       
        dc->IASetVertexBuffers(0, 1, renderTexture->GetVertexBuffer().GetAddressOf(), &stride, &offset);
        ID3D11ShaderResourceView* srvs[] =
        {
            renderTexture->GetShaderResourceView().Get(),
        };
        dc->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
        dc->Draw(4, 0);
    }

    //�I������
    {
        dc->VSSetShader(nullptr, nullptr, 0);
        dc->PSSetShader(nullptr, nullptr, 0);
        dc->IASetInputLayout(nullptr);       
        ID3D11ShaderResourceView* srvs[] =
        {
            nullptr
        };
        dc->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

    }
}