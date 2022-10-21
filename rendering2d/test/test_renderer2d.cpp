#include <string>

#include "catch2/catch.hpp"
#include <thread>

#include "ovis/core/scene.hpp"
#include "ovis/rendering/clear_pass.hpp"
#include "ovis/rendering2d/renderer2d.hpp"
#include "ovis/rendering2d/text.hpp"
#include "ovis/test/utils.hpp"

using namespace ovis;

TEST_CASE("Display shapes", "[ovis][rendering2d][Renderer2D]") {
  ovis::test::TestWindow window;

  Text s = R"(
  {
    "text": "Hello"
  }
  )"_json;

  Scene scene;
  scene.frame_scheduler().AddJob<ClearPass>(&window.graphics_context, Color::Aqua());
  scene.frame_scheduler().AddJob<Renderer2D>(&window.graphics_context);
  scene.Prepare();

  auto circle = scene.CreateEntity("Circle");
  scene.GetComponentStorage<Shape2D>().AddComponent(circle->id);
  auto& shape = scene.GetComponentStorage<Shape2D>()[circle->id];
  shape.SetEllipse({
    .size = {1.0, 1.0},
    .num_segments = 32,
  });
  shape.SetOutlineWidth(0.1);
  shape.SetColor(Color::Yellow());

  scene.Play();
  scene.Update(0.0);
}
