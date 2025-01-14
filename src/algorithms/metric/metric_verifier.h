#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "algorithms/metric/aliases.h"
#include "algorithms/metric/enums.h"
#include "algorithms/metric/highlight_calculator.h"
#include "algorithms/metric/points.h"
#include "algorithms/metric/points_calculator.h"
#include "algorithms/options/equal_nulls_opt.h"
#include "algorithms/primitive.h"
#include "model/column_layout_relation_data.h"
#include "model/column_layout_typed_relation_data.h"
#include "util/convex_hull.h"
#include "util/qgram_vector.h"

namespace algos::metric {

class MetricVerifier : public algos::Primitive {
private:
    Metric metric_ = Metric::_values()[0];
    MetricAlgo algo_ = MetricAlgo::_values()[0];
    std::vector<unsigned int> lhs_indices_;
    std::vector<unsigned int> rhs_indices_;
    long double parameter_;
    unsigned int q_;
    bool dist_from_null_is_infinity_;
    config::EqNullsType is_null_equal_null_;

    bool metric_fd_holds_ = false;

    static const config::OptionType<decltype(dist_from_null_is_infinity_)>
            DistFromNullIsInfinityOpt;
    static const config::OptionType<decltype(parameter_)> ParameterOpt;
    static const config::OptionType<decltype(lhs_indices_)> LhsIndicesOpt;
    static const config::OptionType<decltype(rhs_indices_)> RhsIndicesOpt;
    static const config::OptionType<decltype(metric_)> MetricOpt;
    static const config::OptionType<decltype(algo_)> AlgoOpt;
    static const config::OptionType<decltype(q_)> QGramLengthOpt;

    std::shared_ptr<model::ColumnLayoutTypedRelationData> typed_relation_;
    std::shared_ptr<ColumnLayoutRelationData> relation_;  // temporarily parsing twice
    std::unique_ptr<PointsCalculator> points_calculator_;
    std::unique_ptr<HighlightCalculator> highlight_calculator_;

    DistanceFunction<std::byte const*> GetCosineDistFunction(
            model::StringType const& type,
            std::unordered_map<std::string, util::QGramVector>& q_gram_map) const;

    bool CheckMFDFailIfHasNulls(bool has_nulls) const {
        return dist_from_null_is_infinity_ && has_nulls;
    }
    bool CompareNumericValues(std::vector<IndexedOneDimensionalPoint> const& points) const;

    template <typename T>
    bool ApproxVerifyCluster(std::vector<T> const& points,
                             DistanceFunction<T> const& dist_func) const;

    template <typename T>
    bool BruteVerifyCluster(std::vector<IndexedPoint<T>> const& points,
                            DistanceFunction<T> const& dist_func) const;

    bool CalipersCompareNumericValues(std::vector<util::Point>& points) const;

    template <typename T>
    ClusterFunction CalculateClusterFunction(IndexedPointsFunction<T> points_func,
                                             CompareFunction<T> compare_func,
                                             HighlightFunction<T> highlight_func) const;
    template <typename T>
    ClusterFunction CalculateApproxClusterFunction(PointsFunction<T> points_func,
                                                   DistanceFunction<T> dist_func) const;
    ClusterFunction GetClusterFunctionForSeveralDimensions();
    ClusterFunction GetClusterFunctionForOneDimension();
    ClusterFunction GetClusterFunction();
    void VerifyMetricFD();
    std::string GetStringValue(std::vector<unsigned> const& index_vec,
                               ClusterIndex row_index) const;
    void VisualizeHighlights() const;
    static_assert(std::is_same<decltype(MetricVerifier::lhs_indices_),
                               decltype(MetricVerifier::rhs_indices_)>{},
                  "Types of indices must be the same");
    void ValidateIndices(decltype(MetricVerifier::lhs_indices_) const& indices) const;
    void ValidateRhs(decltype(MetricVerifier::rhs_indices_) const& indices);
    void RegisterOptions();

    void ResetState() final;

protected:
    void FitInternal(model::IDatasetStream& data_stream) override;
    void MakeExecuteOptsAvailable() override;
    unsigned long long ExecuteInternal() override;

public:
    bool GetResult() const {
        return metric_fd_holds_;
    }

    std::vector<std::vector<Highlight>> const& GetHighlights() const {
        return highlight_calculator_->GetHighlights();
    }

    void SetParameter(long double parameter) {
        parameter_ = parameter;
    }

    void SortHighlightsByDistanceAscending() {
        highlight_calculator_->SortHighlightsByDistanceAscending();
    }
    void SortHighlightsByDistanceDescending() {
        highlight_calculator_->SortHighlightsByDistanceDescending();
    }
    void SortHighlightsByFurthestIndexAscending() {
        highlight_calculator_->SortHighlightsByFurthestIndexAscending();
    }
    void SortHighlightsByFurthestIndexDescending() {
        highlight_calculator_->SortHighlightsByFurthestIndexDescending();
    }
    void SortHighlightsByIndexAscending() {
        highlight_calculator_->SortHighlightsByIndexAscending();
    }
    void SortHighlightsByIndexDescending() {
        highlight_calculator_->SortHighlightsByIndexDescending();
    }

    MetricVerifier();
};

}  // namespace algos::metric
