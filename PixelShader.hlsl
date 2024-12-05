struct PixelShaderInput
{
    float4 Color : COLOR;
    // position attribute는 사용하지 않음
};

float4 main(PixelShaderInput IN) : SV_Target
{
    return IN.Color;
}