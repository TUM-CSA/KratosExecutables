// System includes
#include <iostream>
#include <utility>
#include <algorithm>

// Kratos includes
#include "includes/kernel.h"
#include "containers/model.h"
#include "structural_mechanics_application.h"
#include "utilities/parallel_utilities.h"
#include "utilities/reduction_utilities.h"

using namespace Kratos;

using NodePair = std::pair<Node const *, Node const *>;

using NodePairMidPointIdMap = std::unordered_map<NodePair, Node::Pointer>;

NodePair GetMinMaxNodePair(Node const * pNode1, Node const * pNode2) {
    return std::minmax(pNode1, pNode2, [](const auto p1, const auto p2) { return p1->Id() < p2->Id(); });
}

template<class TContainerGetter>
void AddEntitiesRecursively(
    ModelPart& rOutputModelPart,
    ModelPart& rInputModelPart,
    TContainerGetter&& rContainerGetter)
{
    rContainerGetter(rOutputModelPart)->insert(*rContainerGetter(rInputModelPart));

    for (const auto& r_sub_model_part_name : rInputModelPart.GetSubModelPartNames()) {
        AddEntitiesRecursively(rOutputModelPart.GetSubModelPart(r_sub_model_part_name), rInputModelPart.GetSubModelPart(r_sub_model_part_name), rContainerGetter);
    }
}

void AddNewNodesRecursively(
    ModelPart& rOutputModelPart,
    const NodePairMidPointIdMap& rNodePairIdMap,
    const ModelPart& rInputModelPart)
{
    std::vector<Node::Pointer> nodes_to_be_added;
    for (const auto& r_pair : rNodePairIdMap) {
        if (rInputModelPart.HasNode(r_pair.first.first->Id()) && rInputModelPart.HasNode(r_pair.first.second->Id())) {
            nodes_to_be_added.push_back(r_pair.second);
        }
    }
    rOutputModelPart.Nodes().insert(nodes_to_be_added.begin(), nodes_to_be_added.end());

    for (const auto& r_sub_model_part_name : rInputModelPart.GetSubModelPartNames()) {
        auto& r_output_sub_model_part = rOutputModelPart.GetSubModelPart(r_sub_model_part_name);
        const auto& r_input_sub_model_part = rInputModelPart.GetSubModelPart(r_sub_model_part_name);
        AddNewNodesRecursively(r_output_sub_model_part, rNodePairIdMap, r_input_sub_model_part);
    }
}

void CreateSubModelParts(
    ModelPart& rOutputModelPart,
    const ModelPart& rInputModelPart)
{
    for (const auto& r_input_sub_model_part_name : rInputModelPart.GetSubModelPartNames()) {
        CreateSubModelParts(rOutputModelPart.CreateSubModelPart(r_input_sub_model_part_name), rInputModelPart.GetSubModelPart(r_input_sub_model_part_name));
    }
}

void CreateNodes(
    NodePairMidPointIdMap& rOutput,
    ModelPart& rOutputModelPart,
    ModelPart& rInputModelPart)
{
    auto max_node_id = block_for_each<MaxReduction<IndexType>>(rInputModelPart.Nodes(), [](const auto& rNode) {
        return rNode.Id();
    });

    auto p_pair_addition = [&rOutput, &rOutputModelPart, &max_node_id](const NodePair& rNodePair) {
        auto itr = rOutput.find(rNodePair);
        if (itr == rOutput.end()) {
            KRATOS_CRITICAL_SECTION;
            const array_1d<double, 3>& coordinates = (rNodePair.first->Coordinates() + rNodePair.second->Coordinates()) * 0.5;
            auto p_new_node = rOutputModelPart.CreateNewNode(++max_node_id, coordinates[0], coordinates[1], coordinates[2]);
            rOutput[rNodePair] = p_new_node;
        }
    };

    block_for_each(rInputModelPart.Elements(), [&p_pair_addition](const auto& rElement) {
        const auto& r_geometry = rElement.GetGeometry();
        KRATOS_ERROR_IF_NOT(r_geometry.size() == 3) << "This only supports refining meshes with triangular elements.";

        p_pair_addition(GetMinMaxNodePair(&r_geometry[0], &r_geometry[1]));
        p_pair_addition(GetMinMaxNodePair(&r_geometry[1], &r_geometry[2]));
        p_pair_addition(GetMinMaxNodePair(&r_geometry[2], &r_geometry[0]));
    });

    // add the existing nodes recursively
    AddEntitiesRecursively(rOutputModelPart, rInputModelPart, [](ModelPart& rModelPart) { return rModelPart.pNodes(); } );

    // now add the new nodes recursively
    AddNewNodesRecursively(rOutputModelPart, rOutput, rInputModelPart);
}

