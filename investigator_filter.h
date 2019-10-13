#pragma once

#include <chrono>
#include "envoy/http/filter.h"

#include "common/common/logger.h"

#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"

#include "common/buffer/buffer_impl.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Investigator {

/**
 * All CORS filter stats. @see stats_macros.h
 */
// clang-format off
#define ALL_INVESTIGATOR_STATS(COUNTER)\
  COUNTER(origin_valid)        \
  COUNTER(origin_invalid)      \
// clang-format on

/**
 * Struct definition for CORS stats. @see stats_macros.h
 */
struct InvestigatorStats {
  ALL_INVESTIGATOR_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Configuration for the CORS filter.
 */
class InvestigatorFilterConfig {
public:
  InvestigatorFilterConfig(const std::string& stats_prefix, Stats::Scope& scope);
  InvestigatorStats& stats() { return stats_; }
  std::unordered_map<std::string, int>& loc_map() { return loc_map_;}
  std::chrono::system_clock::time_point last_time() { return last_time_; }
  void set_curr_time() { last_time_ = std::chrono::system_clock::now(); /*std::cout << "timeupdated" << std::endl;*/}

private:
  static InvestigatorStats generateStats(const std::string& prefix, Stats::Scope& scope) {
    return InvestigatorStats{ALL_INVESTIGATOR_STATS(POOL_COUNTER_PREFIX(scope, prefix))};
  }

  InvestigatorStats stats_;
  std::unordered_map<std::string, int> loc_map_;
  std::chrono::system_clock::time_point last_time_;

};
using InvestigatorFilterConfigSharedPtr = std::shared_ptr<InvestigatorFilterConfig>;

class InvestigatorFilter : public Http::StreamDecoderFilter {
public:
  InvestigatorFilter(InvestigatorFilterConfigSharedPtr config);

  // Http::StreamDecoderFilterBase
  void onDestroy() override {}

  // Http::StreamDecoderFilter
  Http::FilterHeadersStatus decodeHeaders(Http::HeaderMap& headers, bool end_stream) override;
  Http::FilterDataStatus decodeData(Buffer::Instance&, bool) override;
  Http::FilterTrailersStatus decodeTrailers(Http::HeaderMap&) override;

  void setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) override;

private:
  std::string buildBody(const Buffer::Instance* buffered,
                                    const Buffer::Instance& last);

  Http::StreamDecoderFilterCallbacks* decoder_callbacks_{};
  bool is_investigator_request_{};
  const Http::HeaderEntry* origin_{};

  InvestigatorFilterConfigSharedPtr config_;
};

} // namespace Investigator
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
