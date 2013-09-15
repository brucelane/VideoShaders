#version 110
#extension GL_ARB_texture_rectangle : enable
uniform vec3			iResolution;           // viewport resolution (in pixels)
uniform sampler2DRect	iChannel0;
uniform vec3			iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform int				width;
uniform int				height;
uniform float			iGlobalTime;           // shader playback time (in seconds)

vec3 sample(const int x, const int y)
{
	vec2 uv = (gl_FragCoord.xy + vec2(x, y)) / iResolution.xy;
	return texture2DRect(iChannel0, uv).xyz;
}
float luminance(vec3 c)
{
	return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

vec3 edge(void)
{
	vec3 hc =sample(-1,-1) *  1.0 + sample( 0,-1) *  2.0
		 	+sample( 1,-1) *  1.0 + sample(-1, 1) * -1.0
		 	+sample( 0, 1) * -2.0 + sample( 1, 1) * -1.0;
		
	vec3 vc =sample(-1,-1) *  1.0 + sample(-1, 0) *  2.0
		 	+sample(-1, 1) *  1.0 + sample( 1,-1) * -1.0
		 	+sample( 1, 0) * -2.0 + sample( 1, 1) * -1.0;
	
	return sample(0, 0) * pow(luminance(vc*vc + hc*hc), 0.6);
}

// main
void main()
{
	gl_FragColor = vec4(edge(), 1.0);
}