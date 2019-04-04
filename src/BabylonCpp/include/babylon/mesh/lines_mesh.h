#ifndef BABYLON_MESH_LINE_MESH_H
#define BABYLON_MESH_LINE_MESH_H

#include <babylon/babylon_api.h>
#include <babylon/math/color3.h>
#include <babylon/mesh/mesh.h>

namespace BABYLON {

class ShaderMaterial;
using ShaderMaterialPtr = std::shared_ptr<ShaderMaterial>;

/**
 * @brief Line mesh
 * @see https://doc.babylonjs.com/babylon101/parametric_shapes
 */
class BABYLON_SHARED_EXPORT LinesMesh : public Mesh {

public:
  template <typename... Ts>
  static LinesMeshPtr New(Ts&&... args)
  {
    auto mesh
      = std::shared_ptr<LinesMesh>(new LinesMesh(std::forward<Ts>(args)...));
    mesh->addToScene(mesh);

    return mesh;
  }
  ~LinesMesh() override;

  /**
   * @brief Returns the string "LineMesh".
   */
  const std::string getClassName() const override;

  Type type() const override;

  /**
   * @brief Hidden
   */
  void _bind(SubMesh* subMesh, const EffectPtr& effect,
             unsigned int fillMode) override;

  /**
   * @brief Hidden
   */
  void _draw(SubMesh* subMesh, int fillMode, size_t instancesCount = 0,
             bool alternate = false) override;

  PickingInfo intersects(Ray& ray, bool fastCheck = true) override;

  /**
   * @brief Disposes of the line mesh.
   * @param doNotRecurse If children should be disposed
   */
  void dispose(bool doNotRecurse               = false,
               bool disposeMaterialAndTextures = false) override;

  /**
   * @brief Returns a new LineMesh object cloned from the current one.
   */
  LinesMeshPtr clone(const std::string& name, Node* newParent = nullptr,
                     bool doNotCloneChildren = false);

protected:
  /**
   * @brief Creates a new LinesMesh.
   * @param name defines the name
   * @param scene defines the hosting scene
   * @param parent defines the parent mesh if any
   * @param source defines the optional source LinesMesh used to clone data from
   * @param doNotCloneChildren When cloning, skip cloning child meshes of
   * source, default False. When false, achieved by calling a clone(), also
   * passing False. This will make creation of children, recursive.
   * @param useVertexColor defines if this LinesMesh supports vertex color
   * @param useVertexAlpha defines if this LinesMesh supports vertex alpha
   */
  LinesMesh(const std::string& name, Scene* scene, Node* parent = nullptr,
            LinesMesh* source = nullptr, bool doNotCloneChildren = true,
            bool useVertexColor = false, bool useVertexAlpha = false);

  /**
   * Hidden
   */
  MaterialPtr& get_material() override;

  /**
   * Hidden
   */
  void set_material(const MaterialPtr& material) override;

  /**
   * Hidden
   */
  bool get_checkCollisions() const override;

  /**
   * @brief The intersection Threshold is the margin applied when intersection a
   * segment of the LinesMesh with a Ray.
   * This margin is expressed in world space coordinates, so its value may vary.
   * Default value is 0.1
   * @returns the intersection Threshold value.
   */
  float get_intersectionThreshold() const;

  /**
   * @brief The intersection Threshold is the margin applied when intersection a
   * segment of the LinesMesh with a Ray.
   * This margin is expressed in world space coordinates, so its value may vary.
   * @param value the new threshold to apply
   */
  void set_intersectionThreshold(float value);

public:
  float dashSize;
  float gapSize;

  /**
   * Color of the line (Default: White)
   */
  Color3 color;

  /**
   * Alpha of the line (Default: 1)
   */
  float alpha;

  /**
   * If vertex color should be applied to the mesh
   */
  bool useVertexColor;

  /**
   * If vertex alpha should be applied to the mesh
   */
  bool useVertexAlpha;

  /**
   * The margin applied when intersection a segment of the LinesMesh with a Ray
   */
  Property<LinesMesh, float> intersectionThreshold;

private:
  float _intersectionThreshold;
  MaterialPtr _colorShaderMaterial;
  ShaderMaterialPtr _colorShader;

}; // end of class LinesMesh

} // end of namespace BABYLON

#endif // end of BABYLON_MESH_LINE_MESH_H
