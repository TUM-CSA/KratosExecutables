/// @author Máté Kelemen

#pragma once

// --- Core Includes ---
#include "includes/model_part.h" // ModelPart

// --- STL Includes ---
#include <filesystem> // std::filesystem::path
#include <memory> // std::unique_ptr


namespace Kratos::Executables {


class ModelPartIO
{
public:
    virtual void Read(ModelPart& rTarget) const = 0;

    virtual void Write(const ModelPart& rSource) = 0;

    virtual ~ModelPartIO() = default;
}; // class ModelPartIO


class MDPAModelPartIO final : public ModelPartIO
{
public:
    MDPAModelPartIO();

    explicit MDPAModelPartIO(std::filesystem::path&& rFilePath);

    ~MDPAModelPartIO() override;

    void Read(ModelPart& rTarget) const override;

    void Write(const ModelPart& rSource) override;

private:
    struct Impl;
    std::unique_ptr<Impl> mpImpl;
}; // class MDPAModelPartIO


class MedModelPartIO final : public ModelPartIO
{
public:
    MedModelPartIO();

    explicit MedModelPartIO(std::filesystem::path&& rFilePath);

    ~MedModelPartIO() override;

    void Read(ModelPart& rTarget) const override;

    void Write(const ModelPart& rSource) override;

private:
    struct Impl;
    std::unique_ptr<Impl> mpImpl;
}; // class MedModelPartIO


class HDF5ModelPartIO final : public ModelPartIO
{
public:
    HDF5ModelPartIO();

    HDF5ModelPartIO(std::filesystem::path&& rFilePath);

    ~HDF5ModelPartIO() override;

    void Read(ModelPart& rTarget) const override;

    void Write(const ModelPart& rSource) override;

private:
    struct Impl;
    std::unique_ptr<Impl> mpImpl;
} ; // class HDF5ModelPartIO


std::unique_ptr<ModelPartIO> IOFactory(const std::filesystem::path& rFilePath);


} // namespace Kratos::Executables
