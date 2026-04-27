struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

// Fullscreen triangle expanded from a single vertex id (no vertex buffer needed).
// vertex 0 -> uv (0,0), vertex 1 -> uv (2,0), vertex 2 -> uv (0,2).
VsOut main(uint vertexId : SV_VertexID)
{
    VsOut output;
    output.uv       = float2((vertexId << 1) & 2, vertexId & 2);
    output.position = float4(output.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return output;
}
