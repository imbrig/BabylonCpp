#include <babylon/physics/physics_impostor.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/bones/bone.h>
#include <babylon/core/logging.h>
#include <babylon/culling/bounding_box.h>
#include <babylon/culling/bounding_info.h>
#include <babylon/engine/scene.h>
#include <babylon/math/vector3.h>
#include <babylon/mesh/abstract_mesh.h>
#include <babylon/mesh/mesh.h>
#include <babylon/physics/iphysics_enabled_object.h>
#include <babylon/physics/iphysics_engine_plugin.h>
#include <babylon/physics/joint/physics_joint.h>
#include <babylon/physics/physics_engine.h>

namespace BABYLON {

const Vector3 PhysicsImpostor::DEFAULT_OBJECT_SIZE = Vector3::One();

Quaternion PhysicsImpostor::IDENTITY_QUATERNION = Quaternion::Identity();

array_t<Vector3, 3> PhysicsImpostor::_tmpVecs
  = {{Vector3::Zero(), Vector3::Zero(), Vector3::Zero()}};
Quaternion PhysicsImpostor::_tmpQuat = Quaternion::Identity();

PhysicsImpostor::PhysicsImpostor(IPhysicsEnabledObject* _object,
                                 unsigned int iType,
                                 PhysicsImpostorParameters& options,
                                 Scene* scene)
    : object{_object}
    , isDisposed{this, &PhysicsImpostor::get_isDisposed}
    , mass{this, &PhysicsImpostor::get_mass, &PhysicsImpostor::set_mass}
    , friction{this, &PhysicsImpostor::get_friction,
               &PhysicsImpostor::set_friction}
    , restitution{this, &PhysicsImpostor::get_restitution,
                  &PhysicsImpostor::set_restitution}
    , _type{iType}
    , _options{options}
    , _scene{scene}
    , _bodyUpdateRequired{false}
    , _deltaPosition{Vector3::Zero()}
    , _isDisposed{false}
{
  // Sanity check!
  if (!object) {
    BABYLON_LOG_ERROR("PhysicsImpostor",
                      "No object was provided. A physics object is obligatory");
    return;
  }

  // legacy support for old syntax.
  if (!_scene && object->getScene()) {
    _scene = object->getScene();
  }

  if (!_scene) {
    return;
  }

  _physicsEngine = _scene->getPhysicsEngine();
  if (!_physicsEngine) {
    BABYLON_LOG_ERROR("PhysicsImpostor",
                      "Physics not enabled. Please use "
                      "scene.enablePhysics(...) before creating impostors.");
  }
  else {
    // Set the object's quaternion, if not set
    if (!object->rotationQuaternion()) {
      if (object->rotation() == Vector3::Zero()) {
        object->setRotationQuaternion(Quaternion::RotationYawPitchRoll(
          object->rotation().y, object->rotation().x, object->rotation().z));
      }
      else {
        object->setRotationQuaternion(Quaternion());
      }
    }
    // Default options params
    _options.mass = (_options.mass == nullptr) ? 0.f : *_options.mass;
    _options.friction
      = (_options.friction == nullptr) ? 0.2f : *_options.friction;
    _options.restitution
      = (_options.restitution == nullptr) ? 0.2f : *_options.restitution;

    // If the mesh has a parent, don't initialize the physicsBody. Instead wait
    // for the parent to do that.
    if (!object->getParent() || _options.ignoreParent) {
      _init();
    }
  }
}

PhysicsImpostor::~PhysicsImpostor()
{
}

void PhysicsImpostor::_init()
{
  if (!_physicsEngine) {
    return;
  }

  _physicsEngine->removeImpostor(this);
  _physicsBody = nullptr;
  _parent      = (_parent == nullptr) ? _getPhysicsParent() : _parent;
  if (!_isDisposed && (!parent() || _options.ignoreParent)) {
    _physicsEngine->addImpostor(this);
  }
}

PhysicsImpostor* PhysicsImpostor::_getPhysicsParent()
{
  if (object->parent()->type() == IReflect::Type::ABSTRACTMESH) {
    auto parentMesh = static_cast<AbstractMesh*>(object->parent());
    return parentMesh->physicsImpostor.get();
  }
  return nullptr;
}

bool PhysicsImpostor::isBodyInitRequired() const
{
  return _bodyUpdateRequired || (!_physicsBody && !_parent);
}

void PhysicsImpostor::setScalingUpdated()
{
  forceUpdate();
}

void PhysicsImpostor::forceUpdate()
{
  _init();
  if (parent() && !_options.ignoreParent) {
    parent()->forceUpdate();
  }
}

IPhysicsBody* PhysicsImpostor::physicsBody()
{
  return (_parent && !_options.ignoreParent) ? _parent->physicsBody() :
                                               _physicsBody;
}

PhysicsImpostor* PhysicsImpostor::parent()
{
  return !_options.ignoreParent && _parent ? _parent : nullptr;
}

void PhysicsImpostor::setParent(PhysicsImpostor* value)
{
  _parent = value;
}

unsigned int PhysicsImpostor::type() const
{
  return _type;
}

void PhysicsImpostor::setPhysicsBody(IPhysicsBody* physicsBody)
{
  if (_physicsBody && _physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->removePhysicsBody(this);
  }
  _physicsBody = physicsBody;
  resetUpdateFlags();
}

void PhysicsImpostor::resetUpdateFlags()
{
  _bodyUpdateRequired = false;
}

Vector3 PhysicsImpostor::getObjectExtendSize()
{
  if (object->hasBoundingInfo()) {
    auto& q = object->rotationQuaternion();
    // reset rotation
    object->setRotationQuaternion(PhysicsImpostor::IDENTITY_QUATERNION);
    // calculate the world matrix with no rotation
    object->computeWorldMatrix();
    object->computeWorldMatrix(true);
    auto& boundingInfo = object->getBoundingInfo();
    auto size          = boundingInfo.boundingBox.extendSizeWorld.scale(2.f);

    // bring back the rotation
    object->setRotationQuaternion(*q);
    // calculate the world matrix with the new rotation
    object->computeWorldMatrix();
    object->computeWorldMatrix(true);

    return size;
  }
  else {
    return PhysicsImpostor::DEFAULT_OBJECT_SIZE;
  }
}

Vector3 PhysicsImpostor::getObjectCenter()
{
  if (object->hasBoundingInfo()) {
    const auto& boundingInfo = object->getBoundingInfo();
    return boundingInfo.boundingBox.centerWorld;
  }
  else {
    return object->position();
  }
}

float PhysicsImpostor::getParam(const string_t& paramName) const
{
  if (_options.contains(paramName)) {
    return _options[paramName];
  }

  return 0.f;
}

void PhysicsImpostor::setParam(const string_t& paramName, float value)
{
  _options.setValue(paramName, value);
  _bodyUpdateRequired = true;
}

bool PhysicsImpostor::get_isDisposed() const
{
  return _isDisposed;
}

float PhysicsImpostor::get_mass() const
{
  return _physicsEngine ?
           _physicsEngine->getPhysicsPlugin()->getBodyMass(this) :
           0.f;
}

void PhysicsImpostor::set_mass(float value)
{
  setMass(value);
}

float PhysicsImpostor::get_friction() const
{
  return _physicsEngine ?
           _physicsEngine->getPhysicsPlugin()->getBodyFriction(this) :
           0;
}

void PhysicsImpostor::set_friction(float value)
{
  if (!_physicsEngine) {
    return;
  }
  _physicsEngine->getPhysicsPlugin()->setBodyFriction(this, value);
}

float PhysicsImpostor::get_restitution() const
{
  return _physicsEngine ?
           _physicsEngine->getPhysicsPlugin()->getBodyRestitution(this) :
           0;
}

void PhysicsImpostor::set_restitution(float value)
{
  if (!_physicsEngine) {
    return;
  }
  _physicsEngine->getPhysicsPlugin()->setBodyRestitution(this, value);
}

void PhysicsImpostor::setMass(float mass)
{
  if (stl_util::almost_equal(getParam("mass"), mass)) {
    setParam("mass", mass);
  }
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->setBodyMass(this, mass);
  }
}

