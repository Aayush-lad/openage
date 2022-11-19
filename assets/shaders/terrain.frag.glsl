#version 330

in vec2 tex_pos;

layout(location=0) out vec4 out_col;

uniform sampler2D tex;

void main()
{
    vec4 tex_val = texture(tex, tex_pos);
    if (tex_val.a == 0) {
		discard;
	}
	out_col = tex_val;
}
