#pragma once
#include "Util.h"
#include "Color.h"
#include "Mesh.h"

namespace TX {
	class ObjMaterial {
	public:
		Color			ambient;
		Color			diffuse;
		Color			specular;
		Color			transmittance;
		Color			emission;
		float			shininess;
		float			ior;
		float			dissolve;

		std::string		name;
		std::string		texAmbient;
		std::string		texDiffuse;
		std::string		texSpecular;
		std::string		texSpecularHighlight;
		std::string		texBump;
		std::string		texDisplacement;
		std::string		texAlpha;

		ObjMaterial(const char *name = "") :
			name(name),
			ambient(0.75f, 0.75f, 0.75f),
			specular(0.f, 0.f, 0.f),
			transmittance(0.f, 0.f, 0.f),
			emission(0.f, 0.f, 0.f),
			shininess(0.f),
			ior(0.f),
			dissolve(1.f){}
	};

	class ObjMesh : public Mesh {
	public:
		std::vector<uint>		materials;
	public:
		ObjMesh() : Mesh() {}
		void LoadQuad(
			float length = 1.f,
			const Vec3& position = Vec3::ZERO,
			const Vec3& rotation = Vec3::ZERO,
			const Vec3& scale = Vec3::ONE);
		void LoadSphere(
			float radius,
			const Vec3& position = Vec3::ZERO,
			const Vec3& rotation = Vec3::ZERO,
			const Vec3& scale = Vec3::ONE);
		void Clear();
	};

	class ObjShape {
	public:
		std::string		name;
		ObjMesh			mesh;

		ObjShape(const char *name = "") :
			name(name){}
	};

	class ObjLoader {
	public:
		static void Load(
			std::vector<ObjShape>& objects,
			std::vector<ObjMaterial>& materials,
			const char *objFile, const char *mtlDir = nullptr);
		static void ConvertObj(const void *src, ObjShape& dest);
		static void ConvertMtl(const void *src, ObjMaterial& dest);
	};
}