Vector3 PhysicsImpostor::getLinearVelocity()
{
  return _physicsEngine ?
           _physicsEngine->getPhysicsPlugin()->getLinearVelocity(this) :
           Vector3::Zero();
}

void PhysicsImpostor::setLinearVelocity(const Vector3& velocity)
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->setLinearVelocity(this, velocity);
  }
}

Vector3 PhysicsImpostor::getAngularVelocity()
{
  return _physicsEngine ?
           _physicsEngine->getPhysicsPlugin()->getAngularVelocity(this) :
           Vector3::Zero();
}

void PhysicsImpostor::setAngularVelocity(const Vector3& velocity)
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->setAngularVelocity(this, velocity);
  }
}

void PhysicsImpostor::executeNativeFunction(
  const ::std::function<void(Mesh* world, IPhysicsBody* physicsBody)>& func)
{
  if (_physicsEngine) {
    func(_physicsEngine->getPhysicsPlugin()->world, physicsBody());
  }
}

void PhysicsImpostor::registerBeforePhysicsStep(
  const ::std::function<void(PhysicsImpostor* impostor)>& func)
{
  _onBeforePhysicsStepCallbacks.emplace_back(func);
}

void PhysicsImpostor::unregisterBeforePhysicsStep(
  const ::std::function<void(PhysicsImpostor* impostor)>& func)
{
  using Function = ::std::function<void(PhysicsImpostor * impostor)>;

  _onBeforePhysicsStepCallbacks.erase(
    ::std::remove_if(_onBeforePhysicsStepCallbacks.begin(),
                     _onBeforePhysicsStepCallbacks.end(),
                     [&func](const Function& f) {
                       auto ptr1 = func.template target<Function>();
                       auto ptr2 = f.template target<Function>();
                       return ptr1 < ptr2;
                     }),
    _onBeforePhysicsStepCallbacks.end());
}

