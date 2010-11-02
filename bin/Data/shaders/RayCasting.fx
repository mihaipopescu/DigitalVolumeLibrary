float4x4 WorldViewProj;

float3 SampleDist;
int Iterations;
float IsoValue;

float BaseSampleDist = .5f;
float ActualSampleDist = .5f;

int Side = 2;

float4 ScaleFactor;

float3 LightDir;

// fusion
float3 ROI_Params;  // Omega0, r0, a0
float3 ROI_Center;  // x, y, z

texture2D Front;
texture2D Back;
texture3D Volume;
texture2D Transfer;


sampler2D FrontS = sampler_state
{
	Texture = <Front>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	
	AddressU = Border;				// border sampling in U
    AddressV = Border;				// border sampling in V
    BorderColor = float4(0,0,0,0);	// outside of border should be black
};

sampler2D BackS = sampler_state
{
	Texture = <Back>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	
	AddressU = Border;				// border sampling in U
    AddressV = Border;				// border sampling in V
    BorderColor = float4(0,0,0,0);	// outside of border should be black
};

sampler3D VolumeS = sampler_state
{
	Texture = <Volume>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	
	AddressU = Border;				// border sampling in U
    AddressV = Border;				// border sampling in V
    AddressW = Border;
    BorderColor = float4(1,0,0,1);	// outside of border should be black
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


struct VertexShaderInput
{
    float4 Position : POSITION0;
};

struct VertexShaderOutput
{
    float4 Position		: POSITION0;
    float3 texC			: TEXCOORD0;
    float4 pos			: TEXCOORD1;
};

VertexShaderOutput PositionVS(VertexShaderInput input)
{
    VertexShaderOutput output;
	
	output.Position = mul(input.Position * ScaleFactor, WorldViewProj );
	   
    output.texC = input.Position + float4(0.5, 0.5, 0.5, 0);
    output.pos = output.Position;

    return output;
}


float4 PositionPS(VertexShaderOutput input) : COLOR0
{
    return float4(input.texC, 1.0f);
}

float4 WireFramePS(VertexShaderOutput input) : COLOR0
{
    return float4(1.0f, .5f, 0.0f, .85f);
}

//draws the front or back positions, or the ray direction through the volume
float4 DirectionPS(VertexShaderOutput input) : COLOR0
{
	float2 texC = input.pos.xy /= input.pos.w;
	texC.x =  0.5f*texC.x + 0.5f; 
	texC.y = -0.5f*texC.y + 0.5f;
	
    float3 front = tex2D(FrontS, texC).rgb;
    float3 back = tex2D(BackS, texC).rgb;
	
	if(Side == 0)
	{
		return float4(front, .9f);
	}
	if(Side == 1)
	{
		return float4(back, .9f);
	}
    
    return float4(back - front, .9f);
}


float ROI_Weight(float3 pos, float3 dir)
{
	float3 x0 = pos;
	float3 x1 = ROI_Center;
	float3 x2 = ROI_Center + dir;
	float r = length( cross(x0 - x1, x0 - x2) )/length(x2 - x1);
	float l = length( x0 - x1 );
	float a = sqrt( l*l - r*r );
	float w = ROI_Params.x*( 1 - r/ROI_Params.y - a/ROI_Params.z );
	
	if(w<0) w=1;
	return w;
}


float4 RayCastSimplePS(VertexShaderOutput input) : COLOR0
{ 
	//calculate projective texture coordinates
	//used to project the front and back position textures onto the cube
	float2 texC = input.pos.xy /= input.pos.w;
	texC.x =  0.5*texC.x + 0.5; 
	texC.y = -0.5*texC.y + 0.5;
	
    float3 front = tex2D(FrontS, texC).xyz;
    float3 back = tex2D(BackS, texC).xyz;
    
    float3 dir = normalize(back - front);
    float4 pos = float4(front, 0);
    
    float4 dst = float4(0, 0, 0, 0);
    float4 src = 0;
    
    float4 value = 0;
	
	float3 Step = dir * SampleDist;
    
  
	for(int i = 0; i < Iterations; i++)
	{
		pos.w = 0;
		value = tex3Dlod(VolumeS, pos);
			
		if((value.a * 255.0f) >= IsoValue)
		{
			//index the transfer function with the iso-value (value.a)
			//and get the rgba value for the voxel
			src = tex1Dlod(TransferS, value.a);

			//Oppacity correction: As sampling distance decreases we get more samples.
			//Therefore the alpha values set for a sampling distance of .5f will be too
			//high for a sampling distance of .25f (or conversely, too low for a sampling
			//distance of 1.0f). So we have to adjust the alpha accordingly.
			src.a = 1 - pow(abs(1 - src.a), ActualSampleDist / BaseSampleDist);
			
			//ROI
			//src *= ROI_Weight(pos.xyz, Step);

			//Front to back blending
			// dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
			// dst.a   = dst.a   + (1 - dst.a) * src.a		
			src.rgb *= src.a;
			dst = (1.0f - dst.a)*src + dst;		
			
#ifdef DYN_BRANCHING
			//break from the loop when alpha gets high enough
			if(dst.a >= .95f)
				break;	
#endif
		}

		//advance the current position
		pos.xyz += Step;
		
#ifdef DYN_BRANCHING		
		//break if the position is greater than <1, 1, 1>
		if(pos.x > 1.0f || pos.y > 1.0f || pos.z > 1.0f)
			break;
#endif
    }
    
    return dst;
}

technique RenderPosition
{
    pass Pass1
    {		
        VertexShader = compile vs_2_0 PositionVS();
        PixelShader = compile ps_2_0 PositionPS();
    }
}


technique Raycasting
{
    pass Pass1
    {	
        VertexShader = compile vs_3_0 PositionVS();		
        PixelShader = compile ps_3_0 RayCastSimplePS();
    }
}