template<class TMap, class TContainerGetter>
void RecursivelyAddSubdividedEntities(
    ModelPart& rOutputModelPart,
    ModelPart& rInputModelPart,
    const TMap& rInputIdNewEntityMap,
    TContainerGetter&& rContainerGetter)
{
    auto& r_output_container = *rContainerGetter(rOutputModelPart);
    const auto& r_input_container = *rContainerGetter(rInputModelPart);

    std::vector<typename TMap::mapped_type::value_type> list_of_entities_to_be_added;
    for (const auto& r_input_entity : r_input_container) {
        auto itr = rInputIdNewEntityMap.find(r_input_entity.Id());
        KRATOS_ERROR_IF(itr == rInputIdNewEntityMap.end())
            << "The element id in the input mesh " << r_input_entity.Id()
            << " is not found in the map. Please make sure this id is present in the root model part.";

        list_of_entities_to_be_added.insert(list_of_entities_to_be_added.end(), itr->second.begin(), itr->second.end());
    }

    r_output_container.insert(list_of_entities_to_be_added.begin(), list_of_entities_to_be_added.end());

    for (const auto& r_sub_model_part_name : rInputModelPart.GetSubModelPartNames()) {
        RecursivelyAddSubdividedEntities(rOutputModelPart.GetSubModelPart(r_sub_model_part_name), rInputModelPart.GetSubModelPart(r_sub_model_part_name), rInputIdNewEntityMap, rContainerGetter);
    }
}

void CreateNewElements(
    ModelPart& rOutputModelPart,
    ModelPart& rInputModelPart,
    const NodePairMidPointIdMap& rNodePairIdMap)
{
    auto p_prop = rOutputModelPart.pGetProperties(1);

    IndexType new_element_id = 1;
    std::unordered_map<IndexType, std::vector<Element::Pointer>> map_of_new_elements;

    std::vector<Node::Pointer> nodes(6);
    PointerVector<Node> refined_element_nodes(3);
    for (const auto& r_element : rInputModelPart.Elements()) {
        const auto& r_geometry = r_element.GetGeometry();
        nodes[0] = r_geometry(0);
        nodes[1] = r_geometry(1);
        nodes[2] = r_geometry(2);
        nodes[3] = rNodePairIdMap.find(GetMinMaxNodePair(&*nodes[0], &*nodes[1]))->second;
        nodes[4] = rNodePairIdMap.find(GetMinMaxNodePair(&*nodes[1], &*nodes[2]))->second;
        nodes[5] = rNodePairIdMap.find(GetMinMaxNodePair(&*nodes[2], &*nodes[0]))->second;

        /**

            # for triangular meshes only
            #        0
            #       / \
            #      / 0 \
            #     3-----5
            #    / \ 3 / \
            #   / 1 \ / 2 \
            #  1-----4-----2

        **/

        refined_element_nodes(0) = nodes[0];
        refined_element_nodes(1) = nodes[3];
        refined_element_nodes(2) = nodes[5];
        map_of_new_elements[r_element.Id()].push_back(r_element.Create(new_element_id++, refined_element_nodes, p_prop));


        refined_element_nodes(0) = nodes[3];
        refined_element_nodes(1) = nodes[1];
        refined_element_nodes(2) = nodes[4];
        map_of_new_elements[r_element.Id()].push_back(r_element.Create(new_element_id++, refined_element_nodes, p_prop));

        refined_element_nodes(0) = nodes[5];
        refined_element_nodes(1) = nodes[4];
        refined_element_nodes(2) = nodes[2];
        map_of_new_elements[r_element.Id()].push_back(r_element.Create(new_element_id++, refined_element_nodes, p_prop));

        refined_element_nodes(0) = nodes[3];
        refined_element_nodes(1) = nodes[4];
        refined_element_nodes(2) = nodes[5];
        map_of_new_elements[r_element.Id()].push_back(r_element.Create(new_element_id++, refined_element_nodes, p_prop));
    }

    RecursivelyAddSubdividedEntities(rOutputModelPart, rInputModelPart, map_of_new_elements, [](ModelPart& rModelPart) { return rModelPart.pElements(); });
}

