#include "algorithms/fd_algorithm.h"

#include <thread>

#include "algorithms/options/equal_nulls_opt.h"

namespace algos {

FDAlgorithm::FDAlgorithm(std::vector<std::string_view> phase_names)
        : Primitive(std::move(phase_names)) {
    RegisterOptions();
    MakeOptionsAvailable(config::GetOptionNames(config::EqualNullsOpt));
}

void FDAlgorithm::RegisterOptions() {
    RegisterOption(config::EqualNullsOpt.GetOption(&is_null_equal_null_));
}

void FDAlgorithm::FitInternal(model::IDatasetStream& data_stream) {
    number_of_columns_ = data_stream.GetNumberOfColumns();
    FitFd(data_stream);
}

void FDAlgorithm::ResetState() {
    fd_collection_.clear();
    ResetStateFd();
}

std::string FDAlgorithm::GetJsonFDs() const {
    return FDsToJson(fd_collection_);
}

unsigned int FDAlgorithm::Fletcher16() {
    std::string to_hash = GetJsonFDs();
    unsigned int sum1 = 0, sum2 = 0, modulus = 255;
    for (auto ch : to_hash) {
        sum1 = (sum1 + ch) % modulus;
        sum2 = (sum2 + sum1) % modulus;
    }
    return (sum2 << 8) | sum1;
}

/* Attribute A contains only unique values (i.e. A is the key) iff [A]->[B]
 * holds for every attribute B. So to determine if A is a key, we count
 * number of fds with lhs==[A] and if it equals the total number of attributes
 * minus one (the attribute A itself) then A is the key.
 */
std::vector<Column const*> FDAlgorithm::GetKeys() const {
    std::vector<Column const*> keys;
    std::map<Column const*, size_t> fds_count_per_col;
    unsigned int cols_of_equal_values = 0;

    for (FD const& fd : fd_collection_) {
        Vertical const& lhs = fd.GetLhs();

        if (lhs.GetArity() == 0) {
            /* We separately count columns consisting of only equal values,
             * because they cannot be on the right side of the minimal fd.
             * And obviously for every attribute A true: [A]->[B] holds
             * if []->[B] holds.
             */
            cols_of_equal_values++;
        } else if (lhs.GetArity() == 1) {
            fds_count_per_col[lhs.GetColumns().front()]++;
        }
    }

    for (auto const& [col, num]: fds_count_per_col) {
        if (num + 1 + cols_of_equal_values == number_of_columns_) {
            keys.push_back(col);
        }
    }

    return keys;
}

}  // namespace algos
