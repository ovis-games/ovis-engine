#include <catch2/catch.hpp>

#include <ovis/core/main_vm.hpp>

using namespace ovis;

TEST_CASE("Test game module loading", "[ovis][core][main_vm]") {
  REQUIRE(LoadScriptModule("CoreTest", GetEngineAssetLibrary()));
}
