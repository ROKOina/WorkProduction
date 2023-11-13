#include "../../noise.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

Texture2D mainTex : register(t0);
Texture2D mainDepthTex : register(t1);
SamplerState samplerLiner : register(s0);


cbuffer ATMOSPHERE_CONSTANTS : register(b0)
{
    float4 mist_color;
    float2 mist_density; // x:extinction, y:inscattering
    float2 mist_height_falloff; // x:extinction, y:inscattering
    float2 height_mist_offset; // x:extinction, y:inscattering
	
    float mist_cutoff_distance;
	
    float mist_flow_speed;
    float mist_flow_noise_scale_factor;
    float mist_flow_density_lower_limit;
	
    float distance_to_sun;
    float sun_highlight_exponential_factor; // Affects the area of influence of the sun's highlights.
    float sun_highlight_intensity;

    float enabled;
    
    float2 dummy;
    
    row_major float4x4 view;
    row_major float4x4 invProj;
    row_major float4x4 invViewProj;
}

cbuffer CbScene : register(b1)
{
    float4 viewPosition;
    row_major float4x4 viewProjection;
    float4 ambientLightColor;
    float4 lightDirection;
    float4 lightColor;
};

float3 atmosphere(float3 fragment_color, float3 mist_color, float3 pixel_coord /*world space*/, float3 eye_coord /*world space*/)
{
	// Using 3d noise to represent the flow of mist.
    const float3 mist_flow_direction = float3(-1.0, -.2, -0.5);
    const float3 mist_flow_coord = pixel_coord.xyz + (mist_flow_direction * mist_flow_speed);
    //const float3 mist_flow_coord = pixel_coord.xyz + (mist_flow_direction * mist_flow_speed * time * wind_strength);
    const float flowing_density = lerp(mist_flow_density_lower_limit, 1.0, noise(fmod(mist_flow_coord * mist_flow_noise_scale_factor, 289)));

    float3 eye_to_pixel = pixel_coord - eye_coord;

	// z is the distance from the eye to the point
    float z = length(pixel_coord - eye_coord);
    z = smoothstep(0, mist_cutoff_distance, z) * z;

	// extinction and inscattering coefficients
    const float2 coefficients = mist_density * smoothstep(0.0, mist_height_falloff, height_mist_offset - pixel_coord.y) * flowing_density;

	// extinction and inscattering factors
    const float2 factors = exp(-z * coefficients);

    const float extinction = factors.x;
    const float inscattering = factors.y;
    fragment_color = fragment_color * extinction + mist_color * (1.0 - inscattering);

#if 1
	// Find the sum highlight and use it to blend the mist color
    float3 sun_position = -normalize(lightDirection.xyz) * distance_to_sun;
    float sun_highlight_factor = max(0, dot(normalize(eye_to_pixel), normalize(sun_position - eye_coord)));
	// Affects the area of influence of the sun's highlights.
    sun_highlight_factor = pow(sun_highlight_factor, sun_highlight_exponential_factor);

    const float near = 250.0; // The distance at which the effect begins to take effect.
    const float far = distance_to_sun; // The distance at which the effect reaches its maximum value.
    float3 sunhighlight_color = lerp(0, sun_highlight_intensity * (normalize(lightColor.rgb)), sun_highlight_factor * smoothstep(near, far, z));
    fragment_color += sunhighlight_color;
#endif

    return fragment_color;
}

float3 lens_flare(float2 uv, float2 pos)
{
#if 1
    const float glory_light_intensity = 1.5;
    const float lens_flare_intensity = 3.0;

    float2 main = uv - pos;
    float2 uvd = uv * (length(uv));

    float ang = atan2(main.x, main.y);
    float dist = length(main);
    dist = pow(dist, .1);
#if 1
    float n = noise(float2(ang * 16.0, dist * 32.0));
#else
	float n = noise_map.Sample(sampler_states[LINEAR], float2(ang * 16.0, dist * 32.0)).x;
#endif

	// Glory light
    float f0 = 1.0 / (length(uv - pos) * 16.0 + 1.0);
#if 1
    f0 = f0 + f0 * (sin(noise(sin(ang * 2. + pos.x) * 4.0 - cos(ang * 3. + pos.y)) * 16.) * .1 + dist * .1 + .8);
#else
	f0 = f0 + f0 * (sin(noise_map.Sample(sampler_states[LINEAR], float2(sin(ang * 2. + pos.x) * 4.0 - cos(ang * 3. + pos.y), 0.0)).x * 16.) * .1 + dist * .1 + .8);
#endif

	// Lens flare only 
    float f1 = max(0.01 - pow(length(uv + 1.2 * pos), 1.9), .0) * 7.0;

    float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), .0) * 0.25;
    float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), .0) * 0.23;
    float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), .0) * 0.21;

    float2 uvx = lerp(uv, uvd, -0.5);
	 
    float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), .0) * 6.0;
    float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), .0) * 5.0;
    float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), .0) * 3.0;

    uvx = lerp(uv, uvd, -.4);

    float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), .0) * 2.0;
    float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), .0) * 2.0;
    float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), .0) * 2.0;

    uvx = lerp(uv, uvd, -0.5);

    float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), .0) * 6.0;
    float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), .0) * 3.0;
    float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), .0) * 5.0;

    float3 c = 0;

    c.r += f2 + f4 + f5 + f6;
    c.g += f22 + f42 + f52 + f62;
    c.b += f23 + f43 + f53 + f63;
    c = max(0, c * 1.3 - (length(uvd) * .05));

    return f0 * glory_light_intensity + c * lens_flare_intensity;
