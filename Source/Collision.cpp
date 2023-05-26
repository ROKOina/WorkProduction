#include "Collision.h"
#include <cmath>

//球と球の交差判定
bool Collision::IntersectSphereVsSphere(
    const DirectX::XMFLOAT3& positionA,
    float radiusA,
    const DirectX::XMFLOAT3& positionB,
    float radiusB,
    DirectX::XMFLOAT3& outPositionB)
{
    //B->Aの単位ベクトルを算出
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec       = DirectX::XMVectorSubtract(PositionB, PositionA);  //引き算で方向を求めるa->b
    DirectX::XMVECTOR LengthSq  = DirectX::XMVector3LengthSq(Vec);  //sq = スクエア(平方根)    2乗していると考えて良い
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq); 

    //距離測定
    float range = radiusA + radiusB;
    if (range*range < lengthSq)
    {
        return false;
    }

    Vec = DirectX::XMVector3Normalize(Vec);     //正規化して単位ベクトルに
    Vec = DirectX::XMVectorScale(Vec, range);   //AとBの距離分のベクトル作成

    PositionB = DirectX::XMVectorAdd(PositionA, Vec);   //A->Bに押し出すので、
                                                        //ポジションAを足し、Vecを足す

    //AがBを押し出す
   // PositionB = DirectX::XMVectorAdd(Vec, LengthSq );
    DirectX::XMStoreFloat3(&outPositionB, PositionB);

    return true;

}

//円柱と円柱の交差判定
bool Collision::IntersectCylinder(
    const DirectX::XMFLOAT3& positionA,
    float radiusA,
    float heightA,
    const DirectX::XMFLOAT3& positionB,
    float radiusB,
    float heightB,
    DirectX::XMFLOAT3& outPositionB
)
{
    //Aの足元がBの頭より上なら当たっていない
    if (positionA.y > positionB.y + heightB)
    {
        return false;
    }

    //Aの頭がBの足元より下なら当たっていない
    if (positionA.y + heightA < positionB.y)
    {
        return false;
    }
//    //カイトウ　
////XZ平面での範囲チェック
//    float vx = positionB.x - positionA.x;
//    float vz = positionB.z - positionA.z;
//    float rangeA = radiusA + radiusB;
//    float distXZ = sqrtf(vx * vx + vz * vz);
//    if (distXZ > rangeA)
//    {
//        return false;
//    }
//    //AがBを押し出す
//    vx /= distXZ;
//    vz /= distXZ;
//    outPositionB.x = positionA.x + (vx * rangeA);
//    outPositionB.y = positionA.y;
//    outPositionB.z = positionA.z + (vz * rangeA);
//
//    return true;

    //自分
    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);  //引き算で方向を求めるa->b
    Vec = DirectX::XMVectorSetY(Vec,0); //yだけ0に
                                                                              //DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);  //sq = スクエア(平方根)    2乗していると考えて良い

    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);  //sq = スクエア(平方根)    2乗していると考えて良い
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);

    //距離測定
    float range = radiusA + radiusB;
    if (range * range < lengthSq)
    {
        return false;
    }


    Vec = DirectX::XMVector3Normalize(Vec);     //正規化して単位ベクトルに
    Vec = DirectX::XMVectorScale(Vec, range);   //AとBの距離分のベクトル作成

    PositionB = DirectX::XMVectorAdd(PositionA, Vec);   //A->Bに押し出すので、

    outPositionB.x = DirectX::XMVectorGetX(PositionB);
    outPositionB.y = 0;
    outPositionB.z = DirectX::XMVectorGetZ(PositionB);

    return true;


}

