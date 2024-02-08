#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include "gltfbuilder.h"
#include "b3dm.h"
#include <fstream>
#include <limits>
#include <unordered_map>
#include <QDir>

using namespace std;

void createBufferAndAccessor(tinygltf::Model &modelGltf,
                             void *destBuffer,
                             const void *sourceBuffer,
                             size_t bufferIndex,
                             size_t bufferViewOffset,
                             size_t bufferViewLength,
                             int bufferViewTarget,
                             size_t accessorComponentCount,
                             int accessorComponentType,
                             int accessorType)
{
    std::memcpy(destBuffer, sourceBuffer, bufferViewLength);

    tinygltf::BufferView bufferViewGltf;
    bufferViewGltf.buffer = static_cast<int>(bufferIndex);
    bufferViewGltf.byteOffset = bufferViewOffset;
    bufferViewGltf.byteLength = bufferViewLength;
    bufferViewGltf.target = bufferViewTarget;

    tinygltf::Accessor accessorGltf;
    accessorGltf.bufferView = static_cast<int>(modelGltf.bufferViews.size());
    accessorGltf.byteOffset = 0;
    accessorGltf.count = accessorComponentCount;
    accessorGltf.componentType = accessorComponentType;
    accessorGltf.type = accessorType;

    modelGltf.bufferViews.emplace_back(bufferViewGltf);
    modelGltf.accessors.emplace_back(accessorGltf);
}

void createGltfTexture(const customTexture &texture,
                       tinygltf::Model &gltf,
                       std::unordered_map<tinygltf::Sampler, unsigned> *samplerCache)
{
    tinygltf::Sampler sampler;
    sampler.minFilter = texture.minFilter;
    sampler.magFilter = texture.magFilter;
    int samplerIndex = -1;
    gltf.samplers.emplace_back(sampler);
    samplerIndex = static_cast<int>(gltf.samplers.size() - 1);
    tinygltf::Texture textureGltf;
    textureGltf.sampler = samplerIndex;
    textureGltf.source = texture.source;
    gltf.textures.emplace_back(textureGltf);
}

void createGltfMaterial(tinygltf::Model &gltf, const customMaterial &material) {
    vcg::Point3f specularColor = material.specular;
    float specularIntensity = specularColor[0] * 0.2125f + specularColor[1] * 0.7154f
                              + specularColor[2] * 0.0721f;

    float roughnessFactor = material.shininess;
    roughnessFactor = material.shininess / 1000.0f;
    roughnessFactor = 1.0f - roughnessFactor;
    if(roughnessFactor < 0)
        roughnessFactor = 0;
    else if (roughnessFactor > 1)
        roughnessFactor = 1;

    if (specularIntensity < 0.0) {
        roughnessFactor *= (1.0f - specularIntensity);
    }

    tinygltf::Material materialGltf;
    if (material.texture != -1) {
        materialGltf.pbrMetallicRoughness.baseColorTexture.index = material.texture;
    }

    materialGltf.doubleSided = material.doubleSided;
    materialGltf.alphaMode = "MASK";

    // verifier quand cette condition est vérifiée

    if (!(material.diffuse[0] == 0 && material.diffuse[1] == 0 && material.diffuse[2] == 0)) {
        materialGltf.pbrMetallicRoughness.baseColorFactor = {material.diffuse[0],
                                                             material.diffuse[1],
                                                             material.diffuse[2],
                                                             material.alpha};
    }
    materialGltf.pbrMetallicRoughness.roughnessFactor = roughnessFactor;
    materialGltf.pbrMetallicRoughness.metallicFactor = 0.0;

    if (material.unlit) {
        materialGltf.extensions["KHR_materials_unlit"] = {};
    }

    gltf.materials.emplace_back(materialGltf);
}

