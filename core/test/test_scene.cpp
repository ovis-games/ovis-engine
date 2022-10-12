#include <string>
#include <catch2/catch.hpp>

#include "ovis/core/scene.hpp"

using namespace ovis;

TEST_CASE("Create scene", "[ovis][core][Scene]") {
  Scene scene;

  scene.CreateObject("Test");
}

TEST_CASE("Create scene objects", "[Scene]") {
  // BENCHMARK_ADVANCED("Create objects")(Catch::Benchmark::Chronometer meter) {
  //   Scene scene;
  //   meter.measure([&]() {
  //     for (int i = 0; i < 1000; ++i) {
  //       scene.CreateObject(std::to_string(i));
  //     }
  //   });
  // };
}

// TEST_CASE("Create Scene Object", "[ovis][core][SceneObject]") {
//   Scene test_scene;
//   SceneObject* object = test_scene.CreateObject("TestObject", R"({
//     "components": {
//       "Core.Transform": {
//         "scale": [2, 1, 2]
//       }
//     }
//   })"_json);

//   REQUIRE(object != nullptr);
//   Transform* transform = object->GetComponent<Transform>();
//   REQUIRE(transform != nullptr);
//   REQUIRE(transform->local_position() == Vector3::Zero());
//   REQUIRE(transform->local_scale() == Vector3(2.0, 1.0, 2.0));
// }

// TEST_CASE("Add component via template", "[ovis][core][SceneObject]") {
//   Scene test_scene;
//   SceneObject* object = test_scene.CreateObject("TestObject");

//   REQUIRE(object != nullptr);
//   Transform* transform = object->GetComponent<Transform>();
//   REQUIRE(transform == nullptr);

//   transform = object->AddComponent<Transform>();
//   REQUIRE(transform != nullptr);

//   REQUIRE(transform == object->GetComponent<Transform>());
// }

// TEST_CASE("Create Scene Object with Template", "[ovis][core][SceneObject]") {
//   Scene test_scene;
//   SceneObject* object = test_scene.CreateObject("TestObject", R"({
//     "template": "template",
//     "components": {
//       "Core.Transform": {
//         "scale": [2, 1, 2]
//       }
//     }
//   })"_json);

//   REQUIRE(object != nullptr);
//   Transform* transform = object->GetComponent<Transform>();
//   REQUIRE(transform != nullptr);
//   REQUIRE(transform->local_position() == Vector3(1.0, 2.0, 3.0));
//   REQUIRE(transform->local_scale() == Vector3(2.0, 1.0, 2.0));
// }

// TEST_CASE("Try to create scene object with recursive template", "[ovis][core][SceneObject]") {
//   Scene test_scene;

//   SECTION("Recursion") {
//     SceneObject* object = test_scene.CreateObject("TestObject", R"({
//       "template": "recursive_template",
//       "components": {}
//     })"_json);
//     REQUIRE(object == nullptr);
//   }

//   SECTION("Indirect recursion") {
//     SceneObject* object = test_scene.CreateObject("TestObject", R"({
//       "template": "indirect_recursion1",
//       "components": {}
//     })"_json);
//     REQUIRE(object == nullptr);
//   }

//   SECTION("Child recursion") {
//     SceneObject* object = test_scene.CreateObject("TestObject", R"({
//       "template": "child_recursion_template",
//       "components": {}
//     })"_json);
//     REQUIRE(object == nullptr);
//   }
// }

// TEST_CASE("Create Scene Object with Animation", "[ovis][core][SceneObject]") {
//   Scene test_scene;
//   SceneObject* object = test_scene.CreateObject("TestObject", R"({
//     "template": "animation_test",
//     "components": {
//       "Core.Transform": {
//         "scale": [2, 1, 2]
//       }
//     }
//   })"_json);

//   REQUIRE(object != nullptr);
//   Transform* transform = object->GetComponent<Transform>();
//   REQUIRE(transform != nullptr);
//   REQUIRE(transform->local_scale() == Vector3(2.0, 1.0, 2.0));
//   REQUIRE(SceneObject::GetAnimation("animation_test", "Some Movement") != nullptr);
//   const std::vector<const SceneObjectAnimation*> animations(object->animations().begin(), object->animations().end());
//   REQUIRE(animations.size() == 1);
//   REQUIRE(animations[0]->name() == "Some Movement");

//   SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(50, object);
//   REQUIRE(transform->local_position().x == 0.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == 0.0f);

//   SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(150, object);
//   REQUIRE(transform->local_position().x == 100.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == -10.0f);

//   SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(100, object);
//   REQUIRE(transform->local_position().x == 50.0f);
//   REQUIRE(transform->local_position().y == 0.0f); 
//   REQUIRE(transform->local_position().z == -5.0f);

//   // Extrapolation:
//   SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(0, object);
//   REQUIRE(transform->local_position().x == -50.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == 5.0f);

//   SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(200, object);
//   REQUIRE(transform->local_position().x == 150.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == -15.0f);
// }