//球と円柱の交差判定
bool Collision::IntersectSphereVsCylider(
    const DirectX::XMFLOAT3& spherePosition,
    float sphereRadius,
    const DirectX::XMFLOAT3& cylinderPosition,
    float cylinderRadius,
    float cylinderHeight,
    DirectX::XMFLOAT3& outCylinderPosition
)
{   
    //長方形と球を原点に持っていく
    DirectX::XMFLOAT3 spherePos0 = { 
        spherePosition.x - cylinderPosition.x,
         spherePosition.y - cylinderPosition.y,
          spherePosition.z - cylinderPosition.z
    };                              

    //if (spherePos0.y * spherePos0.y > (cylinderHeight + sphereRadius) * (cylinderHeight + sphereRadius))return false;//y判定
    if (spherePos0.x * spherePos0.x > (cylinderRadius + sphereRadius) * (cylinderRadius + sphereRadius))return false;//x判定
    if (spherePos0.z * spherePos0.z > (cylinderRadius + sphereRadius) * (cylinderRadius + sphereRadius))return false;//z判定

    DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&spherePosition);
    DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&cylinderPosition);
    DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);  //引き算で方向を求めるa->b
                                                                              //DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);  //sq = スクエア(平方根)    2乗していると考えて良い
    DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);  //sq = スクエア(平方根)    2乗していると考えて良い
    float lengthSq;
    DirectX::XMStoreFloat(&lengthSq, LengthSq);

    PositionB = DirectX::XMVectorAdd(PositionA, Vec);   //A->Bに押し出すので、
                                                        //ポジションAを足し、Vecを足す
    DirectX::XMStoreFloat3(&outCylinderPosition, PositionB);

    


    return true;

}

