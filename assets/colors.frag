#version 110
//#extension GL_ARB_texture_rectangle : enable
uniform vec3			iResolution;           // viewport resolution (in pixels)
uniform sampler2D	iChannel0;
uniform vec3			iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform int				width;
uniform int				height;
uniform float			iGlobalTime;           // shader playback time (in seconds)

// main
void main()
{
	// colors
	//Nothing 
	//vec2 p = gl_FragCoord.xy/iResolution.xy;
	//RTE vec2 uv = gl_TexCoord[0].st* vec2(width,height);
	//RTE vec2 uv = gl_TexCoord[0].st;
	//RTE vec2 uv = gl_TexCoord[0].uv;
	//OK but flipped:
	//vec2 p = gl_FragCoord.xy / vec2(5,3);
	
	vec2 p = gl_FragCoord.xy / vec2(5,-3)  + vec2(0, 350);
	

	vec4 col = texture2D(iChannel0, p);
	vec2 offset = vec2(3.1,4.01);
	col.r = texture2D(iChannel0, p+offset.x).r;
	col.g = texture2D(iChannel0, p+2.0     ).g;
	col.b = texture2D(iChannel0, p+offset.y).b;
	gl_FragColor = col;
	gl_FragColor.a = 1.0;

}