void CreateNewConditions(
    ModelPart& rOutputModelPart,
    ModelPart& rInputModelPart,
    const NodePairMidPointIdMap& rNodePairIdMap)
{
    auto p_prop = rOutputModelPart.pGetProperties(1);

    IndexType new_condition_id = 1;
    std::unordered_map<IndexType, std::vector<Condition::Pointer>> map_of_new_conditions;

    std::vector<Node::Pointer> nodes(3);
    PointerVector<Node> refined_condition_nodes(2);
    for (const auto& r_condition : rInputModelPart.Conditions()) {
        const auto& r_geometry = r_condition.GetGeometry();
        nodes[0] = r_geometry(0);
        nodes[1] = r_geometry(1);
        nodes[2] = rNodePairIdMap.find(GetMinMaxNodePair(&*nodes[0], &*nodes[1]))->second;


        /**

            # for triangular meshes only
            #    0     1
            # 0-----2-----1

        **/

        refined_condition_nodes(0) = nodes[0];
        refined_condition_nodes(1) = nodes[2];
        map_of_new_conditions[r_condition.Id()].push_back(r_condition.Create(new_condition_id++, refined_condition_nodes, p_prop));


        refined_condition_nodes(0) = nodes[2];
        refined_condition_nodes(1) = nodes[1];
        map_of_new_conditions[r_condition.Id()].push_back(r_condition.Create(new_condition_id++, refined_condition_nodes, p_prop));
    }

    RecursivelyAddSubdividedEntities(rOutputModelPart, rInputModelPart, map_of_new_conditions, [](ModelPart& rModelPart) { return rModelPart.pConditions(); });
}

int main(int argc, char *argv[])
{
    Kernel kernel;

    if (argc != 3) {
        std::cout << "Please provide input mesh name and output mesh name without the .mdpa extension." << std::endl;
        std::exit(-1);
    }

    std::cout << "Input mesh name : " << argv[1] << std::endl;
    std::cout << "Output mesh name: " << argv[2] << std::endl;

    auto p_structural_app = make_shared<KratosStructuralMechanicsApplication>();
    kernel.ImportApplication(p_structural_app);

    Model model;
    auto& r_input_model_part = model.CreateModelPart("input");
    auto& r_output_model_part = model.CreateModelPart("output");

    ModelPartIO(argv[1]).ReadModelPart(r_input_model_part);

    std::cout << "-------------- Input model part --------------" << std::endl << r_input_model_part << std::endl;

    CreateSubModelParts(r_output_model_part, r_input_model_part);

    NodePairMidPointIdMap node_pair_mid_point_id_map;
    CreateNodes(node_pair_mid_point_id_map, r_output_model_part, r_input_model_part);
    CreateNewElements(r_output_model_part, r_input_model_part, node_pair_mid_point_id_map);
    CreateNewConditions(r_output_model_part, r_input_model_part, node_pair_mid_point_id_map);

    std::cout << "-------------- Output model part --------------" << std::endl << r_output_model_part << std::endl;

    ModelPartIO(argv[2], ModelPartIO::WRITE | ModelPartIO::MESH_ONLY).WriteModelPart(r_output_model_part);
    return 0;
}