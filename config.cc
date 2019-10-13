#include "extensions/filters/http/investigator/config.h"

#include "envoy/registry/registry.h"

#include "extensions/filters/http/investigator/investigator_filter.h"

namespace Envoy {
namespace Extensions {
namespace HttpFilters {
namespace Investigator {

Http::FilterFactoryCb
InvestigatorFilterFactory::createFilter(const std::string& stats_prefix,
                                Server::Configuration::FactoryContext& context) {
  InvestigatorFilterConfigSharedPtr config =
      std::make_shared<InvestigatorFilterConfig>(stats_prefix, context.scope());
  return [config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    callbacks.addStreamDecoderFilter(std::make_shared<InvestigatorFilter>(config));
  };
}

/**
 * Static registration for the cors filter. @see RegisterFactory.
 */
REGISTER_FACTORY(InvestigatorFilterFactory, Server::Configuration::NamedHttpFilterConfigFactory);

} // namespace Cors
} // namespace HttpFilters
} // namespace Extensions
} // namespace Envoy
