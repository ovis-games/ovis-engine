#include <ovis/core/asset_library.hpp>

namespace ovis {

std::shared_ptr<json> LoadJsonSchema(AssetLibrary* asset_library, const std::string& name, bool use_cache = true);
std::shared_ptr<json> LoadJsonSchema(const std::string& name, bool use_cache = true);

}  // namespace ovis
