#include <babylon/materials/node/blocks/smooth_step_block.h>

#include <babylon/core/string.h>
#include <babylon/materials/node/node_material_build_state.h>
#include <babylon/materials/node/node_material_connection_point.h>

namespace BABYLON {

SmoothStepBlock::SmoothStepBlock(const std::string& iName)
    : NodeMaterialBlock{iName, NodeMaterialBlockTargets::Neutral}
    , value{this, &SmoothStepBlock::get_value}
    , edge0{this, &SmoothStepBlock::get_edge0}
    , edge1{this, &SmoothStepBlock::get_edge1}
    , output{this, &SmoothStepBlock::get_output}
{
  registerInput("value", NodeMaterialBlockConnectionPointTypes::Float);
  registerInput("edge0", NodeMaterialBlockConnectionPointTypes::Float);
  registerInput("edge1", NodeMaterialBlockConnectionPointTypes::Float);
  registerOutput("output", NodeMaterialBlockConnectionPointTypes::Float);
}

SmoothStepBlock::~SmoothStepBlock() = default;

std::string SmoothStepBlock::getClassName() const
{
  return "SmoothStepBlock";
}

NodeMaterialConnectionPointPtr& SmoothStepBlock::get_value()
{
  return _inputs[0];
}

NodeMaterialConnectionPointPtr& SmoothStepBlock::get_edge0()
{
  return _inputs[1];
}

NodeMaterialConnectionPointPtr& SmoothStepBlock::get_edge1()
{
  return _inputs[2];
}

NodeMaterialConnectionPointPtr& SmoothStepBlock::get_output()
{
  return _outputs[0];
}

SmoothStepBlock& SmoothStepBlock::_buildBlock(NodeMaterialBuildState& state)
{
  NodeMaterialBlock::_buildBlock(state);

  const auto& iOutput = _outputs[0];

  state.compilationString
    += _declareOutput(iOutput, state)
       + String::printf(" = smoothstep(%s, %s, %s);\r\n", edge0()->associatedVariableName().c_str(),
                        edge1()->associatedVariableName().c_str(),
                        value()->associatedVariableName().c_str());

  return *this;
}

} // end of namespace BABYLON