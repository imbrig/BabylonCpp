#ifndef BABYLON_PARTICLES_POINTS_CLOUD_SYSTEM_H
#define BABYLON_PARTICLES_POINTS_CLOUD_SYSTEM_H

#include <memory>

#include <babylon/babylon_api.h>
#include <babylon/babylon_common.h>
#include <babylon/interfaces/idisposable.h>

namespace BABYLON {

class CloudPoint;
class Color4;
class Mesh;
class PointsGroup;
class Scene;
using CloudPointPtr = std::shared_ptr<CloudPoint>;
using MeshPtr       = std::shared_ptr<Mesh>;

struct PointsCloudSystemOptions {
  std::optional<bool> updatable = std::nullopt;
}; // end of struct PointsCloudSystemOptions

class BABYLON_SHARED_EXPORT PointsCloudSystem : public IDisposable {

public:
  /**
   * @brief Creates a PCS (Points Cloud System) object.
   * @param name (String) is the PCS name, this will be the underlying mesh name
   * @param pointSize (number) is the size for each point
   * @param scene (Scene) is the scene in which the PCS is added
   * @param options defines the options of the PCS e.g.
   * * updatable (optional boolean, default true) : if the PCS must be updatable or immutable
   */
  PointsCloudSystem(const std::string& name, size_t pointSize, Scene* scene,
                    const std::optional<PointsCloudSystemOptions>& options = std::nullopt);
  ~PointsCloudSystem(); // = default;

  /**
   * @brief Builds the PCS underlying mesh. Returns a standard Mesh.
   * If no points were added to the PCS, the returned mesh is just a single point.
   * @returns a promise for the created mesh
   */
  MeshPtr buildMeshSync();

private:
  /**
   * @hidden
   */
  MeshPtr _buildMesh();

  /**
   * @brief Adds a new particle object in the particles array.
   */
  CloudPointPtr _addParticle(size_t idx, PointsGroup* group, size_t groupId, size_t idxInGroup);

  void _randomUnitVector(CloudPoint& particle);
  Color4 _getColorIndicesForCoord(const PointsGroup& pointsGroup, uint32_t x, uint32_t y,
                                  uint32_t width) const;

public:
  /**
   *  The PCS array of cloud point objects. Just access each particle as with any classic array.
   *  Example : var p = SPS.particles[i];
   */
  std::vector<CloudPointPtr> particles;
  /**
   * The PCS total number of particles. Read only. Use PCS.counter instead if you need to set your
   * own value.
   */
  size_t nbParticles;
  /**
   * This a counter for your own usage. It's not set by any SPS functions.
   */
  size_t counter;
  /**
   * The PCS name. This name is also given to the underlying mesh.
   */
  std::string name;
  /**
   * The PCS mesh. It's a standard BJS Mesh, so all the methods from the Mesh class are avalaible.
   */
  MeshPtr mesh;
  /**
   * This empty object is intended to store some PCS specific or temporary values in order to lower
   * the Garbage Collector activity. Please read :
   */
  std::string vars;
  /**
   * @hidden
   */
  size_t _size; // size of each point particle

private:
  Scene* _scene;
  Float32Array _positions;
  Float32Array _indices;
  Float32Array _normals;
  Float32Array _colors;
  Float32Array _uvs;
  IndicesArray
    _indices32; // used as depth sorted array if depth sort enabled, else used as typed indices
  Float32Array _positions32; // updated positions for the VBO
  Float32Array _colors32;
  Float32Array _uvs32;
  bool _updatable;
  bool _isVisibilityBoxLocked;
  bool _alwaysVisible;
  IndicesArray _groups; // start indices for each group of particles
  size_t _groupCounter;
  bool _computeParticleColor;
  bool _computeParticleTexture;
  bool _computeParticleRotation;
  bool _computeBoundingBox;
  bool _isReady;

}; // end of class PointsCloudSystem

} // end of namespace BABYLON

#endif // end of BABYLON_PARTICLES_POINTS_CLOUD_SYSTEM_H
