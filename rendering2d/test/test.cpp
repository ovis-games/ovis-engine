#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <ovis/utils/log.hpp>
#include <ovis/core/asset_library.hpp>
#include <ovis/core/main_vm.hpp>

int main(int argc, char* argv[]) {
  ovis::Log::AddListener(ovis::ConsoleLogger);
  ovis::InitializeMainVM();

  Catch::Session session;

  std::string assets_directory = ".";
  auto cli = session.cli() | Catch::clara::Opt(assets_directory, "assets directory")["--assets-directory"](
                                 "The directory containing the assets for the test.");
  session.cli(cli);

  // writing to session.configData() here sets defaults
  // this is the preferred way to set them
    
  int returnCode = session.applyCommandLine( argc, argv );
  if( returnCode != 0 ) {
    return returnCode;
  }

#if OVIS_EMSCRIPTEN
  ovis::SetEngineAssetsDirectory("/ovis_assets");
#else
  ovis::SetEngineAssetsDirectory(assets_directory);
#endif
 
  // writing to session.configData() or session.Config() here 
  // overrides command line args
  // only do this if you know you need to

  int numFailed = session.run();
  
  // numFailed is clamped to 255 as some unices only use the lower 8 bits.
  // This clamping has already been applied, so just return it here
  // You can also do any post run clean-up here
  return numFailed;
}