bool GltfBuilder::writeNode(int node_index) {

    clearModel();
    nx::Node node = m_nexusStructure.nodes[node_index];
    customTexture customTex;
    customMaterial customMat;

    m_gltfModel.asset.version = "2.0";

    if (customMat.unlit) {
        m_gltfModel.extensionsUsed.emplace_back("KHR_materials_unlit");
    }

    // create root nodes
    tinygltf::Node rootNodeGltf;
    rootNodeGltf.matrix = {1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1};
    m_gltfModel.nodes.emplace_back(rootNodeGltf);
    int rootIndex = 0;

    // create buffer
    size_t totalBufferSize = node.nvert * m_nexusStructure.header.signature.vertex.size()
                           + node.nface * m_nexusStructure.header.signature.face.size();
    tinygltf::Buffer bufferGltf;
    auto &bufferData = bufferGltf.data;
    bufferData.resize(totalBufferSize);

    // add mesh ///////////////////////////////////////////////////////////

    tinygltf::Primitive primitiveGltf;
    primitiveGltf.mode = TINYGLTF_MODE_TRIANGLES;
    primitiveGltf.material = m_gltfModel.materials.size();

    size_t offset = 0;
    size_t nextSize = 0;
    auto bufferIndex = m_gltfModel.buffers.size();

    uint32_t chunk = node.offset;
    uchar *buffer  = m_nexusStructure.chunks.getChunk(chunk, false);

    if (node.nvert != 0) {
        vcg::Point3f *point = (vcg::Point3f *)buffer;
        nextSize = node.nvert * sizeof(vcg::Point3f);

        createBufferAndAccessor(m_gltfModel,
                                bufferData.data() + offset,
                                point,
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ARRAY_BUFFER,
                                node.nvert,
                                TINYGLTF_COMPONENT_TYPE_FLOAT,
                                TINYGLTF_TYPE_VEC3);

        auto box = m_nexusStructure.boxes[node_index].box;
        auto &positionsAccessor = m_gltfModel.accessors.back();
        positionsAccessor.minValues = {box.min[0], box.min[1], box.min[2]};
        positionsAccessor.maxValues = {box.max[0], box.max[1], box.max[2]};
        // positionsAccessor.minValues = {18.252765655517578, 21.16315269470215, -691.7335815429688};
        // positionsAccessor.maxValues = {51.3943977355957, 67.9502182006836, -647.491943359375};

        primitiveGltf.attributes["POSITION"] = static_cast<int>(m_gltfModel.accessors.size() - 1);
        offset += nextSize;
    }
    else {
        throw QString("No mesh data");
        return -1;
    }

    if(m_nexusStructure.header.signature.vertex.hasTextures()) {
        size_t uvOffset = sizeof(vcg::Point3f) * node.nvert;
        vcg::Point2f *uv = (vcg::Point2f *)(buffer + uvOffset);
        nextSize = node.nvert * sizeof(vcg::Point2f);

        createBufferAndAccessor(m_gltfModel,
                                bufferData.data() + offset,
                                uv,
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ARRAY_BUFFER,
                                node.nvert,
                                TINYGLTF_COMPONENT_TYPE_FLOAT,
                                TINYGLTF_TYPE_VEC2);
        primitiveGltf.attributes["TEXCOORD_0"] = static_cast<int>(m_gltfModel.accessors.size() - 1);
        offset += nextSize;

//#define DEBUG_UV
#ifdef DEBUG_UV
        for(int i = 0; i < node.nvert; i++)
        {
            vcg::Point2f UV = *(uv + i);
            float x = UV.X();
            float y = UV.Y();
            float fin = 8.0000001f;
        }
#endif
    }



    if(m_nexusStructure.header.signature.vertex.hasNormals()) {
        size_t normalOffset = sizeof(vcg::Point3f) + m_nexusStructure.header.signature.vertex.hasTextures() * sizeof(vcg::Point2f);
        vcg::Point3s *normal = (vcg::Point3s *)(buffer + normalOffset * node.nvert);

        // A IMPLEMENTER
    }

    if(m_nexusStructure.header.signature.vertex.hasColors()) {
        size_t vertexColorOffset = sizeof(vcg::Point3f) + m_nexusStructure.header.signature.vertex.hasTextures() * sizeof(vcg::Point2f)
                                   + m_nexusStructure.header.signature.vertex.hasNormals() * sizeof(vcg::Point3s);
        uchar *color =  buffer + vertexColorOffset;

        // IMPLEMENTER ???
    }

    if (node.nface !=0) {
        size_t facesOffset = m_nexusStructure.header.signature.vertex.size() * node.nvert;
        uint16_t *face = (uint16_t *)(buffer + facesOffset);

        nextSize = node.nface * 3 * sizeof(uint16_t);
        createBufferAndAccessor(m_gltfModel,
                                bufferData.data() + offset,
                                face,
                                bufferIndex,
                                offset,
                                nextSize,
                                TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER,
                                node.nface * 3,
                                TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,
                                TINYGLTF_TYPE_SCALAR);

        primitiveGltf.indices = static_cast<int>(m_gltfModel.accessors.size() - 1);
        offset += nextSize;
    }


    // add buffer to gltf model
    m_gltfModel.buffers.emplace_back(bufferGltf);

    // add image data to a second buffer
    // then add an image and a texture to gltf model
    if(m_nexusStructure.textures.size()) {

        if(m_nexusStructure.useNodeTex) {

            if(m_nexusStructure.header.signature.flags & nx::Signature::Flags::DEEPZOOM) {


                //int indice = m_nexusStructure.textures.size() - 2 - node_index;
                int indice = m_nexusStructure.patches[node.first_patch].texture;

                quint32 s = m_nexusStructure.textures[indice].offset * NEXUS_PADDING;
                quint32 size = m_nexusStructure.textures[indice + 1].offset * NEXUS_PADDING - s;
                m_nexusStructure.nodeTex.seek(s);
                auto buffer = m_nexusStructure.nodeTex.read(size);

                tinygltf::Buffer buffer_builder;
                auto &imageBufferData = buffer_builder.data;
                imageBufferData.resize(size);
                std::memcpy(imageBufferData.data(), buffer, size);
                auto bufferIndex = m_gltfModel.buffers.size();
                m_gltfModel.buffers.emplace_back(buffer_builder);


                tinygltf::BufferView bufferView_builder;
                bufferView_builder.buffer = bufferIndex;
                bufferView_builder.byteLength = size;
                auto imageBufferViewIndex = (int)m_gltfModel.bufferViews.size();
                m_gltfModel.bufferViews.push_back(bufferView_builder);

                tinygltf::Image img;
                // img.width = 1093;
                // img.height = 146;
                img.mimeType = "image/jpeg";
                img.bufferView = imageBufferViewIndex;
                int imageIndex = m_gltfModel.images.size();
                m_gltfModel.images.emplace_back(img);
                customTex.source = imageIndex;

                createGltfTexture(customTex, m_gltfModel, nullptr);
                auto textureIndex = m_gltfModel.textures.size() - 1;
                customMat.texture = static_cast<int>(textureIndex);

#define DEBUG_TEXTURE
#ifdef DEBUG_TEXTURE
                QString testFolder("textureTestFolder");
                QDir dir;
                dir.mkdir(testFolder);
                QString texfileName = QString("%1/%2.jpg").arg(testFolder).arg(node_index);
                QFile texfile(texfileName);
                if(!texfile.open(QFile::WriteOnly)) {
                    throw QString("Error while opening %1").arg(texfileName);
                }
                texfile.write(buffer);
                texfile.close();
#endif
            }
            else {

            }
        }
    }

    // add material
    createGltfMaterial(m_gltfModel, customMat);

    // add mesh
    tinygltf::Mesh meshGltf;
    meshGltf.primitives.emplace_back(primitiveGltf);
    m_gltfModel.meshes.emplace_back(meshGltf);

    // create node
    tinygltf::Node meshNode;
    meshNode.mesh = static_cast<int>(m_gltfModel.meshes.size() - 1);
    //meshNode.translation = {center.x, center.y, center.z};
    m_gltfModel.nodes.emplace_back(meshNode);

    // add node to the root
    m_gltfModel.nodes[rootIndex].children.emplace_back(m_gltfModel.nodes.size() - 1);

    // create scene
    tinygltf::Scene sceneGltf;
    sceneGltf.nodes.emplace_back(0);
    m_gltfModel.scenes.emplace_back(sceneGltf);


    //QString extension = QString("%1.gltf").arg(node_index);
    QString filename = QString("%1.b3dm").arg(node_index);
    QString folder("output");
    QDir dir;
    dir.mkdir(folder);
    QString path = QString("%1/%2").arg(folder).arg(filename);

#define DEBUG_GLTF
#ifdef DEBUG_GLTF
    QString filename2 = QString("%1.glb").arg(node_index);
    QString folder2("output/debug");
    dir.mkdir(folder2);
    QString path2 = QString("%1/%2").arg(folder2).arg(filename2);
    writeGltf(path2);
#endif
    return writeB3DM(path, node_index);

}

