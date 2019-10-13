#include "extensions/filters/http/investigator/investigator_filter.h"

#include "envoy/http/codes.h"
#include "envoy/stats/scope.h"

#include "common/common/logger.h"
#include "common/common/stack_array.h"
#include "common/protobuf/protobuf.h"
#include "common/protobuf/utility.h"

#include "common/common/assert.h"
#include "common/common/empty_string.h"
#include "common/common/enum_to_int.h"
#include "common/http/header_map_impl.h"
#include "common/http/headers.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Investigator {

InvestigatorFilterConfig::InvestigatorFilterConfig(const std::string& stats_prefix, Stats::Scope& scope)
    : stats_(generateStats(stats_prefix + "investigator.", scope)) {
      last_time_ = std::chrono::system_clock::now();
    }

InvestigatorFilter::InvestigatorFilter(InvestigatorFilterConfigSharedPtr config)
    : config_(std::move(config)) {
    }

Http::FilterHeadersStatus InvestigatorFilter::decodeHeaders(Http::HeaderMap&, bool) {
  std::cout << "here" << std::endl;
  return Http::FilterHeadersStatus::Continue;
}

Http::FilterDataStatus InvestigatorFilter::decodeData(Buffer::Instance& data, bool end_stream) {
  if (end_stream) {
    std::string body = buildBody(decoder_callbacks_->decodingBuffer(), data);
    if (!body.empty()) {
      try {
        Json::ObjectSharedPtr json_body = Json::Factory::loadFromString(body);
        std::string name = json_body->getString("name");
        std::string location = json_body->getString("location");
        std::cout<< "Received request from " << name << " : " << location  << std::endl;

        std::unordered_map<std::string, int>& locm = config_->loc_map();
        auto it = locm.find(location);
        if(it != locm.end()) 
            it->second++;
        else {
          locm[location] = 1;
        }

        auto curr_time = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = curr_time - config_->last_time();

        if ( elapsed_seconds.count() > 10 ) {
          std::cout << "Request from locations" << std::endl;
          for (auto x : locm) {
            std::cout << x.first << " " << x.second << std::endl; 
            locm[x.first] = 0;
          }
          config_->set_curr_time();
        }

        if (name.compare("David") == 0) {
          std::cout << "Blocked request from david" << std::endl;
          decoder_callbacks_->sendLocalReply(Http::Code::Unauthorized, "David is not allowed", nullptr,
                                       absl::nullopt, "more filler words");
          return Http::FilterDataStatus::StopIterationNoBuffer;
        }
      } catch (const Json::Exception& jsonEx) {
        // Body parsing failed. This should not happen, just put a stat for that.
        std::cout << "Not valid json" << std::endl;
      }
    }
    return Http::FilterDataStatus::Continue;
  }
  return Http::FilterDataStatus::StopIterationAndBuffer;
};

Http::FilterTrailersStatus InvestigatorFilter::decodeTrailers(Http::HeaderMap&) {
  return Http::FilterTrailersStatus::Continue;
}

void InvestigatorFilter::setDecoderFilterCallbacks(Http::StreamDecoderFilterCallbacks& callbacks) {
  decoder_callbacks_ = &callbacks;
}

std::string InvestigatorFilter::buildBody(const Buffer::Instance* buffered,
                                    const Buffer::Instance& last) {
  std::string body;
  if (buffered) {
    uint64_t num_slices = buffered->getRawSlices(nullptr, 0);
    STACK_ARRAY(slices, Buffer::RawSlice, num_slices);
    buffered->getRawSlices(slices.begin(), num_slices);
    for (const Buffer::RawSlice& slice : slices) {
      body.append(static_cast<const char*>(slice.mem_), slice.len_);
    }
  }

  uint64_t num_slices = last.getRawSlices(nullptr, 0);
  STACK_ARRAY(slices, Buffer::RawSlice, num_slices);
  last.getRawSlices(slices.begin(), num_slices);
  for (const Buffer::RawSlice& slice : slices) {
    body.append(static_cast<const char*>(slice.mem_), slice.len_);
  }

  return body;
}

} // namespace Investigator
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
