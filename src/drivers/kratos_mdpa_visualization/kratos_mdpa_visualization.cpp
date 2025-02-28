// System includes
#include <iostream>
#include <sstream>
#include <filesystem>

// Kratos includes
#include "includes/kernel.h"
#include "containers/model.h"
#include "structural_mechanics_application.h"
#include "input_output/vtu_output.h"
#include "utilities/parallel_utilities.h"
#include "includes/variables.h"
#include "expression/container_expression.h"
#include "expression/variable_expression_io.h"

using namespace Kratos;

void RecursiveOutput(ModelPart& rOutputModelPart)
{
    VtuOutput vtu_output(rOutputModelPart);

    ContainerExpression<ModelPart::NodesContainerType> node_ids(rOutputModelPart);
    VariableExpressionIO::Read(node_ids, &STEP, false);
    vtu_output.AddContainerExpression<ModelPart::NodesContainerType>("node_ids", node_ids.Clone());

    if (rOutputModelPart.NumberOfElements() > 0) {
        ContainerExpression<ModelPart::ElementsContainerType> element_ids(rOutputModelPart);
        VariableExpressionIO::Read(element_ids, &STEP);
        vtu_output.AddContainerExpression<ModelPart::ElementsContainerType>("element_ids", element_ids.Clone());
    } else if (rOutputModelPart.NumberOfConditions() > 0) {
        ContainerExpression<ModelPart::ConditionsContainerType> condition_ids(rOutputModelPart);
        VariableExpressionIO::Read(condition_ids, &STEP);
        vtu_output.AddContainerExpression<ModelPart::ConditionsContainerType>("condition_ids", condition_ids.Clone());
    }

    std::stringstream file_name;
    file_name << rOutputModelPart.GetRootModelPart().FullName() << "/" << rOutputModelPart.FullName();
    vtu_output.PrintOutput(file_name.str());

    for (const auto& r_sub_model_part_name : rOutputModelPart.GetSubModelPartNames()) {
        RecursiveOutput(rOutputModelPart.GetSubModelPart(r_sub_model_part_name));
    }
}

int main(int argc, char *argv[])
{
    Kernel kernel;

    if (argc != 2) {
        std::cout << "Please provide input mesh name without the .mdpa extension." << std::endl;
        std::exit(-1);
    }

    std::cout << "Input mesh name : " << argv[1] << std::endl;

    auto p_structural_app = make_shared<KratosStructuralMechanicsApplication>();
    kernel.ImportApplication(p_structural_app);

    Model model;
    auto& r_input_model_part = model.CreateModelPart(argv[1]);
    ModelPartIO(argv[1]).ReadModelPart(r_input_model_part);

    std::cout << "-------------- Input model part --------------" << std::endl << r_input_model_part << std::endl;

    block_for_each(r_input_model_part.Nodes(), [](auto& rNode) { rNode.SetValue(STEP, rNode.Id()); });
    block_for_each(r_input_model_part.Conditions(), [](auto& rCondition) { rCondition.SetValue(STEP, rCondition.Id()); });
    block_for_each(r_input_model_part.Elements(), [](auto& rElement) { rElement.SetValue(STEP, rElement.Id()); });

    if (!std::filesystem::is_directory(r_input_model_part.FullName())) {
        std::filesystem::create_directory(r_input_model_part.FullName());
    }

    RecursiveOutput(r_input_model_part);
    return 0;
}