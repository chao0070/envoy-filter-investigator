#pragma once

#include "envoy/server/filter_config.h"

#include "extensions/filters/http/common/empty_http_filter_config.h"
#include "extensions/filters/http/well_known_names.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Investigator {

/**
 * Config registration for the investigator filter. @see NamedHttpFilterConfigFactory.
 */
class InvestigatorFilterFactory : public Common::EmptyHttpFilterConfig {
public:
  InvestigatorFilterFactory() : Common::EmptyHttpFilterConfig("envoy.investigator") {}

  Http::FilterFactoryCb createFilter(const std::string& stats_prefix,
                                     Server::Configuration::FactoryContext& context) override;
};

} // namespace Investigator
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
