#version 110
#extension GL_ARB_texture_rectangle : enable
#define Resolution 500.
#define LightDir normalize(vec3(1.,0.,-1.))
#define Zoom .4
#define Height .2
#define Offset vec2(.3,.15)
#define ScanLines 100 // Should be adjusted depending on Height and Resolution
uniform vec3			iResolution;           // viewport resolution (in pixels)
uniform sampler2DRect	iChannel0;
uniform vec3			iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform int				width;
uniform int				height;
uniform float			iGlobalTime;           // shader playback time (in seconds)

float res=1./Resolution;

float terrain(vec2 p) {
	p*=Zoom;
	return texture2DRect(iChannel0,p).r*Height;
}

vec3 normal(vec2 p) {
	vec2 eps=vec2(0,res*2.);
	float d1=max(.003,terrain(p+eps.xy)-terrain(p-eps.xy));
	float d2=max(.003,terrain(p+eps.yx)-terrain(p-eps.yx));
	vec3 n1=(vec3(0.,eps.y*2.,d1));
	vec3 n2=(vec3(eps.y*2.,0.,d2));
	return normalize(cross(n1,n2));
}

// main
void main()
{
	// test
	// RTE vec2 uv = gl_FragCoord.xy / iResolution.xy;
	// Needs Flipping and scaling: 
	vec2 uv = (gl_FragCoord.xy + vec2(iResolution.x, iResolution.y));
	//vec2 uv = vec2(gl_FragCoord.x,gl_FragCoord.y) + vec2(x, y);
	uv=floor(uv*Resolution)/Resolution;
	vec2 p2=vec2(0.);
	float height=0.;
	for (int l=0; l<ScanLines; l++) {
		float scan=uv.y-float(l)*res;
		vec2 p=vec2(uv.x,scan*2.)+Offset/Zoom;
		float h=terrain(p);
		if (scan+h>uv.y) {p2=p;height=h;}
	}
	vec3 col=texture2DRect(iChannel0,p2*Zoom).rgb;
	col*=max(.2,dot(normal(p2),LightDir));
	col*=1.+pow(max(0.,dot(normal(p2),LightDir)),6.);
	col=mix(col,vec3(.8),pow(max(0.,length(vec3(uv.x*.8-.4,uv.y,height))),2.));
	gl_FragColor = vec4(col,1);
	gl_FragColor.a = 1.0;
}
