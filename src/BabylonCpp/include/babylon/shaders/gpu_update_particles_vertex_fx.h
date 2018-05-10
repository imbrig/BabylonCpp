﻿#ifndef BABYLON_SHADERS_GPU_UPDATE_PARTICLES_VERTEX_FX_H
#define BABYLON_SHADERS_GPU_UPDATE_PARTICLES_VERTEX_FX_H

namespace BABYLON {

extern const char* gpuUpdateParticlesVertexShader;

const char* gpuUpdateParticlesVertexShader
  = "#version 300 es\n"
    "\n"
    "#define PI 3.14159\n"
    "\n"
    "uniform float currentCount;\n"
    "uniform float timeDelta;\n"
    "uniform float stopFactor;\n"
    "uniform vec3 generalRandoms;\n"
    "uniform mat4 emitterWM;\n"
    "uniform vec2 lifeTime;\n"
    "uniform vec2 emitPower;\n"
    "uniform vec2 sizeRange;\n"
    "uniform vec4 color1;\n"
    "uniform vec4 color2;\n"
    "uniform vec3 gravity;\n"
    "uniform sampler2D randomSampler;\n"
    "\n"
    "#ifdef BOXEMITTER\n"
    "uniform vec3 direction1;\n"
    "uniform vec3 direction2;\n"
    "uniform vec3 minEmitBox;\n"
    "uniform vec3 maxEmitBox;\n"
    "#endif\n"
    "\n"
    "#ifdef SPHEREEMITTER\n"
    "uniform float radius;\n"
    "  #ifdef DIRECTEDSPHEREEMITTER\n"
    "  uniform vec3 direction1;\n"
    "  uniform vec3 direction2;\n"
    "  #else\n"
    "  uniform float directionRandomizer;\n"
    "  #endif\n"
    "#endif\n"
    "\n"
    "#ifdef CONEEMITTER\n"
    "uniform float radius;\n"
    "uniform float angle;\n"
    "uniform float height;\n"
    "uniform float directionRandomizer;\n"
    "#endif\n"
    "\n"
    "// Particles state\n"
    "in vec3 position;\n"
    "in float age;\n"
    "in float life;\n"
    "in float seed;\n"
    "in float size;\n"
    "in vec4 color;\n"
    "in vec3 direction;\n"
    "\n"
    "// Output\n"
    "out vec3 outPosition;\n"
    "out float outAge;\n"
    "out float outLife;\n"
    "out float outSeed;\n"
    "out float outSize;\n"
    "out vec4 outColor;\n"
    "out vec3 outDirection;\n"
    "\n"
    "vec3 getRandomVec3(float offset) {\n"
    "  return texture(randomSampler, vec2(float(gl_VertexID) * offset / currentCount, 0)).rgb;\n"
    "}\n"
    "\n"
    "vec4 getRandomVec4(float offset) {\n"
    "  return texture(randomSampler, vec2(float(gl_VertexID) * offset / currentCount, 0));\n"
    "}\n"
    "\n"
    "\n"
    "void main() {\n"
    "  if (age >= life) {\n"
    "  if (stopFactor == 0.) {\n"
    "  outPosition = position;\n"
    "  outAge = life;\n"
    "  outLife = life;\n"
    "  outSeed = seed;\n"
    "  outColor = vec4(0.,0.,0.,0.);\n"
    "  outSize = 0.;\n"
    "  outDirection = direction;\n"
    "  return;\n"
    "  }\n"
    "  vec3 position;\n"
    "  vec3 direction;\n"
    "\n"
    "  // Let's get some random values\n"
    "  vec4 randoms = getRandomVec4(generalRandoms.x);\n"
    "\n"
    "  // Age and life\n"
    "  outAge = 0.0;\n"
    "  outLife = lifeTime.x + (lifeTime.y - lifeTime.x) * randoms.r;\n"
    "\n"
    "  // Seed\n"
    "  outSeed = seed;\n"
    "\n"
    "  // Size\n"
    "  outSize = sizeRange.x + (sizeRange.y - sizeRange.x) * randoms.g;\n"
    "\n"
    "  // Color\n"
    "  outColor = color1 + (color2 - color1) * randoms.b;\n"
    "\n"
    "  // Position / Direction (based on emitter type)\n"
    "#ifdef BOXEMITTER\n"
    "  vec3 randoms2 = getRandomVec3(generalRandoms.y);\n"
    "  vec3 randoms3 = getRandomVec3(generalRandoms.z);\n"
    "\n"
    "  position = minEmitBox + (maxEmitBox - minEmitBox) * randoms2;\n"
    "\n"
    "  direction = direction1 + (direction2 - direction1) * randoms3;\n"
    "#elif defined(SPHEREEMITTER)\n"
    "  vec3 randoms2 = getRandomVec3(generalRandoms.y);\n"
    "  vec3 randoms3 = getRandomVec3(generalRandoms.z);\n"
    "\n"
    "  // Position on the sphere surface\n"
    "  float phi = 2.0 * PI * randoms2.x;\n"
    "  float theta = PI * randoms2.y;\n"
    "  float randX = cos(phi) * sin(theta);\n"
    "  float randY = cos(theta);\n"
    "  float randZ = sin(phi) * sin(theta);\n"
    "\n"
    "  position = (radius * randoms2.z) * vec3(randX, randY, randZ);\n"
    "\n"
    "  #ifdef DIRECTEDSPHEREEMITTER\n"
    "  direction = direction1 + (direction2 - direction1) * randoms3;\n"
    "  #else\n"
    "  // Direction\n"
    "  direction = position + directionRandomizer * randoms3;\n"
    "  #endif\n"
    "#elif defined(CONEEMITTER)\n"
    "  vec3 randoms2 = getRandomVec3(generalRandoms.y);\n"
    "\n"
    "  float s = 2.0 * PI * randoms2.x;\n"
    "  float h = randoms2.y;\n"
    "  \n"
    "  // Better distribution in a cone at normal angles.\n"
    "  h = 1. - h * h;\n"
    "  float lRadius = radius * randoms2.z;\n"
    "  lRadius = lRadius * h;\n"
    "\n"
    "  float randX = lRadius * sin(s);\n"
    "  float randZ = lRadius * cos(s);\n"
    "  float randY = h  * height;\n"
    "\n"
    "  position = vec3(randX, randY, randZ); \n"
    "\n"
    "  // Direction\n"
    "  if (angle == 0.) {\n"
    "  direction = vec3(0., 1.0, 0.);\n"
    "  } else {\n"
    "  vec3 randoms3 = getRandomVec3(generalRandoms.z);\n"
    "  direction = position + directionRandomizer * randoms3;\n"
    "  }\n"
    "#else    \n"
    "  // Create the particle at origin\n"
    "  position = vec3(0., 0., 0.);\n"
    "\n"
    "  // Spread in all directions\n"
    "  direction = 2.0 * (getRandomVec3(seed) - vec3(0.5, 0.5, 0.5));\n"
    "#endif\n"
    "\n"
    "  float power = emitPower.x + (emitPower.y - emitPower.x) * randoms.a;\n"
    "\n"
    "  outPosition = (emitterWM * vec4(position, 1.)).xyz;\n"
    "  outDirection = (emitterWM * vec4(direction * power, 0.)).xyz;\n"
    "\n"
    "  } else {   \n"
    "  outPosition = position + direction * timeDelta;\n"
    "  outAge = age + timeDelta;\n"
    "  outLife = life;\n"
    "  outSeed = seed;\n"
    "  outColor = color;\n"
    "  outSize = size;\n"
    "  outDirection = direction + gravity * timeDelta;\n"
    "  }\n"
    "}\n";

} // end of namespace BABYLON

#endif // end of BABYLON_SHADERS_GPU_UPDATE_PARTICLES_VERTEX_FX_H
