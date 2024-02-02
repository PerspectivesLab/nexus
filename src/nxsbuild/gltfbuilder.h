#ifndef GLTFBUILDER_H
#define GLTFBUILDER_H

#include "deps/tiny_gltf.h"
#include "nexusbuilder.h"

struct customTexture {

    std::string uri = "";
    int source = -1;
    int minFilter = TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
    int magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;
};

struct customMaterial {
    vcg::Point3f specular {0, 0, 0};
    vcg::Point3f diffuse {0, 0, 0};
    float shininess  = 0;
    float alpha = 1;
    int texture = -1;
    bool doubleSided = true;
    bool unlit = true;

};

class GltfBuilder
{
public:
    GltfBuilder() = delete;
    GltfBuilder(NexusBuilder& nexus): m_nexusStructure(nexus) {};
    bool writeNode(int node_index, const QString &filename);
private:
    NexusBuilder &m_nexusStructure;
    tinygltf::Model m_gltfModel;
    bool writeGltf(const QString &filename);
    bool writeB3DM(const QString & filename);
    void clearModel() { m_gltfModel = tinygltf::Model();};
};

#endif // GLTFBUILDER_H
