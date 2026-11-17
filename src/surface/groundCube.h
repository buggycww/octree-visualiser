#pragma once

#include "structs/gameObjects.h"

class GroundCube : public GameObject
{
public:
	GroundCube(vec3::Vector3 scale, vec3::Vector3 pos, Color color, Color outlineColor, const Resources& res)
	{
		transform.position = pos;
		transform.scale = scale;

		ModelAttachment& body = Attach(res.cubeModel.Get(), {}, color);
		body.outline = true;
		body.outlineColor = outlineColor;
		body.outlineStyle = OutlineStyle::Edges;
	}
};
