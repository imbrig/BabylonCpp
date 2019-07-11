#include <babylon/samples/meshes/ray_helper_scene.h>

#include <babylon/cameras/free_camera.h>
#include <babylon/engines/scene.h>
#include <babylon/lights/hemispheric_light.h>
#include <babylon/materials/standard_material.h>
#include <babylon/meshes/builders/mesh_builder_options.h>
#include <babylon/meshes/mesh.h>
#include <babylon/meshes/mesh_builder.h>
#include <babylon/meshes/vertex_data_options.h>

namespace BABYLON {
namespace Samples {

RayHelperScene::RayHelperScene(ICanvas* iCanvas)
    : IRenderableScene(iCanvas)
    , _box{nullptr}
    , _boxTarget{nullptr}
    , _ground{nullptr}
    , _sphere{nullptr}
    , _rayHelper{_ray}
{
}

RayHelperScene::~RayHelperScene()
{
}

const char* RayHelperScene::getName()
{
  return "Ray Helper Scene";
}

void RayHelperScene::initializeScene(ICanvas* canvas, Scene* scene)
{
  auto camera = FreeCamera::New("camera1", Vector3(10.f, 8.f, -5.f), scene);
  camera->fov = 0.6f;
  camera->setTarget(Vector3::Zero());
  camera->attachControl(canvas, false);

  auto light = HemisphericLight::New("light1", Vector3(0.f, 1.f, 0.f), scene);
  light->intensity = 0.5f;

  _ground               = Mesh::CreateGround("ground1", 6, 6, 2, scene);
  _ground->position().y = -0.1f;
  auto mat              = StandardMaterial::New("mat1", scene);
  mat->alpha            = 0.2f;
  _ground->material     = mat;

  _box               = Mesh::CreateBox("box1", 0.5f, scene);
  _box->position().x = 0.2f;
  _box->position().y = 1.f;

  _boxTarget               = Mesh::CreateBox("box2", 1, scene);
  _boxTarget->position().x = 2.f;
  _boxTarget->position().z = 2.f;

  _box->lookAt(_boxTarget->position());

  scene->render();

  _rayHelper.attachToMesh(_box);
  _rayHelper.show(scene);

  _box->showBoundingBox       = true;
  _boxTarget->showBoundingBox = true;
  _ground->showBoundingBox    = true;

  SphereOptions options;
  options.diameter = 0.15f;
  _sphere          = MeshBuilder::CreateSphere("sphere", options, scene);
  _sphere->setEnabled(false);

  _meshes = {_boxTarget.get(), _ground.get()};

  _scene->registerBeforeRender([this](Scene* /*scene*/, EventState& /*es*/) {
    _box->rotation().x += 0.01f;
    _box->rotation().z += 0.01f;

    auto hitInfo = _ray.intersectsMeshes(_meshes);

    if (!hitInfo.empty()) {
      _sphere->setEnabled(true);
      if (hitInfo[0].pickedPoint.has_value()) {
        _sphere->position().copyFrom(*hitInfo[0].pickedPoint);
      }
    }
    else {
      _sphere->setEnabled(false);
    }
  });
}

} // end of namespace Samples
} // end of namespace BABYLON
