#version 410 core

subroutine vec4 GetColor();

subroutine uniform GetColor getColor;

out vec4 fragColor;

subroutine(GetColor)
vec4 redColor()
{
	return vec4(1.0, 0.0, 0.0, 1.0);
}

subroutine(GetColor)
vec4 greenColor()
{
	return vec4(0.0, 1.0, 0.0, 1.0);
}

subroutine(GetColor)
vec4 blueColor()
{
	return vec4(0.0, 0.0, 1.0, 1.0);
}

void main(void)
{
	fragColor = getColor();
}
