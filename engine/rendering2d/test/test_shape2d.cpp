#include <catch2/catch.hpp>

#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/rendering2d/shape2d.hpp>

using namespace ovis;

TEST_CASE("Add Shape2D to scene object", "[ovis][rendering2d][Shape2d]") {
  Scene test_scene;
  SceneObject* test_object = test_scene.CreateObject("Test");
  Shape2D* shape = test_object->AddComponent<Shape2D>();
  REQUIRE(shape != nullptr);
  REQUIRE(shape == test_object->GetComponent<Shape2D>());
}

TEST_CASE("Deserialize scene object with Shape2D", "[ovis][rendering2d][Shape2d]") {
  Scene test_scene;
  SceneObject* test_object = test_scene.CreateObject("TestObject", R"({
    "components": {
      "Rendering2D.Shape2D": {
        "color": [0.5, 1, 0.5, 1.0]
      }
    }
  })"_json);
  Shape2D* shape = test_object->GetComponent<Shape2D>();
  REQUIRE(shape != nullptr);
  REQUIRE(shape->color() == Color(0.5, 1.0, 0.5, 1.0));
}
