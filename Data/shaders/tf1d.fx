float4x4 matWorld;
float4x4 matView;
float4x4 matProj;
float4x4 matTex;

texture3D Volume;
texture2D Transfer;

float4 ScaleFactor;
float IsoValue;

sampler3D VolumeS = sampler_state
{
	Texture = <Volume>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	
	AddressU = Clamp;				// border sampling in U
    AddressV = Clamp;				// border sampling in V
    AddressW = Clamp;				// border sampling in W
    BorderColor = float4(0,0,0,0);	// outside of border should be black
};

sampler1D TransferS = sampler_state
{
	Texture = <Transfer>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	
	AddressU  = CLAMP;
    AddressV  = CLAMP;
};


struct VS_INPUT
{
	float4 Position : POSITION0;
	float3 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position : POSITION0;
	float3 Tex : TEXCOORD0;

};

VS_OUTPUT mainVS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul( mul( mul( input.Position * ScaleFactor, matWorld ), matView ), matProj );
	output.Tex = mul( float4(input.Tex, 1), matTex ).xyz;
	
	return output;
}

float4 mainPS(VS_OUTPUT input) : COLOR0
{
  float4 v = tex3D(VolumeS, input.Tex);  		// Read 3D data texture
 
  if(v.a * 255.f < IsoValue) v.a = 0.f;			// selecting isosurface
  
  return float4(tex1D(TransferS, v.a).rgb, v.a);
}

float4 WirePS(VS_OUTPUT input) : COLOR0
{
	return float4(0,0,0,1);
}

technique TransferFn1D
{
	pass P0
	{
		AlphaBlendEnable = true;
		DestBlend = INVSRCALPHA;
		SrcBlend = SRCALPHA;
		ZEnable = false;
		FillMode = Solid;
		
		VertexShader = compile vs_3_0 mainVS();
		PixelShader = compile ps_3_0 mainPS();
	}
}
