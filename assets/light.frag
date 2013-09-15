#version 110
#extension GL_ARB_texture_rectangle : enable
uniform vec3			iResolution;           // viewport resolution (in pixels)
uniform sampler2DRect	iChannel0;
uniform vec3			iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform int				width;
uniform int				height;
uniform float			iGlobalTime;           // shader playback time (in seconds)

// main
void main()
{
	// light
	vec2 uv = gl_TexCoord[0].st* vec2(width,height);
	vec2 lightPosition = iMouse.xy;
	float radius = 350.0;
    float distance  = length( lightPosition - gl_FragCoord.xy );
    float maxDistance = pow( radius, 0.21);
    float quadDistance = pow( distance, 0.23);
    float quadIntensity = 2.0 - min( quadDistance, maxDistance )/maxDistance;
	vec4 texture = texture2DRect(iChannel0, uv);
	gl_FragColor = texture * vec4(quadIntensity);
	gl_FragColor.a = 1.0;
}