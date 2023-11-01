#include "Particle.hlsli"

VS_OUT main(uint vertexId : SV_VERTEXID)
{
    VS_OUT vout;
    vout.vertex_id = vertexId;
    return vout;
}