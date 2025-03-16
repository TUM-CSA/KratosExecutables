/// @author Máté Kelemen

// --- Core Includes ---
#include "includes/model_part_io.h"

// --- Internal Includes ---
#include "KratosExecutables/ModelPartIO.hpp"

// --- Optional MED Includes ---
#ifdef KRATOSEXECUTABLES_MED_APPLICATION
#include "custom_io/med_model_part_io.h"
#endif

// --- Optional HDF5 Includes ---
#ifdef KRATOSEXECUTABLES_HDF5_APPLICATION
#include "custom_io/hdf5_model_part_io.h"
#include "custom_io/hdf5_file.h"
#endif

// --- STL Includes ---
#include <filesystem> // std::filesystem::path


namespace Kratos::Executables {


struct MDPAModelPartIO::Impl {
    std::filesystem::path mFilePath;
}; // struct MDPAModelPartIO::Impl


MDPAModelPartIO::MDPAModelPartIO()
    : mpImpl(new Impl)
{
}


MDPAModelPartIO::MDPAModelPartIO(std::filesystem::path&& rFilePath)
    : mpImpl(new Impl {std::move(rFilePath)})
{
}


MDPAModelPartIO::~MDPAModelPartIO()
{
}


void MDPAModelPartIO::Read(ModelPart& rTarget) const
{
    Kratos::ModelPartIO(mpImpl->mFilePath, IO::READ).ReadModelPart(rTarget);
}


void MDPAModelPartIO::Write(const ModelPart& rSource)
{
    ModelPart& omfg = const_cast<ModelPart&>(rSource);
    Kratos::ModelPartIO(mpImpl->mFilePath, IO::WRITE | IO::SCIENTIFIC_PRECISION).WriteModelPart(omfg);
}


struct MedModelPartIO::Impl {
    std::filesystem::path mFilePath;
}; // struct MedModelPartIO::Impl


MedModelPartIO::MedModelPartIO()
    : mpImpl(new Impl)
{
}


MedModelPartIO::MedModelPartIO(std::filesystem::path&& rFilePath)
    : mpImpl(new Impl {std::move(rFilePath)})
{
}


MedModelPartIO::~MedModelPartIO()
{
}


void MedModelPartIO::Read(ModelPart& rTarget) const
{
    #ifdef KRATOSEXECUTABLES_MED_APPLICATION
    Kratos::MedModelPartIO(mpImpl->mFilePath, IO::READ).ReadModelPart(rTarget);
    #else
    KRATOS_ERROR << "KratosExecutables was built without MED support."
    #endif
}


void MedModelPartIO::Write(const ModelPart& rSource)
{
    #ifdef KRATOSEXECUTABLES_MED_APPLICATION
    ModelPart& omfg = const_cast<ModelPart&>(rSource);
    Kratos::ModelPartIO(mpImpl->mFilePath, IO::WRITE).WriteModelPart(omfg);
    #else
    KRATOS_ERROR << "KratosExecutables was built without MED support."
    #endif
}


struct HDF5ModelPartIO::Impl {
    std::filesystem::path mFilePath;
}; // HDF5ModelPartIO::Impl


HDF5ModelPartIO::HDF5ModelPartIO()
    : mpImpl(new Impl)
{
}


HDF5ModelPartIO::~HDF5ModelPartIO()
{
}


HDF5ModelPartIO::HDF5ModelPartIO(std::filesystem::path&& rFilePath)
    : mpImpl(new Impl {std::move(rFilePath)})
{
}


void HDF5ModelPartIO::Read(ModelPart& rTarget) const
{
    #ifdef KRATOSEXECUTABLES_HDF5_APPLICATION
    Kratos::Parameters file_parameters(R"({
        "file_name" : "",
        "file_access_mode" : "read_only"
    })");
    file_parameters["file_name"].SetString(mpImpl->mFilePath.string());
    Kratos::HDF5::File::Pointer p_file(new Kratos::HDF5::File(
        rTarget.GetCommunicator().GetDataCommunicator(),
        file_parameters));
    Kratos::HDF5::ModelPartIO(p_file, "/ModelData").ReadModelPart(rTarget);
    #else
    KRATOS_ERROR << "KratosExecutables was built without HDF5 support."
    #endif
}


void HDF5ModelPartIO::Write(const ModelPart& rSource)
{
    #ifdef KRATOSEXECUTABLES_HDF5_APPLICATION
    Kratos::Parameters file_parameters(R"({
        "file_name" : "",
        "file_access_mode" : "exclusive"
    })");
    file_parameters["file_name"].SetString(mpImpl->mFilePath.string());
    Kratos::HDF5::File::Pointer p_file(new Kratos::HDF5::File(
        rSource.GetCommunicator().GetDataCommunicator(),
        file_parameters));
    ModelPart& omfg = const_cast<ModelPart&>(rSource);
    Kratos::HDF5::ModelPartIO(p_file, "/ModelData").WriteModelPart(omfg);
    #else
    KRATOS_ERROR << "KratosExecutables was built without HDF5 support."
    #endif
}


std::unique_ptr<ModelPartIO> IOFactory(const std::filesystem::path& rFilePath)
{
    std::string suffix = rFilePath.extension().string();
    std::transform(suffix.begin(),
                   suffix.end(),
                   suffix.begin(),
                   [](auto item){return std::tolower(item);});

    using IOPtr = std::unique_ptr<ModelPartIO>;
    if (suffix == ".mdpa") {
        std::filesystem::path path = rFilePath;
        path.replace_extension("");
        return IOPtr(new MDPAModelPartIO(std::move(path)));
    } else if (suffix == ".med") {
        #ifdef KRATOSEXECUTABLES_MED_APPLICATION
        return IOPtr(new MedModelPartIO(std::filesystem::path(rFilePath)));
        #else
        KRATOS_ERROR << "KratosExecutables was built without MED support."
        #endif
    } else if (suffix == ".h5" || suffix == ".hdf5") {
        #ifdef KRATOSEXECUTABLES_HDF5_APPLICATION
        return IOPtr(new HDF5ModelPartIO(std::filesystem::path(rFilePath)));
        #else
        KRATOS_ERROR << "KratosExecutables was built without HDF5 support."
        #endif
    }
    KRATOS_ERROR << "Unsupported file format: " << rFilePath.extension();
}


} // namespace Kratos::Executables