#else
	// Lens flare only 
	float intensity = 1.5;
	float2 main = uv - pos;
	float2 uvd = uv * (length(uv));

	float dist = length(main); 
	dist = pow(dist, .1);

	float f1 = max(0.01/*max radius*/ - pow(length(uv + 1.2/*distance*/ * pos), 1.9/*boost*/), .0) * 7.0;

	float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), .0) * 00.1;
	float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), .0) * 00.08;
	float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), .0) * 00.06;

	float2 uvx = lerp(uv, uvd, -0.5);

	float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), .0) * 6.0;
	float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), .0) * 5.0;
	float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), .0) * 3.0;

	uvx = lerp(uv, uvd, -.4);

	float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), .0) * 2.0;
	float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), .0) * 2.0;
	float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), .0) * 2.0;

	uvx = lerp(uv, uvd, -0.5);

	float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), .0) * 6.0;
	float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), .0) * 3.0;
	float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), .0) * 5.0;

	float3 c = .0;

	c.r += f2 + f4 + f5 + f6; 
	c.g += f22 + f42 + f52 + f62; 
	c.b += f23 + f43 + f53 + f63;
	c = c * 1.3 - (length(uvd) * .05);

	return c * intensity;
#endif 
}

float3 cc(float3 color, float factor, float factor2) // color modifier
{
    float w = color.x + color.y + color.z;
    return lerp(color, w * factor, w * factor2);
}

float4 main(VS_OUT pin) : SV_TARGET
{
    //return float4(pin.texcoord.x, pin.texcoord.y, 0, 1);
 
    if(enabled>0)
    {
        float3 fragment_color = mainTex.Sample(samplerLiner, pin.texcoord).rgb;
        float alpha = pin.color.a;
    
        uint2 dimensions;
        uint mip_level = 0, number_of_samples;
        mainTex.GetDimensions(mip_level, dimensions.x, dimensions.y, number_of_samples);
        const float aspect = (float) dimensions.y / dimensions.x;

    
        float scene_depth = mainDepthTex.Sample(samplerLiner, pin.texcoord).x;
    
        float4 ndc_position;
    //texture space to ndc
        ndc_position.x = pin.texcoord.x * +2 - 1;
        ndc_position.y = pin.texcoord.y * -2 + 1;
        ndc_position.z = scene_depth;
        ndc_position.w = 1;

	//ndc to world space
        float4 world_position = mul(ndc_position, invViewProj);
        world_position = world_position / world_position.w;

    
	// Adapt atmosphere effects.
        fragment_color = atmosphere(fragment_color, mist_color.rgb * lightColor.rgb * lightColor.w, world_position.xyz, viewPosition.xyz);

	// Lens flare
        float4 ndc_sun_position = mul(float4(-normalize(lightDirection.xyz) * distance_to_sun, 1), viewProjection);
        ndc_sun_position /= ndc_sun_position.w;
        if (saturate(ndc_sun_position.z) == ndc_sun_position.z)
        {
            float4 occluder;
            occluder.xy = ndc_sun_position.xy;
            occluder.z = mainDepthTex.Sample(samplerLiner, float2(ndc_sun_position.x * 0.5 + 0.5, 0.5 - ndc_sun_position.y * 0.5)).x;
            occluder = mul(float4(occluder.xyz, 1), invProj);
            occluder /= occluder.w;
            float occluded_factor = step(250.0, occluder.z);

		//const float2 aspect_correct = float2(1.0, aspect);
            const float2 aspect_correct = float2(1.0 / aspect, 1.0);

            float sun_highlight_factor = max(0, dot(normalize(mul(world_position - viewPosition, view)).xyz, float3(0, 0, 1)));
            float3 lens_flare_color = float3(1.4, 1.2, 1.0) * lens_flare(ndc_position.xy * aspect_correct, ndc_sun_position.xy * aspect_correct);
            lens_flare_color -= noise(ndc_position.xy * 256) * .015;
            lens_flare_color = cc(lens_flare_color, .5, .1);
            fragment_color += max(0.0, lens_flare_color) * occluded_factor * lightColor.rgb * 0.5 /* * directional_light_color[0].w*/;
        }
    
        return float4(fragment_color, alpha);
    }
    else
    {
        return mainTex.Sample(samplerLiner, pin.texcoord);
    }
}