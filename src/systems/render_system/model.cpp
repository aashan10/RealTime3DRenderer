#include "model.h"
#include "render_defaults.h"
#include "shader_config.h"
#include "utils/model_loader.h"

namespace render_system {

/**
 * GLTF Terms
 *
 * 1) Buffer -> Compact data which contains all the geometric data and textures
 * of a model
 *
 * 2) BufferView -> It represent a part of the buffer, which might contain data
 * about a texture, geometric data such and position, normal, texture coords.
 * Can be used create VBOS
 *
 * 3) Accessor -> It contains information about layout and type of data
 * represented by BufferView. (also
 * Can be used to create vertex attrib pointers of vbos
 *
 * 4) Primitives -> They represent the sub-mesh that is actually rendered.
 * They contain information about the
 * attributes [POSITION, NROMAL, TEXT_COORDs with index respective accessor],
 * render mode [GL_POINTS, GL_TRIANGLES etc],
 * material [index to material],
 * indices [index to accessor]
 *
 * more:
 * 	https://www.khronos.org/files/gltf20-reference-guide.pdf
 *
 */

Model::Model(tinygltf::Model &modelData) {
  const tinygltf::Scene &scene = modelData.scenes[modelData.defaultScene];
  this->name = scene.name;
  // load all buffer view into vbos

  std::map<int, GLuint> vbos;

  // store all buffer view on vbos
  for (size_t i = 0; i < modelData.bufferViews.size(); i++) {
    const tinygltf::BufferView &bufferView = modelData.bufferViews[i];
    // invalid buffer view (NOT ARRAY_BUFFER or ELEMENT_ARRAY_BUFFER)
    if (bufferView.target == 0)
      continue;

    const tinygltf::Buffer &buffer = modelData.buffers[bufferView.buffer];

    GLuint vbo;
    glGenBuffers(1, &vbo);
    vbos[i] = vbo;
    glBindBuffer(bufferView.target, vbo);
    glBufferData(bufferView.target, bufferView.byteLength,
                 &buffer.data[0] + bufferView.byteOffset, GL_STATIC_DRAW);
  }

  // load meshes
  for (const tinygltf::Mesh &meshData : modelData.meshes) {
    this->meshes.push_back(processMesh(vbos, meshData, modelData));
  }

  // cleanup vbos
  for (size_t i = 0; i < vbos.size(); ++i)
    glDeleteBuffers(1, &vbos[i]);
}

Mesh Model::processMesh(const std::map<int, GLuint> &vbos,
                        const tinygltf::Mesh &meshData,
                        const tinygltf::Model &modelData) {
  Mesh mesh;
  mesh.name = meshData.name;
  glGenVertexArrays(1, &mesh.vao);
  glBindVertexArray(mesh.vao);

  // loop through mesh primitives
  for (size_t i = 0; i < meshData.primitives.size(); ++i) {
    const tinygltf::Primitive &primitive = meshData.primitives[i];
    // primitive attributes (Position, Normal, TexCoords)
    for (const auto &attrib : primitive.attributes) {
      // attibe pair<string, int> pair of attribute name and accessor index
      const tinygltf::Accessor &accessor = modelData.accessors[attrib.second];
      // bind respective accessor buffer view
      glBindBuffer(GL_ARRAY_BUFFER, vbos.at(accessor.bufferView));

      int size = 1;
      /*
       * Accessor type represents bufferView data type (VEC2, VEC3,...)
       * Except for SCALAR type id is same as type size
       */
      if (accessor.type != TINYGLTF_TYPE_SCALAR)
        size = accessor.type;

      int vaa = -1;
      if (attrib.first.compare("POSITION") == 0)
        vaa = shader_config::POSITION_LOC;
      if (attrib.first.compare("NORMAL") == 0)
        vaa = shader_config::NORMAL_LOC;
      if (attrib.first.compare("TEXCOORD_0") == 0)
        vaa = shader_config::TEXCOORD0_LOC;

      if (vaa > -1) {
        // calculate stride
        int byteStride =
            accessor.ByteStride(modelData.bufferViews[accessor.bufferView]);
        glEnableVertexAttribArray(vaa);
        glVertexAttribPointer(vaa, size, accessor.componentType,
                              accessor.normalized ? GL_TRUE : GL_FALSE,
                              byteStride, (void *)accessor.byteOffset);
      }
    }

    // primitive materials
    const tinygltf::Material &materialData =
        modelData.materials[primitive.material];
    std::unique_ptr<Material> material =
        processMaterial(materialData, modelData);

    // primitive indices
    const tinygltf::Accessor &indexAccessor =
        modelData.accessors[primitive.indices];

    // register primitives to our mesh
    mesh.primitives.emplace_back(
        Primitive(std::move(material), indexAccessor.componentType,
                  indexAccessor.count, (void *)indexAccessor.byteOffset));
  }
  glBindVertexArray(0);
  return mesh;
}

std::unique_ptr<Material>
Model::processMaterial(const tinygltf::Material &materialData,
                       const tinygltf::Model &modelData) {
  std::unique_ptr<Material> material = std::make_unique<Material>();
  std::unique_ptr<FlatMaterial> flatMaterial = std::make_unique<FlatMaterial>();
  const RenderDefaults &renderDefault = RenderDefaults::getInstance("");

  bool setFlatMaterial = false;
  const tinygltf::PbrMetallicRoughness &pbrInfo =
      materialData.pbrMetallicRoughness;
  int baseColorTextureIndex = pbrInfo.baseColorTexture.index;
  int normalTextureIndex = materialData.normalTexture.index;
  int emissionTextureIndex = materialData.emissiveTexture.index;
  int occlusionTextureIndex = materialData.occlusionTexture.index;
  int metallicRoughnessTextureIndex = pbrInfo.metallicRoughnessTexture.index;

  // TODO: Different material for mesh without texture coords
  if (baseColorTextureIndex != -1) {
    const tinygltf::Image &baseColorImage =
        modelData.images[modelData.textures[baseColorTextureIndex].source];
    material->albedo = processTexture(baseColorImage);
  } else {
    setFlatMaterial = true;
    material->albedo = renderDefault.getCheckerTexture();
    material->flatMaterial->albedo =
        glm::vec4(pbrInfo.baseColorFactor[0], pbrInfo.baseColorFactor[1],
                  pbrInfo.baseColorFactor[2], pbrInfo.baseColorFactor[3]);
    material->flatMaterial->emission = glm::vec4(
        materialData.emissiveFactor[0], materialData.emissiveFactor[1],
        materialData.emissiveFactor[2], materialData.emissiveFactor[3]);
    material->flatMaterial->ao = 1.0f;
    material->flatMaterial->metallic = pbrInfo.metallicFactor;
    material->flatMaterial->roughtness = pbrInfo.roughnessFactor;
  }
  if (normalTextureIndex != -1) {
    const tinygltf::Image &normalImage =
        modelData.images[modelData.textures[normalTextureIndex].source];
    material->normal = processTexture(normalImage);
  } else
    material->normal =
        renderDefault.getBlackTexture(); // TODO: Use default 1x1 texture
  if (emissionTextureIndex != -1) {
    const tinygltf::Image &emissionImage =
        modelData.images[modelData.textures[emissionTextureIndex].source];
    material->emission = processTexture(emissionImage);
  } else
    material->emission = renderDefault.getBlackTexture();
  if (metallicRoughnessTextureIndex != -1) {
    const tinygltf::Image &metallicRoughnessImage =
        modelData
            .images[modelData.textures[metallicRoughnessTextureIndex].source];
    material->metallicRoughness = processTexture(metallicRoughnessImage);
  } else
    material->metallicRoughness = renderDefault.getBlackTexture();
  if (occlusionTextureIndex != -1) {
    const tinygltf::Image &occlusionImage =
        modelData.images[modelData.textures[occlusionTextureIndex].source];
    material->ao = processTexture(occlusionImage);
  } else
    material->ao = renderDefault.getBlackTexture();

  material->shaderType = ShaderType::FORWARD_SHADER;
  if (setFlatMaterial)
    material->flatMaterial = std::move(flatMaterial);
  return material;
}

GLuint Model::processTexture(const tinygltf::Image &image) {
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GLenum format = GL_RGBA;
  if (image.component == 1)
    format = GL_RED;
  else if (image.component == 2)
    format = GL_RG;
  else if (image.component == 3)
    format = GL_RGB;

  GLenum type = GL_UNSIGNED_BYTE;
  if (image.bits == 9)
    ;
  else if (image.bits == 16)
    type = GL_UNSIGNED_SHORT;

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, format,
               type, &image.image.at(0));
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);

  return texId;
}
} // namespace render_system