void PhysicsImpostor::registerAfterPhysicsStep(
  const ::std::function<void(PhysicsImpostor* impostor)>& func)
{
  _onAfterPhysicsStepCallbacks.emplace_back(func);
}

void PhysicsImpostor::unregisterAfterPhysicsStep(
  const ::std::function<void(PhysicsImpostor* impostor)>& func)
{
  using Function = ::std::function<void(PhysicsImpostor * impostor)>;

  _onAfterPhysicsStepCallbacks.erase(
    ::std::remove_if(_onAfterPhysicsStepCallbacks.begin(),
                     _onAfterPhysicsStepCallbacks.end(),
                     [&func](const Function& f) {
                       auto ptr1 = func.template target<Function>();
                       auto ptr2 = f.template target<Function>();
                       return ptr1 < ptr2;
                     }),
    _onAfterPhysicsStepCallbacks.end());
}

void PhysicsImpostor::registerOnPhysicsCollide()
{
}

void PhysicsImpostor::unregisterOnPhysicsCollide()
{
}

void PhysicsImpostor::getParentsRotation()
{
}

void PhysicsImpostor::beforeStep()
{
  if (!_physicsEngine) {
    return;
  }

  object->translate(_deltaPosition, -1.f);
  if (_deltaRotationConjugated && object->rotationQuaternion()) {
    object->rotationQuaternion()->multiplyToRef(*_deltaRotationConjugated,
                                                *object->rotationQuaternion());
  }
  object->computeWorldMatrix(false);
  if (object->parent() && object->rotationQuaternion()) {
    getParentsRotation();
    _tmpQuat.multiplyToRef(*object->rotationQuaternion(), _tmpQuat);
  }
  else {
    _tmpQuat.copyFrom(object->rotationQuaternion() ?
                        *object->rotationQuaternion() :
                        Quaternion());
  }
  if (!_options.disableBidirectionalTransformation) {
    if (object->rotationQuaternion()) {
      _physicsEngine->getPhysicsPlugin()->setPhysicsBodyTransformation(
        this, /*bInfo.boundingBox.centerWorld*/ object->getAbsolutePivotPoint(),
        _tmpQuat);
    }
  }

  for (auto& func : _onBeforePhysicsStepCallbacks) {
    func(this);
  }
}

