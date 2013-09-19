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
	// colors
	//Nothing vec2 p = gl_FragCoord.xy/iResolution.xy;
	//RTE vec2 uv = gl_TexCoord[0].st* vec2(width,height);
	//RTE vec2 uv = gl_TexCoord[0].st;
	//RTE vec2 uv = gl_TexCoord[0].uv;
	//OK but flipped:
	vec2 p = gl_FragCoord.xy / vec2(5,3);
	

	vec4 col = texture2DRect(iChannel0, p);
	vec2 offset = vec2(.01,.0);
	col.r = texture2DRect(iChannel0, p+offset.xy).r;
	col.g = texture2DRect(iChannel0, p          ).g;
	col.b = texture2DRect(iChannel0, p+offset.yx).b;
	gl_FragColor = col;
	// RTE gl_FragColor = vec4(col,1);
	gl_FragColor.a = 1.0;

}