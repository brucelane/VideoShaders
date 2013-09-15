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
	// pass through
	vec2 uv = gl_TexCoord[0].st* vec2(width,height);
    gl_FragColor = texture2DRect(iChannel0, uv);
	gl_FragColor.a = 1.0;
}