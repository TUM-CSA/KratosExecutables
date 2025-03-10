#include <iostream>
#include <utility>
#include <memory>
#include <ostream>

// --- External Includes ---
#include "benchmark/benchmark.h"
#include "nanoflann/nanoflann.hpp"

// --- Kratos Includes ---
#include "includes/kernel.h"
#include "containers/array_1d.h"
#include "spatial_containers/spatial_containers.h"
#include "utilities/parallel_utilities.h"

struct Entity
{
    using Pointer = std::shared_ptr<Entity>;
    std::size_t mId;
    Kratos::array_1d<double, 3> mPosition;

    double operator[](const std::size_t Index) const { return mPosition[Index]; }

    double& operator[](const std::size_t Index) { return mPosition[Index]; }

    Kratos::array_1d<double, 3> Coordinates() const { return mPosition; }

    Kratos::array_1d<double, 3>& Coordinates() { return mPosition; }

    auto begin() { return mPosition.begin(); }

    auto end() { return mPosition.end(); }
};

/// output stream function
inline std::ostream& operator<<(
    std::ostream& rOStream,
    const Entity& rThis)
{
    return rOStream;
}

// --- Entity vector ---
using EntityPointVectorType = std::vector<Entity::Pointer>;

void CreatePoints(
    EntityPointVectorType& rOutput,
    const std::size_t N)
{
    /// bounding box size
    /// H = 1.0, W = 1.0, B = 1.0

    const double gap = 1.0 / (N - 1);
    rOutput.resize(N * N * N);

    Kratos::IndexPartition<std::size_t>(rOutput.size()).for_each([&rOutput, gap, N](const std::size_t Index) {
        const std::size_t i_x = Index / (N * N);
        const std::size_t i_y = Index / N;
        const std::size_t i_z = Index % N;

        auto new_entity = std::make_shared<Entity>();

        new_entity->mId = Index + 1;
        new_entity->mPosition[0] = gap * i_x;
        new_entity->mPosition[1] = gap * i_y;
        new_entity->mPosition[2] = gap * i_z;

        rOutput[Index] = new_entity;
    });
}

// =======================================================================
// ============================== NanoFlann ==============================
// =======================================================================

// --- NanoFlann KD Tree types
struct NanoFlannEntityAdapter
{
    NanoFlannEntityAdapter(const EntityPointVectorType& rData): mrData(rData) {};

    inline std::size_t kdtree_get_point_count() const { return mrData.size(); }

    inline double kdtree_get_pt(const std::size_t idx, int dim) const { return mrData[idx]->mPosition[dim]; }

    template <class BBOX> bool kdtree_get_bbox(BBOX &bb) const { return false; }

    const EntityPointVectorType& mrData;
};

using NanoFlannResultType = nanoflann::ResultItem<unsigned int, double>;
using NanoFlannResultVectorType = std::vector<NanoFlannResultType>;
using NanoFlannDistanceMetricType = typename nanoflann::metric_L2_Simple::traits<double, NanoFlannEntityAdapter>::distance_t;
using NanoFlannKDTreeIndexType = nanoflann::KDTreeSingleIndexAdaptor<NanoFlannDistanceMetricType, NanoFlannEntityAdapter, 3>;

void NanoFlannKDTreeBuild(benchmark::State& rState)
{
    const std::size_t leaf_size = 10;
    for (auto _ : rState) {
        const std::size_t input_size = rState.range(0);

        EntityPointVectorType points;

        rState.PauseTiming();
        CreatePoints(points, input_size);
        rState.ResumeTiming();

        NanoFlannEntityAdapter adapter(points);

        NanoFlannKDTreeIndexType index(3, adapter, nanoflann::KDTreeSingleIndexAdaptorParams(leaf_size, nanoflann::KDTreeSingleIndexAdaptorFlags::None, 0));
        index.buildIndex();

        rState.PauseTiming();
        NanoFlannResultVectorType result;
        index.radiusSearch(points.front()->mPosition.data().begin(), 0.5 * 0.5, result, nanoflann::SearchParameters());
        rState.ResumeTiming();
    }
}

void NanoFlannKDTreeSearch(benchmark::State& rState)
{
    const std::size_t leaf_size = 10;
    for (auto _ : rState) {
        const std::size_t input_size = rState.range(0);

        EntityPointVectorType points;

        rState.PauseTiming();
        CreatePoints(points, input_size);
        NanoFlannEntityAdapter adapter(points);
        NanoFlannKDTreeIndexType index(3, adapter, nanoflann::KDTreeSingleIndexAdaptorParams(leaf_size, nanoflann::KDTreeSingleIndexAdaptorFlags::None, 0));
        index.buildIndex();
        rState.ResumeTiming();

        // now search for everything
        Kratos::block_for_each(points, NanoFlannResultVectorType(), [&index](const auto& pPoint, auto& rTLS) {
            index.radiusSearch(pPoint->mPosition.data().begin(), 0.5 * 0.5, rTLS, nanoflann::SearchParameters());
        });

    }
}

// ======================================================================
// ============================ KratosKdTree ============================
// ======================================================================

// --- Kratos KD Tree types
using KratosBucketType = Kratos::Bucket<3, Entity, EntityPointVectorType>;
using KratosKDTreeType = Kratos::Tree<Kratos::KDTreePartition<KratosBucketType>>;

struct KratosKDtreeResults
{
    explicit KratosKDtreeResults(const std::size_t MaxNumberOfNeighbours)
    {
        mNeighbours.resize(MaxNumberOfNeighbours);
        mDistances.resize(MaxNumberOfNeighbours);
    }

    EntityPointVectorType mNeighbours;
    std::vector<double> mDistances;
};

void KratosKDTreeBuild(benchmark::State& rState)
{
    const std::size_t leaf_size = 10;
    for (auto _ : rState) {
        const std::size_t input_size = rState.range(0);

        EntityPointVectorType points;

        rState.PauseTiming();
        CreatePoints(points, input_size);
        rState.ResumeTiming();

        KratosKDTreeType index(points.begin(), points.end(), leaf_size);

        rState.PauseTiming();
        EntityPointVectorType neighbours(10000);
        std::vector<double> distances(10000);
        index.SearchInRadius(*points.front(), 0.5, neighbours.begin(), distances.begin(), 10000);
        rState.ResumeTiming();
    }
}

void KratosKDTreeSearch(benchmark::State& rState)
{
    const std::size_t leaf_size = 10;
    for (auto _ : rState) {
        const std::size_t input_size = rState.range(0);

        EntityPointVectorType points;

        rState.PauseTiming();
        CreatePoints(points, input_size);
        KratosKDTreeType index(points.begin(), points.end(), leaf_size);
        rState.ResumeTiming();

        // now search for everything
        Kratos::block_for_each(points, KratosKDtreeResults(10000), [&index](const auto& pPoint, auto& rTLS) {
            index.SearchInRadius(*pPoint, 0.5, rTLS.mNeighbours.begin(), rTLS.mDistances.begin(), 10000);
        });

    }
}


BENCHMARK(NanoFlannKDTreeBuild) -> RangeMultiplier(4) -> Range(1, 256);
BENCHMARK(NanoFlannKDTreeSearch) -> RangeMultiplier(4) -> Range(1, 256);

BENCHMARK(KratosKDTreeBuild) -> RangeMultiplier(4) -> Range(1, 256);
BENCHMARK(KratosKDTreeSearch) -> RangeMultiplier(4) -> Range(1, 256);

BENCHMARK_MAIN();