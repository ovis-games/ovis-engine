#include <catch2/catch.hpp>

#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_animation.hpp>
#include <ovis/core/transform.hpp>
#include <ovis/core/animator.hpp>

using namespace ovis;

// TEST_CASE("AnimationComponent", "[ovis][core][animations]") {
//   Scene test_scene;
//   test_scene.AddController("AnimatorController");

//   SceneObject* object = test_scene.CreateObject("TestObject", R"({
//     "template": "animation_test",
//     "components": {
//       "Core.Transform": {
//         "scale": [2, 1, 2]
//       },
//       "Core.Animator": {
//         "animation": "Some Movement",
//         "loop": true
//       }
//     }
//   })"_json);

//   REQUIRE(object != nullptr);
//   Transform* transform = object->GetComponent<Transform>();
//   REQUIRE(transform != nullptr);

//   Animator* animation = object->GetComponent<Animator>();
//   REQUIRE(animation != nullptr);

//   REQUIRE(transform->local_position().x == 0.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == 0.0f);

//   using namespace std::chrono_literals;
//   test_scene.Play();

//   test_scene.Update(0s);
//   REQUIRE(transform->local_position().x == -50.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == 5.0f);

//   test_scene.Update(1s);
//   REQUIRE(transform->local_position().x == 0.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == 0.0f);

//   test_scene.Update(1s);
//   REQUIRE(transform->local_position().x == 50.0f);
//   REQUIRE(transform->local_position().y == 0.0f); 
//   REQUIRE(transform->local_position().z == -5.0f);

//   test_scene.Update(1s);
//   REQUIRE(transform->local_position().x == 100.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == -10.0f);

//   test_scene.Update(1s);
//   REQUIRE(transform->local_position().x == 150.0f);
//   REQUIRE(transform->local_position().y == 0.0f);
//   REQUIRE(transform->local_position().z == -15.0f);

//   SECTION("Test Looping") {
//     test_scene.Update(2s);
//     REQUIRE(transform->local_position().x == 50.0f);
//     REQUIRE(transform->local_position().y == 0.0f); 
//     REQUIRE(transform->local_position().z == -5.0f);
//   }
// }
