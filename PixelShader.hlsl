struct PixelShaderInput
{
    float4 Color : COLOR;
    // position attribute�� ������� ����
};

float4 main(PixelShaderInput IN) : SV_Target
{
    return IN.Color;
}