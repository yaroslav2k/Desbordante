#include "all_options.h"

#include <string>
#include <vector>

#include "algorithms/ar_algorithm_enums.h"
#include "algorithms/create_primitive.h"
#include "algorithms/metric/enums.h"
#include "algorithms/options/descriptions.h"
#include "algorithms/options/names.h"

namespace algos {
template <typename T>
void validate(boost::any& v, const std::vector<std::string>& values, T*, int) {
    namespace po = boost::program_options;

    const std::string& s = po::validators::get_single_string(values);
    try {
        v = boost::any(T::_from_string_nocase(s.c_str()));
    } catch (std::runtime_error& e) {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}
namespace metric {
using algos::validate;
}  // namespace metric
}  // namespace algos

namespace algos::config {
boost::program_options::options_description GeneralOptions() {
    namespace po = boost::program_options;
    using namespace config::names;
    using namespace config::descriptions;

    std::string const kSeparatorOpt = std::string(algos::config::names::kSeparator) + ",s";

    // clang-format off
    po::options_description general_options("General options");
    general_options.add_options()
            (kData, po::value<std::filesystem::path>()->required(), kDData)
            (kSeparatorOpt.c_str(), po::value<char>()->default_value(','), kDSeparator)
            (kHasHeader, po::value<bool>()->default_value(true), kDHasHeader)
            (kEqualNulls, po::value<bool>(), kDEqualNulls)
            (kThreads, po::value<ushort>(), kDThreads)
            ;
    // clang-format on
    return general_options;
}

boost::program_options::options_description AlgoOptions() {
    namespace po = boost::program_options;
    namespace desc = descriptions;

    // clang-format off
    po::options_description fd_options("FD options");
    fd_options.add_options()
            (names::kError, po::value<double>(), desc::kDError)
            (names::kMaximumLhs, po::value<unsigned int>(), desc::kDMaximumLhs)
            (names::kSeed, po::value<int>(), desc::kDSeed)
            ;

    po::options_description typo_options("Typo mining options");
    typo_options.add_options()
            (names::kRatio, po::value<double>(), desc::kDRatio)
            (names::kRadius, po::value<double>(), desc::kDRadius)
            (names::kApproximateAlgorithm, po::value<algos::PrimitiveType>(),
             desc::kDApproximateAlgorithm)
            (names::kPreciseAlgorithm, po::value<algos::PrimitiveType>(), desc::kDPreciseAlgorithm)
            ;

    po::options_description ar_options("AR options");
    ar_options.add_options()
            (names::kMinimumSupport, po::value<double>(), desc::kDMinimumSupport)
            (names::kMinimumConfidence, po::value<double>(), desc::kDMinimumConfidence)
            (names::kInputFormat, po::value<algos::InputFormat>(), desc::kDInputFormat)
            ;

    po::options_description ar_singular_options("AR \"singular\" input format options");
    ar_singular_options.add_options()
            (names::kTIdColumnIndex, po::value<unsigned>(), desc::kDTIdColumnIndex)
            (names::kItemColumnIndex, po::value<unsigned>(), desc::kDItemColumnIndex)
            ;

    po::options_description ar_tabular_options("AR \"tabular\" input format options");
    ar_tabular_options.add_options()
            (names::kFirstColumnTId, po::value<bool>()->default_value(true), desc::kDFirstColumnTId)
            ;

    ar_options.add(ar_singular_options).add(ar_tabular_options);

    po::options_description mfd_options("MFD options");
    mfd_options.add_options()
            (names::kMetric, po::value<algos::metric::Metric>(), desc::kDMetric)
            (names::kMetricAlgorithm, po::value<algos::metric::MetricAlgo>(),
             desc::kDMetricAlgorithm)
            (names::kLhsIndices, po::value<std::vector<unsigned int>>()->multitoken(),
             desc::kDLhsIndices)
            (names::kRhsIndices, po::value<std::vector<unsigned int>>()->multitoken(),
             desc::kDRhsIndices)
            (names::kParameter, po::value<long double>(), desc::kDParameter)
            (names::kDistFromNullIsInfinity, po::value<bool>()->default_value(false),
             desc::kDDistFromNullIsInfinity)
            ;

    po::options_description cosine_options("Cosine metric options");
    cosine_options.add_options()
            (names::kQGramLength, po::value<unsigned int>(), desc::kDQGramLength)
            ;

    mfd_options.add(cosine_options);

    po::options_description ac_options("AC options");
    ac_options.add_options()
            (names::kBinaryOperation, po::value<char>()->default_value('+'),
             "one of available operations: /, *, +, - ")
            (names::kFuzziness, po::value<double>()->default_value(0.15),
             "fraction of exceptional records")
            (names::kFuzzinessProbability, po::value<double>()->default_value(0.9),
             "probability, the fraction of exceptional records that lie outside the "
             "bump intervals is at most Fuzziness")
            (names::kWeight, po::value<double>()->default_value(0.05),
             "value between 0 and 1. Closer to 0 - many short intervals. "
             "Closer to 1 - small number of long intervals")
            (names::kBumpsLimit, po::value<size_t>()->default_value(5),
             "max considered intervals amount. Pass 0 to remove limit")
            (names::kIterationsLimit, po::value<size_t>()->default_value(10),
             "limit for iterations of sampling")
            (names::kPairingRule, po::value<std::string>()->default_value("trivial"),
             "one of available pairing rules: trivial")
            ;
    // clang-format on

    po::options_description algorithm_options("Algorithm options");
    algorithm_options.add(fd_options)
            .add(mfd_options)
            .add(ar_options)
            .add(ac_options)
            .add(typo_options);
    return algorithm_options;
}
}  // namespace algos::config
