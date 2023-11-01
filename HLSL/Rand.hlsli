/*
 * Xorshift32��p����32bit�̋[�������𐶐�����
 */
uint XOrShift32(uint value)
{
    value = value ^ (value << 13);
    value = value ^ (value >> 17);
    value = value ^ (value << 5);
    return value;
}

/*
 * �����̒l��1�����̏����Ƀ}�b�s���O����
 */
float MapToFloat(uint value)
{
    const float precion = 100000000.0;
    return (value % precion) * rcp(precion);
}

/*
 * 3�����̃����_���Ȓl���Z�o����
 */
float3 Random3(uint3 src, int seed)
{
    uint3 random;
    random.x = XOrShift32(mad(src.x, src.y, src.z));
    random.y = XOrShift32(mad(random.x, src.z, src.x) + seed);
    random.z = XOrShift32(mad(random.y, src.x, src.y) + seed);
    random.x = XOrShift32(mad(random.z, src.y, src.z) + seed);

    return float3(MapToFloat(random.x), MapToFloat(random.y), MapToFloat(random.z));
}

float2 Random2(uint2 src, int seed)
{
    uint2 random;
    random.x = XOrShift32(mad(src.x, src.y,1));
    random.y = XOrShift32(mad(random.x, 1, src.x) + seed);

    return float2(MapToFloat(random.x), MapToFloat(random.y));
}