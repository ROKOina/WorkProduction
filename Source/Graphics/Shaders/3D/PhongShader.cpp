#include "Misc.h"
#include "PhongShader.h"
#include "Graphics\Graphics.h"

PhongShader::PhongShader(ID3D11Device* device)
{
    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

    //頂点シェーダー
    {
        //入力レイアウト
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        dx11State->createVsFromCso(device, "Shader\\PhongVS.cso", vertexShader_.GetAddressOf(),
            inputLayout_.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
    }

    //ピクセルシェーダー
    {
        dx11State->createPsFromCso(device, "Shader\\PhongPS.cso", pixelShader_.GetAddressOf());
    }

    //定数バッファ
    {
        dx11State->createConstantBuffer(device, sizeof(CbScene), sceneConstantBuffer_.GetAddressOf());
        dx11State->createConstantBuffer(device, sizeof(CbMesh), meshConstantBuffer_.GetAddressOf());
        dx11State->createConstantBuffer(device, sizeof(CbSubset), subsetConstantBuffer_.GetAddressOf());
        dx11State->createConstantBuffer(device, sizeof(CbShadowMap), shadowMapConstantBuffer_.GetAddressOf());
    }

}

void PhongShader::Begin(ID3D11DeviceContext* dc, const ShaderParameter3D& rc)
{
    dc->VSSetShader(vertexShader_.Get(), nullptr, 0);
    dc->PSSetShader(pixelShader_.Get(), nullptr, 0);
    dc->IASetInputLayout(inputLayout_.Get());

    ID3D11Buffer* constantBuffers[] =
    {
        sceneConstantBuffer_.Get(),
        meshConstantBuffer_.Get(),
        subsetConstantBuffer_.Get(),
        shadowMapConstantBuffer_.Get()
    };
    dc->VSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);
    dc->PSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);

    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();

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
       dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_BORDER_POINT).Get(),
    };
    dc->PSSetSamplers(0, ARRAYSIZE(samplerStates), samplerStates);


    // シーン用定数バッファ更新
    CbScene cbScene;

    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
    DirectX::XMStoreFloat4x4(&cbScene.viewProjection, V * P);
    cbScene.viewPosition = rc.viewPosition;
    cbScene.lightDirection = rc.lightDirection;
    cbScene.lightColor = rc.lightColor;

    cbScene.ambientLightColor = {1,1,1,1};

    dc->UpdateSubresource(sceneConstantBuffer_.Get(), 0, 0, &cbScene, 0, 0);

    //	シャドウマップ設定
    CbShadowMap shadowMapData;
    shadowMapData.lightViewProjection = rc.shadowMapData.lightViewProjection;
    shadowMapData.shadowBias = rc.shadowMapData.shadowBias;
    shadowMapData.shadowColor = rc.shadowMapData.shadowColor;
    dc->UpdateSubresource(shadowMapConstantBuffer_.Get(), 0, 0, &shadowMapData, 0, 0);
    dc->PSSetShaderResources(2, 1, rc.shadowMapData.shadowmapDepthStencil->diffuseMap.GetAddressOf());

}

void PhongShader::Draw(ID3D11DeviceContext* dc, const Model* model)
{
    const ModelResource* resource = model->GetResource();
    const std::vector<Model::Node>& nodes = model->GetNodes();
    Dx11StateLib* dx11State = Graphics::Instance().GetDx11State().get();
    

    //透明マテリアル
    std::vector<ModelResource::Mesh> alphaMesh;

        for (const ModelResource::Mesh& mesh : resource->GetMeshes())
        {
            // メッシュ用定数バッファ更新
            CbMesh cbMesh;
            ::memset(&cbMesh, 0, sizeof(cbMesh));
            if (mesh.nodeIndices.size() > 0)
            {
                for (size_t i = 0; i < mesh.nodeIndices.size(); ++i)
                {
                    DirectX::XMMATRIX worldTransform = DirectX::XMLoadFloat4x4(&nodes.at(mesh.nodeIndices.at(i)).worldTransform);
                    DirectX::XMMATRIX offsetTransform = DirectX::XMLoadFloat4x4(&mesh.offsetTransforms.at(i));
                    DirectX::XMMATRIX boneTransform = offsetTransform * worldTransform;
                    DirectX::XMStoreFloat4x4(&cbMesh.boneTransforms[i], boneTransform);
                }
            }
            else
            {
                cbMesh.boneTransforms[0] = nodes.at(mesh.nodeIndex).worldTransform;
            }
            dc->UpdateSubresource(meshConstantBuffer_.Get(), 0, 0, &cbMesh, 0, 0);

            UINT stride = sizeof(ModelResource::Vertex);
            UINT offset = 0;
            dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
            dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            for (const ModelResource::Subset& subset : mesh.subsets)
            {
                CbSubset cbSubset;

                //cbSubset.materialColor = subset.material->color;
                cbSubset.materialColor = model->GetMaterialColor();


                ////透明描画なら後回しにする
                //if (!alphaMat)
                //{
                //    if (cbSubset.materialColor.w < 1)
                //    {
                //        alphaMesh.emplace_back(mesh);
                //        continue;
                //    }
                //}

                dc->UpdateSubresource(subsetConstantBuffer_.Get(), 0, 0, &cbSubset, 0, 0);
                ID3D11ShaderResourceView* srvs[] =
                {
                    subset.material->diffuseMap.Get(),
                    subset.material->normalMap.Get(),
                };
                dc->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);

                dc->PSSetSamplers(0, 1, dx11State->GetSamplerState(Dx11StateLib::SAMPLER_TYPE::TEXTURE_ADDRESS_WRAP).GetAddressOf());
                dc->DrawIndexed(subset.indexCount, subset.startIndex, 0);
            }
        }

}

void PhongShader::End(ID3D11DeviceContext* context)
{
    context->VSSetShader(nullptr, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);
    context->IASetInputLayout(nullptr);

    ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr };
    context->PSSetShaderResources(0, ARRAYSIZE(srvs), srvs);
}
