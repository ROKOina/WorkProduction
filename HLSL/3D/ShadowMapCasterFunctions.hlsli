//--------------------------------------------
//	�V���h�E�}�b�v�Q�Ɨp���̌v�Z
//--------------------------------------------
// worldPosition		: ���[���h���W
// lightViewProjection	: ���C�g�r���[�v���W�F�N�V�����s��
// �Ԃ��l				: �V���h�E�}�b�v�Q�Ɨp��UV���W�y�ѐ[�x���
float3 CalcShadowTexcoord(float3 worldPosition, matrix lightViewProjection)
{
	// �N���b�v��Ԃ̍��W�ɕϊ�
    float4 worldViewProjectionPosition = mul(float4(worldPosition, 1), lightViewProjection);

	// �������Z����NDC���W�ɕϊ�
    worldViewProjectionPosition /= worldViewProjectionPosition.w;

	// NDC���W��XY���W��UV���W�ɕϊ�
	// Z�l�͂��ł�0�`1�͈̔͂ɕϊ�����Ă���̂ŁA�������Ȃ��ėǂ�
    worldViewProjectionPosition.y *= -1;
    worldViewProjectionPosition.xy
		= worldViewProjectionPosition.xy * 0.5f + 0.5f;

    return worldViewProjectionPosition.xyz;
}

//--------------------------------------------
//	�V���h�E�}�b�v����[�x�l���擾���ĉe���ǂ�����n��
//--------------------------------------------
// tex				: �V���h�E�}�b�v
// samplerState		: �T���v���X�e�[�g
// shadowTexcoord	: �V���h�E�}�b�v�Q�Ɨp���
// shadowColor		: �e�̐F
// shadowBias		: �[�x��r�p�̃I�t�Z�b�g�l
// �Ԃ��l			: �e���ǂ���
float3 CalcShadowColor(Texture2D tex, SamplerState samplerState, float3 shadowTexcoord, float3 shadowColor, float shadowBias)
{
	// �V���h�E�}�b�v����[�x�l�擾
    float depth = tex.Sample(samplerState, shadowTexcoord.xy).r;

	// �[�x�l���r���ĉe���ǂ����𔻒肷��
    float shadow = step(shadowTexcoord.z - depth, shadowBias);

    return lerp(shadowColor, 1, shadow);
}