/*
	Copyright 2020 Myles Trevino
	Licensed under the Apache License, Version 2.0
	https://www.apache.org/licenses/LICENSE-2.0
*/


#include "Exporter.hpp"

#include <iostream>
#include <filesystem>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

#include "Constants.hpp"
#include "Utilities.hpp"
#include "Generator.hpp"


namespace
{
	std::string format;
	std::string file_name;
	bool z_up;

	glm::fvec2 dft_size;
	LV::Mesh dft_mesh;
	LV::Mesh base_mesh;
	aiScene* scene;


	void populate_scene_mesh(unsigned scene_mesh_index,
		const std::string& mesh_name, const LV::Mesh& dft_mesh)
	{
		// Populate the vertices.
		aiMesh* mesh{scene->mMeshes[scene_mesh_index]};

		mesh->mName = mesh_name;

		mesh->mNumVertices = static_cast<unsigned>(dft_mesh.vertices.size());
		mesh->mVertices = new aiVector3D[mesh->mNumVertices];

		for(unsigned index{}; index < mesh->mNumVertices; ++index)
		{
			const glm::fvec3 vertex{dft_mesh.vertices[index]};

			if(z_up) mesh->mVertices[index] = aiVector3D(vertex.x, -vertex.z, vertex.y);
			else mesh->mVertices[index] = aiVector3D(vertex.x, vertex.y, vertex.z);
		}

		// Populate the indices.
		if(dft_mesh.indices.size()%3 != 0)
			throw std::runtime_error{"Failed to generate the export data."};

		mesh->mNumFaces = static_cast<unsigned>(dft_mesh.indices.size()/3);
		mesh->mFaces = new aiFace[mesh->mNumFaces];

		for(unsigned index{}; index < mesh->mNumFaces; ++index)
		{
			aiFace* face{&mesh->mFaces[index]};

			face->mIndices = new unsigned[3];
			face->mNumIndices = 3;

			const unsigned indicies_index{index*3};
			face->mIndices[0] = dft_mesh.indices[indicies_index];
			face->mIndices[1] = dft_mesh.indices[indicies_index+1];
			face->mIndices[2] = dft_mesh.indices[indicies_index+2];
		}
	}


	void generate_scene()
	{
		std::cout<<"Generating the export data...\n";

		// Create the scene and root node.
		scene = new aiScene();
		scene->mRootNode = new aiNode();

		// Create the material.
		scene->mNumMaterials = 1;
		scene->mMaterials = new aiMaterial*[scene->mNumMaterials];
		scene->mMaterials[0] = new aiMaterial;

		scene->mMaterials[0]->AddProperty(new aiString{
			LV::Constants::material_name}, AI_MATKEY_NAME);

		const glm::fvec3 color{LV::Constants::material_color};
		scene->mMaterials[0]->AddProperty(new aiColor3D{
			color.x, color.y, color.z}, 1, AI_MATKEY_COLOR_DIFFUSE);

		// Create the meshes.
		scene->mNumMeshes = 2;
		scene->mMeshes = new aiMesh*[scene->mNumMeshes];
		for(unsigned index{}; index < scene->mNumMeshes; ++index)
		{
			scene->mMeshes[index] = new aiMesh;
			scene->mMeshes[index]->mMaterialIndex = 0;

			if(index == 0) populate_scene_mesh(index, "DFT", dft_mesh);
			else if(index == 1) populate_scene_mesh(index, "Base", base_mesh);
		}

		// Link the meshes to the root node.
		scene->mRootNode->mNumMeshes = scene->mNumMeshes;
		scene->mRootNode->mMeshes = new unsigned int[scene->mRootNode->mNumMeshes];
		for(unsigned index{}; index < scene->mRootNode->mNumMeshes; ++index)
			scene->mRootNode->mMeshes[index] = index;
	}

	
	void export_scene()
	{
		std::cout<<"Exporting...\n";
		std::filesystem::create_directory(LV::Constants::exports_directory);

		std::string name{file_name};
		std::replace(name.begin(), name.end(), '.', '-');

		Assimp::Exporter exporter;
		if(exporter.Export(scene, format, LV::Constants::exports_directory+name+
			"."+format) != AI_SUCCESS) throw std::runtime_error{"Export failed."};
	}
}


void LV::Exporter::export_model(const std::string& file_name,
	const std::string& format, const std::string& orientation)
{
	::file_name = file_name;
	::format = format;

	// Validate the format.
	if(!LV::Utilities::is_supported(format, LV::Constants::supported_formats))
		throw std::runtime_error{"Unrecognized export format."};

	// Parse the Y-Up variable.
	if(orientation == "z-up") z_up = true;
	else if(orientation == "y-up") z_up = false;
	else throw std::runtime_error{"'orientation' must be either 'z-up' or 'y-up'."};
	
	// Generate the meshes.
	LV::Generator::generate(file_name);
	dft_size = LV::Generator::get_size();
	dft_mesh = LV::Generator::get_dft_mesh();
	base_mesh = LV::Generator::get_base_mesh();

	// Generate the Assimp scene.
	generate_scene();

	// Export the Assimp scene as the given format.
	export_scene();
	std::cout<<"Export finished.\n";

	// Destroy the scene.
	delete scene;
}
