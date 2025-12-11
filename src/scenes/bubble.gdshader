shader_type spatial;

// Adapted from shadertoy bubble shaders.

uniform float size = 0.05;

uniform vec4 outline_color : source_color = vec4( 0.45, 0.70, 1.0, 1.0 );
uniform float outline_ratio = 1.0;
uniform float noise_speed = 0.2;
uniform float noise_scale = 0.5;

// Standard hsv-rgb converter.
vec3 hsv2rgb( float h, float s, float v )
{
	return (
		( clamp( abs( fract( h + vec3( 0.0, 2.0, 1.0 ) / 3.0 ) * 6.0 - 3.0 ) - 1.0, 0.0, 1.0 ) - 1.0 ) * s + 1.0
	) * v;
}

// Standard pseudorandom one-liner (W.J.J. Rey, 22nd European Meeting of Statisticians, 1998).
float random( vec2 pos )
{
	return fract(sin(dot(pos, vec2(12.9898,78.233))) * 43758.5453);
}

void vertex( )
{
	vec3 v = clamp( cos( VERTEX * 10.0 + vec3( TIME ) ) + vec3( 1.0 ), 0.0, 1.0 );
	VERTEX *= 1.0 + dot( v, v ) * size * 0.5;
}

void fragment( )
{
	float outline_alpha = clamp( ( 1.0 - dot( NORMAL, VIEW ) ) * outline_ratio, 0.0, 1.0 );
	vec3 color = hsv2rgb( outline_alpha - 1.5 + noise_speed * TIME, 1.0, 1.0 ) * noise_scale;

	ALBEDO = mix(color, outline_color.rgb, max(outline_alpha - 0.3, 0.0));
	ALPHA = outline_color.a * outline_alpha;
}