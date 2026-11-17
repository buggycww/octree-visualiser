#pragma once

#include <raylib.h>
#include <vector>

struct ModelAsset
{
	// handles its own unloading logic
	explicit ModelAsset(Model model) : model(model) {}
	~ModelAsset() { UnloadModel(model); }

	ModelAsset(const ModelAsset&) = delete;
	ModelAsset& operator=(const ModelAsset&) = delete;

	const Model& Get() const { return model; }

private:
	Model model;
};

struct Resources
{
	Resources() = default;

	// A Resources object owns GPU objects (Model/Mesh), so copying it would mean two
	// objects claiming the same GPU handles and unloading them twice.
	Resources(const Resources&) = delete;
	Resources& operator=(const Resources&) = delete;

	ModelAsset cubeModel{ LoadModelFromMesh(GenMeshCube(1, 1, 1)) };
	ModelAsset cylinderModel{ LoadModelFromMesh(GenMeshCylinder(1, 1, 8)) };
	ModelAsset sphereModel{ LoadModelFromMesh(GenMeshSphere(1, 8, 8)) };
};