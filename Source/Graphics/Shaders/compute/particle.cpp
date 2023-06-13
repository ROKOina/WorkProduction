#include "particle.h"

#include <random>

Particle::Particle(DirectX::XMFLOAT4 initialPosition)
{
    //半径の外周半径
    float outermostRadius{ 10 };
    //パーティクル間隔
    float interval{ 0.5f };
    //パーティクル発生高さ
    float areaHeight{ 20 };
    //速度
    float fallSpeed{ -0.5f };

    int orbitCount = static_cast<int>(outermostRadius / interval);

    for (int orbitIndex = 1; orbitIndex <= orbitCount; ++orbitIndex)
    {
        float radius = orbitIndex * interval;

        for (float theta = 0; theta < DirectX::XM_2PI; theta += interval / radius)
        {
            for (float height = 0; height < areaHeight; height += interval)
            {
                particleCount_++;
            }
        }
    }
    particles_.resize(particleCount_);

    std::mt19937 mt{ std::random_device{}() };
    std::uniform_real_distribution<float> rand(-interval * 0.5f, +interval * 0.5f);

    size_t index{ 0 };
    for (int orbitIndex = 1; orbitIndex <= orbitCount; ++orbitIndex)
    {
        float radius = orbitIndex * interval;

        for (float theta = 0; theta < DirectX::XM_2PI; theta += interval / radius)
        {
            const float x{ radius * cosf(theta) };
            const float z{ radius * sinf(theta) };
            for (float height = -areaHeight * 0.5f; height < areaHeight * 0.5f; height += interval)
            {
                particles_.at(index).position = { x + initialPosition.x + rand(mt), height + initialPosition.y + rand(mt), z + initialPosition.z + rand(mt) };
                particles_.at(index++).velocity = { 0.0f, fallSpeed + rand(mt) * (fallSpeed * 0.5f), 0.0f };
            }
        }
    }

    HRESULT hr{ S_OK };

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(particle) * particleCount_);
    bufferDesc.StructureByteStride = sizeof(particle);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = particles_.data();
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, &subresource_data, particleBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
    unorderedAccessViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    unorderedAccessViewDesc.Buffer.FirstElement = 0;
    unorderedAccessViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount_);
    unorderedAccessViewDesc.Buffer.Flags = 0;
    hr = Graphics::Instance().GetDevice()->CreateUnorderedAccessView(particleBuffer_.Get(), &unorderedAccessViewDesc, particleBufferUav_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shaderResourceViewDesc.Buffer.ElementOffset = 0;
    shaderResourceViewDesc.Buffer.NumElements = static_cast<UINT>(particleCount_);
    hr = Graphics::Instance().GetDevice()->CreateShaderResourceView(particleBuffer_.Get(), &shaderResourceViewDesc, particleBufferSrv_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = sizeof(particle_constants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bufferDesc, nullptr, constantBuffer_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //シェーダー生成
    //頂点シェーダー
    hr = Graphics::Instance().GetDx11State()->createVsFromCso(Graphics::Instance().GetDevice(), "Shader\\ParticleVS.cso", vertexShader_.GetAddressOf(), nullptr, nullptr, 0);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //ピクセルシェーダー
    hr = Graphics::Instance().GetDx11State()->createPsFromCso(Graphics::Instance().GetDevice(), "Shader\\ParticlePS.cso", pixelShader_.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //コンピュートシェーダー
    {
        FILE* fp{ nullptr };

        fopen_s(&fp, "Shader\\ParticleCS.cso", "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long csoSz{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> csoData{ std::make_unique<unsigned char[]>(csoSz) };
        fread(csoData.get(), csoSz, 1, fp);
        fclose(fp);

        hr = Graphics::Instance().GetDevice()->CreateComputeShader(csoData.get(), csoSz, nullptr, computeShader_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
    }
    //ジオメトリシェーダー
    {       
        FILE* fp{ nullptr };

        fopen_s(&fp, "Shader\\ParticleGS.cso", "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long csoSz{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> csoData{ std::make_unique<unsigned char[]>(csoSz) };
        fread(csoData.get(), csoSz, 1, fp);
        fclose(fp);

        hr = Graphics::Instance().GetDevice()->CreateGeometryShader(csoData.get(), csoSz, nullptr, geometryShader_.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
    }

    particleData_.currentEyePosition = initialPosition;
    particleData_.previousEyePosition = initialPosition;
    particleData_.areaHeight = areaHeight;
    particleData_.outermostRadius = outermostRadius;
    particleData_.particleSize = 0.005f;
    particleData_.particleCount = static_cast<uint32_t>(particleCount_);
}

static UINT align(UINT num, UINT alignment)
{
    return (num + (alignment - 1)) & ~(alignment - 1);
}
void Particle::integrate(float deltaTime, DirectX::XMFLOAT4 eyePosition,
    DirectX::XMFLOAT4X4	view, DirectX::XMFLOAT4X4	projection)
{
    ID3D11DeviceContext* context = Graphics::Instance().GetDeviceContext();

    particleData_.previousEyePosition= particleData_.currentEyePosition;
    particleData_.currentEyePosition = eyePosition;
    particleData_.deltaTime = deltaTime;
    particleData_.sceneTimer += deltaTime;
    particleData_.view = view;
    particleData_.projection = projection;

    context->UpdateSubresource(constantBuffer_.Get(), 0, 0, &particleData_, 0, 0);
    context->CSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

    context->CSSetShader(computeShader_.Get(), NULL, 0);
    context->CSSetUnorderedAccessViews(0, 1, particleBufferUav_.GetAddressOf(), nullptr);

    UINT numThreads = align(static_cast<UINT>(particleCount_), 256);
    context->Dispatch(numThreads / 256, 1, 1);

    ID3D11UnorderedAccessView* nullUnorderedAccessView{};
    context->CSSetUnorderedAccessViews(0, 1, &nullUnorderedAccessView, nullptr);
}

void Particle::Render(const RenderContext& rc)
{
    ID3D11DeviceContext* context = Graphics::Instance().GetDeviceContext();

    context->VSSetShader(vertexShader_.Get(), NULL, 0);
    context->PSSetShader(pixelShader_.Get(), NULL, 0);
    context->GSSetShader(geometryShader_.Get(), NULL, 0);
    context->GSSetShaderResources(9, 1, particleBufferSrv_.GetAddressOf());
    context->GSSetConstantBuffers(9, 1, constantBuffer_.GetAddressOf());

    context->IASetInputLayout(NULL);
    context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    context->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->Draw(static_cast<UINT>(particleCount_), 0);

    ID3D11ShaderResourceView* nullShaderResourceView{};
    context->GSSetShaderResources(9, 1, &nullShaderResourceView);
    context->VSSetShader(NULL, NULL, 0);
    context->PSSetShader(NULL, NULL, 0);
    context->GSSetShader(NULL, NULL, 0);

}


//void Particle::Init()
//{
//    //頂点資料入れる (pSizeは自分で決める)
//    VERTEX_3D_PARTICLE vertex[4];
//    vertex[0].Position = { -pSize,pSize,0 };
//    vertex[0].TexCoord = { 0,0 };
//    vertex[1].Position = { pSize,pSize,0 };
//    vertex[1].TexCoord = { 1,0 };
//    vertex[2].Position = { -pSize,-pSize,0 };
//    vertex[2].TexCoord = { 0,1 };
//    vertex[3].Position = { pSize,-pSize,0 };
//    vertex[3].TexCoord = { 1,1 };
//
//    //パーティクル資料生成(pAmountは最大数)
//    mParticle = new ParticleCompute[pAmount];
//
//    //パーティクルの資料入れる（固定かランダムとか自分で作る）
//    for (int i = 0; i < pAmount; i++) {
//        mParticle[i].vel = { 0,1,0 }; // 速度
//        mParticle[i].life = 300.0f;   // ライフ（フレーム）
//        mParticle[i].pos = { 0,0,0 }; // 座標
//    }
//
//    HRESULT hr{ S_OK };
//    //バッファ生成
//    D3D11_BUFFER_DESC bd;
//    ZeroMemory(&bd, sizeof(bd));
//    bd.Usage = D3D11_USAGE_DYNAMIC;
//    bd.ByteWidth = sizeof(VERTEX_3D_PARTICLE) * 4;
//    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//    D3D11_SUBRESOURCE_DATA sd;
//    ZeroMemory(&sd, sizeof(sd));
//    sd.pSysMem = vertex;
//
//    hr = Graphics::Instance().GetDevice()->CreateBuffer(&bd, &sd, mVertexBuffer.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//    D3D11_BUFFER_DESC desc;
//    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//    desc.ByteWidth = sizeof(ParticleCompute) * pAmount;
//    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//    desc.StructureByteStride = sizeof(ParticleCompute);
//    desc.Usage = D3D11_USAGE_DEFAULT;
//    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//    hr = Graphics::Instance().GetDevice()->CreateBuffer(&desc, 0, mpParticleBuffer.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
//    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//
//    hr = Graphics::Instance().GetDevice()->CreateBuffer(&desc, 0, mpResultBuffer.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//    desc.ByteWidth = sizeof(DirectX::XMFLOAT3) * pAmount;
//    desc.StructureByteStride = sizeof(DirectX::XMFLOAT3);
//    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//    desc.CPUAccessFlags = 0;
//
//    hr = Graphics::Instance().GetDevice()->CreateBuffer(&desc, 0, mpPositionBuffer.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//
//    //シェーダーリソースビュー生成
//    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
//    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
//    srvDesc.Buffer.FirstElement = 0;
//    srvDesc.Buffer.NumElements = 1;
//
//    hr = Graphics::Instance().GetDevice()->CreateShaderResourceView(mpParticleBuffer.Get(), &srvDesc, mpParticleSRV.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//    hr = Graphics::Instance().GetDevice()->CreateShaderResourceView(mpPositionBuffer.Get(), &srvDesc, mpPositionSRV.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//    //UAV生成
//    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
//    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
//    uavDesc.Buffer.FirstElement = 0;
//    uavDesc.Buffer.Flags = 0;
//    uavDesc.Buffer.NumElements = pAmount;
//
//    hr = Graphics::Instance().GetDevice()->CreateUnorderedAccessView(mpResultBuffer.Get(), &uavDesc, mpResultUAV.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//
//    //コンスタントバッファ
//    ::memset(&desc, 0, sizeof(desc));
//    desc.Usage = D3D11_USAGE_DEFAULT;
//    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//    desc.CPUAccessFlags = 0;
//    desc.MiscFlags = 0;
//    desc.ByteWidth = sizeof(WVP);
//    desc.StructureByteStride = 0;
//
//    hr = Graphics::Instance().GetDevice()->CreateBuffer(&desc, 0, wvpBuffer.GetAddressOf());
//    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//
//
//    //コンピュートシェーダー生成
//    {
//        FILE* fp{ nullptr };
//
//        fopen_s(&fp, "Shader\\ParticleCS.cso", "rb");
//        _ASSERT_EXPR_A(fp, "CSO File not found");
//
//        fseek(fp, 0, SEEK_END);
//        long cso_sz{ ftell(fp) };
//        fseek(fp, 0, SEEK_SET);
//
//        std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
//        fread(cso_data.get(), cso_sz, 1, fp);
//        fclose(fp);
//
//        hr = Graphics::Instance().GetDevice()->CreateComputeShader(cso_data.get(), cso_sz, nullptr, mComputeShader.GetAddressOf());
//        _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//    }
//}
//
//void Particle::Update()
//{
//    // パーティクルの資料をバッファに入れる
//    D3D11_MAPPED_SUBRESOURCE subRes;
//    //Graphics::Instance().GetDeviceContext()->Map(mpParticleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
//    //memcpy(subRes.pData, mParticle, sizeof(ParticleCompute) * pAmount);
//    //Graphics::Instance().GetDeviceContext()->Unmap(mpParticleBuffer.Get(), 0);
//
//    //　コンピュートシェーダー実行
//    ID3D11ShaderResourceView* pSRVs[1] = { mpParticleSRV.Get() };
//    Graphics::Instance().GetDeviceContext()->CSSetShaderResources(0, 1, pSRVs);
//    Graphics::Instance().GetDeviceContext()->CSSetShader(mComputeShader.Get(), nullptr, 0);
//    Graphics::Instance().GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, mpResultUAV.GetAddressOf(), 0);
//    Graphics::Instance().GetDeviceContext()->Dispatch(256, 1, 1);
//
//    //// 戻った計算結果をバッファに入れる
//    //subRes = {};
//    //ID3D11Buffer* pBufDbg = {};
//
//    //D3D11_BUFFER_DESC desc;
//    //desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
//    //desc.ByteWidth = sizeof(ParticleCompute) * pAmount;
//    //desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//    //desc.StructureByteStride = sizeof(ParticleCompute);
//    //desc.Usage = D3D11_USAGE_DEFAULT;
//    //desc.CPUAccessFlags = 0;
//
//    //HRESULT hr = Graphics::Instance().GetDevice()->CreateBuffer(&desc, 0, &pBufDbg);
//    //_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//    //Graphics::Instance().GetDeviceContext()->CopyResource(mpResultBuffer.Get(), pBufDbg);
//    //Graphics::Instance().GetDeviceContext()->Map(pBufDbg, 0, D3D11_MAP_READ, 0, &subRes);
//    //ParticleCompute* pBufType = (ParticleCompute*)subRes.pData;
//    //memcpy(mParticle, pBufType, sizeof(ParticleCompute) * pAmount);
//    //Graphics::Instance().GetDeviceContext()->Unmap(pBufDbg, 0);
//    //pBufDbg->Release();
//
//    // 座標を座標バッファに入れる(頂点シェーダーで使う)
//    subRes = {};
//    Graphics::Instance().GetDeviceContext()->Map(mpPositionBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
//    //DirectX::XMFLOAT3* pBufTypePos = (DirectX::XMFLOAT3*)subRes.pData;
//    DirectX::XMFLOAT3* pBufTypePos = new DirectX::XMFLOAT3[pAmount];
//    for (int v = 0; v < pAmount; v++) {
//        pBufTypePos[v] = mParticle[v].pos;
//    }
//    Graphics::Instance().GetDeviceContext()->Unmap(mpPositionBuffer.Get(), 0);
//}
//
//void Particle::Render(const RenderContext& rc_)
//{
//    ID3D11Buffer* constantBuffers[] =
//    {
//        wvpBuffer.Get(),
//    };
//    Graphics::Instance().GetDeviceContext()->VSSetConstantBuffers(5, ARRAYSIZE(constantBuffers), constantBuffers);
//
//    WVP wvp;
//    DirectX::XMStoreFloat4x4(&wvp.world, DirectX::XMMatrixIdentity());
//    wvp.view = rc_.view;
//    wvp.projection = rc_.projection;
//    Graphics::Instance().GetDeviceContext()->UpdateSubresource(wvpBuffer.Get(), 0, 0, &wvp, 0, 0);
//
//    // 描画
//    UINT stride = sizeof(VERTEX_3D_PARTICLE);
//    UINT offset = 0;
//    Graphics::Instance().GetDeviceContext()->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
//    //Graphics::Instance().GetDeviceContext()->PSSetShaderResources(0, 1, mTexture); // テクスチャ設定（あれば）
//    Graphics::Instance().GetDeviceContext()->VSSetShaderResources(2, 1, mpPositionSRV.GetAddressOf()); // VSに入れる座標設定
//    Graphics::Instance().GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//    Graphics::Instance().GetDeviceContext()->DrawInstanced(4, pAmount, 0, 0);
//}