void PhysicsImpostor::afterStep()
{
  if (!_physicsEngine) {
    return;
  }

  for (auto& func : _onAfterPhysicsStepCallbacks) {
    func(this);
  }

  _physicsEngine->getPhysicsPlugin()->setTransformationFromPhysicsBody(this);
  // object has now its world rotation. needs to be converted to local.
  if (object->parent() && object->rotationQuaternion()) {
    getParentsRotation();
    _tmpQuat.conjugateInPlace();
    _tmpQuat.multiplyToRef(*object->rotationQuaternion(),
                           *object->rotationQuaternion());
  }
  // take the position set and make it the absolute position of this object.
  object->setAbsolutePosition(object->position());
  if (_deltaRotation && object->rotationQuaternion()) {
    object->rotationQuaternion()->multiplyToRef(*_deltaRotation,
                                                *object->rotationQuaternion());
  }
  object->translate(_deltaPosition, 1.f);
}

void PhysicsImpostor::onCollide(IPhysicsBody* /*body*/)
{
}

PhysicsImpostor& PhysicsImpostor::applyForce(const Vector3& force,
                                             const Vector3& contactPoint)
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->applyForce(this, force, contactPoint);
  }

  return *this;
}

PhysicsImpostor& PhysicsImpostor::applyImpulse(const Vector3& force,
                                               const Vector3& contactPoint)
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->applyImpulse(this, force, contactPoint);
  }

  return *this;
}

PhysicsImpostor& PhysicsImpostor::createJoint(PhysicsImpostor* otherImpostor,
                                              unsigned int jointType,
                                              const PhysicsJointData& jointData)
{
  addJoint(otherImpostor,
           ::std::make_shared<PhysicsJoint>(jointType, jointData));

  return *this;
}

PhysicsImpostor&
PhysicsImpostor::addJoint(PhysicsImpostor* otherImpostor,
                          const shared_ptr_t<PhysicsJoint>& joint)
{
  Joint _joint;
  _joint.otherImpostor = otherImpostor;
  _joint.joint         = joint;
  _joints.emplace_back(_joint);

  if (_physicsEngine) {
    _physicsEngine->addJoint(this, otherImpostor, joint);
  }

  return *this;
}

PhysicsImpostor& PhysicsImpostor::sleep()
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->sleepBody(this);
  }

  return *this;
}

PhysicsImpostor& PhysicsImpostor::wakeUp()
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->wakeUpBody(this);
  }

  return *this;
}

unique_ptr_t<PhysicsImpostor>
PhysicsImpostor::clone(IPhysicsEnabledObject* newObject)
{
  if (!newObject) {
    return nullptr;
  }
  return ::std::make_unique<PhysicsImpostor>(newObject, _type, _options,
                                             _scene);
}

void PhysicsImpostor::dispose()
{
  // No dispose if no physics engine is available.
  if (!_physicsEngine) {
    return;
  }

  for (auto& j : _joints) {
    if (_physicsEngine) {
      _physicsEngine->removeJoint(this, j.otherImpostor, j.joint.get());
    }
  }

  // Dispose the physics body
  _physicsEngine->removeImpostor(this);
  if (parent()) {
    parent()->forceUpdate();
  }

  _isDisposed = true;
}

void PhysicsImpostor::setDeltaPosition(const Vector3& position)
{
  _deltaPosition.copyFrom(position);
}

void PhysicsImpostor::setDeltaRotation(const Quaternion& rotation)
{
  if (!_deltaRotation) {
    _deltaRotation = ::std::make_unique<Quaternion>();
  }
  _deltaRotation->copyFrom(rotation);
  _deltaRotationConjugated->copyFrom(_deltaRotation->conjugate());
}

PhysicsImpostor& PhysicsImpostor::getBoxSizeToRef(Vector3& result)
{
  if (_physicsEngine) {
    _physicsEngine->getPhysicsPlugin()->getBoxSizeToRef(this, result);
  }

  return *this;
}

