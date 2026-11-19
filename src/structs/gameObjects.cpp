#include "structs/gameObjects.h"

#include <rlgl.h>

namespace
{
	// Draw a model with one explicit model matrix. Replaces DrawModelEx() bc it
	// builds and mutates its own transform, which doesn't work properly with 
	// GameObject hierarchy. DrawMesh takes the final matrix directly and leaves 
	// the raylib matrix stack untouched.
	void DrawModelWithMatrix(const Model& model, Matrix matrix, Color color, bool replaceColor)
	{
		for (int i = 0; i < model.meshCount; ++i)
		{
			Material material = model.materials[model.meshMaterial[i]];
			const Color diffuse = material.maps[MATERIAL_MAP_DIFFUSE].color;

			if (replaceColor)
			{
				material.maps[MATERIAL_MAP_DIFFUSE].color = color;
			}
			else
			{
				material.maps[MATERIAL_MAP_DIFFUSE].color = {
					(unsigned char)(((int)diffuse.r * (int)color.r) / 255),
					(unsigned char)(((int)diffuse.g * (int)color.g) / 255),
					(unsigned char)(((int)diffuse.b * (int)color.b) / 255),
					(unsigned char)(((int)diffuse.a * (int)color.a) / 255)
				};
			}

			DrawMesh(model.meshes[i], material, matrix);

			// restore original color
			material.maps[MATERIAL_MAP_DIFFUSE].color = diffuse;
		}
	}

	void DrawSilhouetteOutline(const Model& model, Matrix modelWorld, Color color)
	{
		constexpr float outlineScale = 1.12f;
		const Matrix outlineWorld = MatrixMultiply(
			MatrixScale(outlineScale, outlineScale, outlineScale),
			modelWorld
		);

		rlEnableBackfaceCulling();
		rlSetCullFace(RL_CULL_FACE_FRONT);
		DrawModelWithMatrix(model, outlineWorld, color, true);
		rlSetCullFace(RL_CULL_FACE_BACK);
	}

	void DrawBoxEdges(Matrix attachmentWorld, Color color)
	{
		constexpr float outlineInflation = 1.004f;

		rlPushMatrix();
			rlMultMatrixf(MatrixToFloat(attachmentWorld));
			DrawCubeWiresV(
				{ 0.f, 0.f, 0.f },
				{ outlineInflation, outlineInflation, outlineInflation },
				color
			);
		rlPopMatrix();
	}
}

void GameObject::Move(float deltaTime)
{
	transform.position += direction * speed * deltaTime;
}

void GameObject::Draw() const
{
	if (!bIsVisible) return;

	const Matrix objectWorld = WorldMatrix();

	for (const ModelAttachment& attachment : models)
	{
		if (attachment.model.meshCount <= 0 || !attachment.bIsVisible) continue;
		
		const Matrix attachmentWorld = MatrixMultiply(attachment.local.ToMatrix(), objectWorld);
		const Matrix modelWorld = MatrixMultiply(attachment.model.transform, attachmentWorld);

		// draw silhouette behind the solid
		if (attachment.outline && attachment.outlineStyle == OutlineStyle::Silhouette)
			DrawSilhouetteOutline(attachment.model, modelWorld, attachment.outlineColor);

		DrawModelWithMatrix(attachment.model, modelWorld, attachment.tint, false);

		// draw box edges after the solid
		if (attachment.outline && attachment.outlineStyle == OutlineStyle::Edges)
			DrawBoxEdges(attachmentWorld, attachment.outlineColor);
	}
}