//レイとモデルの交差判定
bool Collision::IntersectRayVsModel(
    const DirectX::XMFLOAT3& start,
    const DirectX::XMFLOAT3& end,
    const Model* model,
    HitResult& result)
{
    DirectX::XMVECTOR WorldStart = DirectX::XMLoadFloat3(&start);
    DirectX::XMVECTOR WorldEnd = DirectX::XMLoadFloat3(&end);
    DirectX::XMVECTOR WorldRayVec = DirectX::XMVectorSubtract(WorldEnd, WorldStart);
    DirectX::XMVECTOR WorldRayLength = DirectX::XMVector3Length(WorldRayVec);

    //ワールド空間のレイの長さ
    DirectX::XMStoreFloat(&result.distance, WorldRayLength);

    bool hit = false;
    const ModelResource* resource = model->GetResource();
    for (const ModelResource::Mesh& mesh : resource->GetMeshes())
    {
        //メッシュノード取得
        const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);

        //レイをワールド空間からローカル空間へ変換
        DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&node.worldTransform);
        DirectX::XMMATRIX InverseWorldTransform = DirectX::XMMatrixInverse(nullptr, WorldTransform);

        DirectX::XMVECTOR S = DirectX::XMVector3TransformCoord(WorldStart, InverseWorldTransform);
        DirectX::XMVECTOR E = DirectX::XMVector3TransformCoord(WorldEnd, InverseWorldTransform);
        DirectX::XMVECTOR SE = DirectX::XMVectorSubtract(E, S);
        DirectX::XMVECTOR V = DirectX::XMVector3Normalize(SE);
        DirectX::XMVECTOR Length = DirectX::XMVector3Length(SE);

        //レイの長さ
        float neart;
        DirectX::XMStoreFloat(&neart, Length);

        //三角形（面）との交差判定
        const std::vector<ModelResource::Vertex>& vertices = mesh.vertices;
        const std::vector<UINT> indices = mesh.indices;

        int materialIndex = -1;
        DirectX::XMVECTOR HitPosition;
        DirectX::XMVECTOR HitNormal;
        for (const ModelResource::Subset& subset : mesh.subsets)
        {
            for (UINT i = 0; i < subset.indexCount; i += 3)
            {
                UINT index = subset.startIndex + i;

                //三角形の頂点を抽出
                const ModelResource::Vertex& a = vertices.at(indices.at(index));
                const ModelResource::Vertex& b = vertices.at(indices.at(index + 1));
                const ModelResource::Vertex& c = vertices.at(indices.at(index + 2));
                
                DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a.position);
                DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b.position);
                DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c.position);

                //三角形の三辺ベクトルを算出
                DirectX::XMVECTOR AB = DirectX::XMVectorSubtract(B, A);
                DirectX::XMVECTOR BC = DirectX::XMVectorSubtract(C, B);
                DirectX::XMVECTOR CA = DirectX::XMVectorSubtract(A, C);

                //三角形の法線ベクトルを算出
                DirectX::XMVECTOR N = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(AB, BC));

                //内積の結果がプラスならば裏向き
                DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(V, N);
                float dotLen = DirectX::XMVectorGetX(Dot);
                if (dotLen >= 0) continue;   //裏向きの場合continue

                //レイと平面の交点を算出
                DirectX::XMVECTOR SA = DirectX::XMVector3Dot(DirectX::XMVectorSubtract(A, S), N);

                float x= DirectX::XMVectorGetX(SA) / DirectX::XMVectorGetX(Dot);
                if (x<0.0f || x>neart)continue; //交点までの距離が今までに計算した最近距離より
                                                //大きい時はスキップ
                DirectX::XMVECTOR P = DirectX::XMVectorAdd(S, DirectX::XMVectorScale(V, x));

                //交点が三角形の内側にあるか判定
                //一つ目
                DirectX::XMVECTOR PA = DirectX::XMVectorSubtract(A, P);
                DirectX::XMVECTOR Cross1 = DirectX::XMVector3Cross(PA, AB); //三角形の外積
                DirectX::XMVECTOR Dot1 = DirectX::XMVector3Dot(Cross1, N);  //法線と外積の内積を求める
                float dot1;
                dot1 = DirectX::XMVectorGetX(Dot1);
                if (dot1 < 0)continue;  //三角形の外側ならcontinue

                //二つ目
                DirectX::XMVECTOR PB = DirectX::XMVectorSubtract(B, P);
                DirectX::XMVECTOR Cross2 = DirectX::XMVector3Cross(PB, BC); //三角形の外積
                DirectX::XMVECTOR Dot2 = DirectX::XMVector3Dot(Cross2, N);  //法線と外積の内積を求める
                float dot2;
                dot2 = DirectX::XMVectorGetX(Dot2);
                if (dot2 < 0)continue;  //三角形の外側ならcontinue

                //三つ目
                DirectX::XMVECTOR PC = DirectX::XMVectorSubtract(C, P);
                DirectX::XMVECTOR Cross3 = DirectX::XMVector3Cross(PC, CA); //三角形の外積
                DirectX::XMVECTOR Dot3 = DirectX::XMVector3Dot(Cross3, N);  //法線と外積の内積を求める
                float dot3;
                dot3 = DirectX::XMVectorGetX(Dot3);
                if (dot3 < 0)continue;  //三角形の外側ならcontinue

                //最近距離を更新
                neart = x;

                //交点を法線を更新
                HitPosition = P;
                HitNormal = N;
                materialIndex = subset.materialIndex;

            }
        }
        if (materialIndex >= 0)
        {
            //ローカル空間からワールド空間へ変換
            DirectX::XMVECTOR WorldPosition =
                DirectX::XMVector3TransformCoord(HitPosition, WorldTransform);
            DirectX::XMVECTOR WorldCrossVec = DirectX::XMVectorSubtract(WorldPosition, WorldStart);
            DirectX::XMVECTOR WorldCrossLength = DirectX::XMVector3Length(WorldCrossVec);
            float distance;
            DirectX::XMStoreFloat(&distance, WorldCrossLength);

            //ヒット情報保存
            if (result.distance > distance)
            {
                DirectX::XMVECTOR WorldNormal = 
                    DirectX::XMVector3TransformNormal(HitNormal, WorldTransform);
                result.distance = distance;
                result.materialIndex = materialIndex;
                DirectX::XMStoreFloat3(&result.position, WorldPosition);
                DirectX::XMStoreFloat3(&result.normal, DirectX::XMVector3Normalize(WorldNormal));
                //DirectX::XMStoreFloat3(&result.normal, DirectX::XMVector3Normalize(WorldNormal));
                hit = true;
            }

        }

    }

    return hit;
}
