// System includes
#include <iostream>

// Kratos includes
#include "includes/kernel.h"
#include "containers/model.h"
#include "structural_mechanics_application.h"
#include "utilities/parallel_utilities.h"

using namespace Kratos;

int main(int argc, char *argv[])
{
    Kernel kernel;

    if (argc != 4) {
        std::cout << "Please provide input mesh name, output mesh name without the .mdpa extension and scaling factor" << std::endl;
        std::exit(-1);
    }

    const double scaling_factor = std::stod(argv[3]);

    std::cout << "Input mesh name : " << argv[1] << std::endl;
    std::cout << "Output mesh name: " << argv[2] << std::endl;
    std::cout << "Scaling factor  : " << scaling_factor << std::endl;

    auto p_structural_app = make_shared<KratosStructuralMechanicsApplication>();
    kernel.ImportApplication(p_structural_app);

    Model model;
    auto& r_input_model_part = model.CreateModelPart(argv[1]);
    ModelPartIO(argv[1]).ReadModelPart(r_input_model_part);

    std::cout << "-------------- Input model part --------------" << std::endl << r_input_model_part << std::endl;

    block_for_each(r_input_model_part.Nodes(), [scaling_factor](auto& rNode) { rNode.Coordinates() *= scaling_factor; });

    ModelPartIO(argv[2], ModelPartIO::WRITE).WriteModelPart(r_input_model_part);
    return 0;
}