#include "particle.h"

#include <random>

Particle::Particle(DirectX::XMFLOAT4 initial_position)
{
    //半径の外周半径
    float outermost_radius{ 30 };
    //パーティクル間隔
    float interval{ 0.5f };
    //パーティクル発生高さ
    float area_height{ 20 };
    //速度
    float fall_speed{ -0.5f };

    int orbit_count = static_cast<int>(outermost_radius / interval);

    for (int orbit_index = 1; orbit_index <= orbit_count; ++orbit_index)
    {
        float radius = orbit_index * interval;

        for (float theta = 0; theta < DirectX::XM_2PI; theta += interval / radius)
        {
            for (float height = 0; height < area_height; height += interval)
            {
                particle_count++;
            }
        }
    }
    particles.resize(particle_count);

    std::mt19937 mt{ std::random_device{}() };
    std::uniform_real_distribution<float> rand(-interval * 0.5f, +interval * 0.5f);

    size_t index{ 0 };
    for (int orbit_index = 1; orbit_index <= orbit_count; ++orbit_index)
    {
        float radius = orbit_index * interval;

        for (float theta = 0; theta < DirectX::XM_2PI; theta += interval / radius)
        {
            const float x{ radius * cosf(theta) };
            const float z{ radius * sinf(theta) };
            for (float height = -area_height * 0.5f; height < area_height * 0.5f; height += interval)
            {
                particles.at(index).position = { x + initial_position.x + rand(mt), height + initial_position.y + rand(mt), z + initial_position.z + rand(mt) };
                particles.at(index++).velocity = { 0.0f, fall_speed + rand(mt) * (fall_speed * 0.5f), 0.0f };
            }
        }
    }

    HRESULT hr{ S_OK };

    D3D11_BUFFER_DESC buffer_desc = {};
    buffer_desc.ByteWidth = static_cast<UINT>(sizeof(particle) * particle_count);
    buffer_desc.StructureByteStride = sizeof(particle);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    D3D11_SUBRESOURCE_DATA subresource_data{};
    subresource_data.pSysMem = particles.data();
    subresource_data.SysMemPitch = 0;
    subresource_data.SysMemSlicePitch = 0;
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&buffer_desc, &subresource_data, particle_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    D3D11_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc;
    unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
    unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    unordered_access_view_desc.Buffer.FirstElement = 0;
    unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(particle_count);
    unordered_access_view_desc.Buffer.Flags = 0;
    hr = Graphics::Instance().GetDevice()->CreateUnorderedAccessView(particle_buffer.Get(), &unordered_access_view_desc, particle_buffer_uav.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
    shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
    shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    shader_resource_view_desc.Buffer.ElementOffset = 0;
    shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(particle_count);
    hr = Graphics::Instance().GetDevice()->CreateShaderResourceView(particle_buffer.Get(), &shader_resource_view_desc, particle_buffer_srv.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    ZeroMemory(&buffer_desc, sizeof(buffer_desc));
    buffer_desc.ByteWidth = sizeof(particle_constants);
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.MiscFlags = 0;
    buffer_desc.StructureByteStride = 0;
    hr = Graphics::Instance().GetDevice()->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //シェーダー生成
    //頂点シェーダー
    hr = Graphics::Instance().GetDx11State()->createVsFromCso(Graphics::Instance().GetDevice(), "Shader\\ParticleVS.cso", vertex_shader.GetAddressOf(), nullptr, nullptr, 0);
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //ピクセルシェーダー
    hr = Graphics::Instance().GetDx11State()->createPsFromCso(Graphics::Instance().GetDevice(), "Shader\\ParticlePS.cso", pixel_shader.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

    //コンピュートシェーダー
    {
        FILE* fp{ nullptr };

        fopen_s(&fp, "Shader\\ParticleCS.cso", "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long cso_sz{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
        fread(cso_data.get(), cso_sz, 1, fp);
        fclose(fp);

        hr = Graphics::Instance().GetDevice()->CreateComputeShader(cso_data.get(), cso_sz, nullptr, compute_shader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
    }
    //ジオメトリシェーダー
    {       
        FILE* fp{ nullptr };

        fopen_s(&fp, "Shader\\ParticleGS.cso", "rb");
        _ASSERT_EXPR_A(fp, "CSO File not found");

        fseek(fp, 0, SEEK_END);
        long cso_sz{ ftell(fp) };
        fseek(fp, 0, SEEK_SET);

        std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(cso_sz) };
        fread(cso_data.get(), cso_sz, 1, fp);
        fclose(fp);

        hr = Graphics::Instance().GetDevice()->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, geometry_shader.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
    }

    particle_data.current_eye_position = initial_position;
    particle_data.previous_eye_position = initial_position;
    particle_data.area_height = area_height;
    particle_data.outermost_radius = outermost_radius;
    particle_data.particle_size = 0.005f;
    particle_data.particle_count = static_cast<uint32_t>(particle_count);
}

static UINT align(UINT num, UINT alignment)
{
    return (num + (alignment - 1)) & ~(alignment - 1);
}
void Particle::integrate(float delta_time, DirectX::XMFLOAT4 eye_position,
    DirectX::XMFLOAT4X4	view, DirectX::XMFLOAT4X4	projection)
{
    ID3D11DeviceContext* context = Graphics::Instance().GetDeviceContext();

    particle_data.previous_eye_position= particle_data.current_eye_position;
    particle_data.current_eye_position = eye_position;
    particle_data.deltaTime = delta_time;
    particle_data.sceneTimer += delta_time;
    particle_data.view = view;
    particle_data.projection = projection;

    context->UpdateSubresource(constant_buffer.Get(), 0, 0, &particle_data, 0, 0);
    context->CSSetConstantBuffers(9, 1, constant_buffer.GetAddressOf());

    context->CSSetShader(compute_shader.Get(), NULL, 0);
    context->CSSetUnorderedAccessViews(0, 1, particle_buffer_uav.GetAddressOf(), nullptr);

    UINT num_threads = align(static_cast<UINT>(particle_count), 256);
    context->Dispatch(num_threads / 256, 1, 1);

    ID3D11UnorderedAccessView* null_unordered_access_view{};
    context->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, nullptr);
}

void Particle::Render(const RenderContext& rc)
{
    ID3D11DeviceContext* context = Graphics::Instance().GetDeviceContext();

    context->VSSetShader(vertex_shader.Get(), NULL, 0);
    context->PSSetShader(pixel_shader.Get(), NULL, 0);
    context->GSSetShader(geometry_shader.Get(), NULL, 0);
    context->GSSetShaderResources(9, 1, particle_buffer_srv.GetAddressOf());
    context->GSSetConstantBuffers(9, 1, constant_buffer.GetAddressOf());

    context->IASetInputLayout(NULL);
    context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    context->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    context->Draw(static_cast<UINT>(particle_count), 0);

    ID3D11ShaderResourceView* null_shaderresource_view{};
    context->GSSetShaderResources(9, 1, &null_shaderresource_view);
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
//void Particle::Render(const RenderContext& rc)
//{
//    ID3D11Buffer* constantBuffers[] =
//    {
//        wvpBuffer.Get(),
//    };
//    Graphics::Instance().GetDeviceContext()->VSSetConstantBuffers(5, ARRAYSIZE(constantBuffers), constantBuffers);
//
//    WVP wvp;
//    DirectX::XMStoreFloat4x4(&wvp.world, DirectX::XMMatrixIdentity());
//    wvp.view = rc.view;
//    wvp.projection = rc.projection;
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