float PhysicsImpostor::getRadius() const
{
  return _physicsEngine ? _physicsEngine->getPhysicsPlugin()->getRadius(this) :
                          0.f;
}

void PhysicsImpostor::syncBoneWithImpostor(
  Bone* bone, AbstractMesh* boneMesh, const Nullable<Vector3>& jointPivot,
  Nullable<float> distToJoint, const Nullable<Quaternion>& adjustRotation)
{
  auto& tempVec = PhysicsImpostor::_tmpVecs[0];
  auto mesh     = static_cast<AbstractMesh*>(object);

  if (mesh->rotationQuaternion()) {
    if (adjustRotation) {
      auto& tempQuat = PhysicsImpostor::_tmpQuat;
      mesh->rotationQuaternion()->multiplyToRef(*adjustRotation, tempQuat);
      bone->setRotationQuaternion(tempQuat, Space::WORLD, boneMesh);
    }
    else {
      bone->setRotationQuaternion(*mesh->rotationQuaternion(), Space::WORLD,
                                  boneMesh);
    }
  }

  tempVec.x = 0;
  tempVec.y = 0;
  tempVec.z = 0;

  if (jointPivot) {
    auto _jointPivot = *jointPivot;
    tempVec.x        = _jointPivot.x;
    tempVec.y        = _jointPivot.y;
    tempVec.z        = _jointPivot.z;

    bone->getDirectionToRef(tempVec, tempVec, boneMesh);

    if (distToJoint.isNull()) {
      distToJoint = (*jointPivot).length();
    }

    tempVec.x *= distToJoint;
    tempVec.y *= distToJoint;
    tempVec.z *= distToJoint;
  }

  if (bone->getParent()) {
    tempVec.addInPlace(mesh->getAbsolutePosition());
    bone->setAbsolutePosition(tempVec, boneMesh);
  }
  else {
    boneMesh->setAbsolutePosition(mesh->getAbsolutePosition());
    boneMesh->position().x -= tempVec.x;
    boneMesh->position().y -= tempVec.y;
    boneMesh->position().z -= tempVec.z;
  }
}

void PhysicsImpostor::syncImpostorWithBone(
  Bone* bone, AbstractMesh* boneMesh, const Nullable<Vector3>& jointPivot,
  Nullable<float> distToJoint, const Nullable<Quaternion>& adjustRotation,
  Nullable<Vector3>& boneAxis)
{
  auto mesh = static_cast<AbstractMesh*>(object);

  if (mesh->rotationQuaternion()) {
    if (adjustRotation) {
      auto& tempQuat = PhysicsImpostor::_tmpQuat;
      bone->getRotationQuaternionToRef(tempQuat, Space::WORLD, boneMesh);
      tempQuat.multiplyToRef(*adjustRotation, *mesh->rotationQuaternion());
    }
    else {
      bone->getRotationQuaternionToRef(*mesh->rotationQuaternion(),
                                       Space::WORLD, boneMesh);
    }
  }

  auto& pos     = PhysicsImpostor::_tmpVecs[0];
  auto& boneDir = PhysicsImpostor::_tmpVecs[1];

  if (!boneAxis) {
    auto& _boneAxis = PhysicsImpostor::_tmpVecs[2];
    _boneAxis.x     = 0;
    _boneAxis.y     = 1;
    _boneAxis.z     = 0;
    boneAxis        = _boneAxis;
  }

  bone->getDirectionToRef(*boneAxis, boneDir, boneMesh);
  bone->getAbsolutePositionToRef(boneMesh, pos);

  if ((distToJoint.isNull()) && jointPivot) {
    distToJoint = (*jointPivot).length();
  }

  if (!distToJoint.isNull()) {
    float _distToJoint = *distToJoint;
    pos.x += boneDir.x * _distToJoint;
    pos.y += boneDir.y * _distToJoint;
    pos.z += boneDir.z * _distToJoint;
  }

  mesh->setAbsolutePosition(pos);
}

} // end of namespace BABYLON