bool GltfBuilder::writeGltf(const QString &path) {


    tinygltf::TinyGLTF loader;
    return loader.WriteGltfSceneToFile(&m_gltfModel, path.toStdString(), true, true, true, true);

}

bool GltfBuilder::writeB3DM(const QString & path, int node_index) {
    tinygltf::TinyGLTF loader;

    // writing .glb file to output/temp%
    QString tempPath = QString("output/temp%1").arg(node_index);
    bool result = loader.WriteGltfSceneToFile(&m_gltfModel, tempPath.toStdString(), true, true, true, true);
    if(!result) { throw QString("Error : can't write gltf to file : output/temp"); }

    // looking for file size
    std::ifstream is(tempPath.toStdString(), std::ifstream::in|std::ifstream::binary);
    is.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize length = is.gcount();
    is.clear();
    is.seekg( 0, std::ifstream::beg);

    char *buffer = new char[length];
    is.read(buffer, length);
    is.close();

    b3dm b3dmFile(length);
    std::filebuf fb;
    fb.open(path.toStdString(), std::ios::out|std::ios::binary);
    std::ostream os(&fb);

    b3dmFile.writeToStream(os);
    os.write(buffer, length);
    os.write(b3dmFile.getEndPadding().c_str(), std::streamsize(b3dmFile.getEndPadding().size()));
    auto test = std::streamsize(b3dmFile.getEndPadding().size());
    fb.close();

     delete[] buffer;
     return true;
 }

bool GltfBuilder::generateTiles() {

    for(int i = 0; i < m_nexusStructure.nodes.size() - 1; i++)
    {
        writeNode(i);
    }
    return true;
}

