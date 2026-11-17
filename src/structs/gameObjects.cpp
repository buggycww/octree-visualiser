#include "structs/gameObjects.h"

#include <rlgl.h>

namespace
{
	// Draw a model with one explicit model matrix. This deliberately replaces
	// DrawModelEx(): DrawModelEx builds and mutates its own transform, which
	// does not compose cleanly with a GameObject hierarchy. DrawMesh takes the
	// final matrix directly and leaves the raylib matrix stack untouched.
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

			// DrawMesh does not copy the material; put the shared diffuse
			// colour back so other attachments using the same model are not
			// permanently tinted.
			material.maps[MATERIAL_MAP_DIFFUSE].color = diffuse;
		}
	}

	// Inverted-hull outline: draw a slightly enlarged copy with front faces
	// culled, then draw the real model over it. Only the border of the
	// enlarged hull remains visible.
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

	// Cheap 12-edge outline for the unit-cube tile model. The tiny inflation
	// keeps the lines outside the faces, avoiding z-fighting.
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

		// The silhouette has to be behind the solid, so draw it first.
		if (attachment.outline && attachment.outlineStyle == OutlineStyle::Silhouette)
			DrawSilhouetteOutline(attachment.model, modelWorld, attachment.outlineColor);

		DrawModelWithMatrix(attachment.model, modelWorld, attachment.tint, false);

		// Box edges sit on the surface, so draw them after the solid.
		if (attachment.outline && attachment.outlineStyle == OutlineStyle::Edges)
			DrawBoxEdges(attachmentWorld, attachment.outlineColor);
	}
}
