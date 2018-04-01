#ifndef BABYLON_PHYSICS_HELPER_PHYSICS_VORTEX_EVENT_H
#define BABYLON_PHYSICS_HELPER_PHYSICS_VORTEX_EVENT_H

#include <babylon/babylon_global.h>
#include <babylon/math/vector3.h>

namespace BABYLON {

/**
 * @brief Vortex.
 */
class BABYLON_SHARED_EXPORT PhysicsVortexEvent {

public:
  PhysicsVortexEvent(Scene* scene, Vector3 origin, float radius, float strength,
                     float height);
  ~PhysicsVortexEvent();

  /**
   * @brief Returns the data related to the vortex event (cylinder).
   * @returns {PhysicsVortexEventData}
   */
  PhysicsVortexEventData getData();

  /**
   * @brief Enables the vortex.
   */
  void enable();

  /**
   * @brief Disables the cortex.
   */
  void disable();

  /**
   * @brief Disposes the sphere.
   * @param {bolean} force
   */
  void dispose(bool force = true);

private:
  unique_ptr_t<PhysicsForceAndContactPoint>
  getImpostorForceAndContactPoint(PhysicsImpostor* impostor);

  void _tick();

  /*** Helpers ***/

  void _prepareCylinder();

  bool _intersectsWithCylinder(PhysicsImpostor* impostor);

private:
  Scene* _scene;
  Vector3 _origin;
  float _radius;
  float _strength;
  float _height;
  PhysicsEngine* _physicsEngine;
  // the most upper part of the cylinder
  Vector3 _originTop;
  // at which distance, relative to the radius the centripetal forces should
  // kick in
  float _centripetalForceThreshold;
  float _updraftMultiplier;
  ::std::function<void(Scene* scene, EventState& es)> _tickCallback;
  Mesh* _cylinder;
  // to keep the cylinders position, because normally the origin is in the
  // center and not on the bottom
  Vector3 _cylinderPosition;
  // check if the has been fetched the data. If not, do cleanup
  bool _dataFetched;

}; // end of class PhysicsVortexEvent

} // end of namespace BABYLON

#endif // end of BABYLON_PHYSICS_HELPER_PHYSICS_VORTEX_EVENT_H
