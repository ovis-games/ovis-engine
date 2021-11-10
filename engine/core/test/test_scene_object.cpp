#include <catch2/catch.hpp>

#include <iomanip>

#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
#include <ovis/core/scene_object_animation.hpp>
#include <ovis/core/transform.hpp>

using namespace ovis;

TEST_CASE("Parse object name", "[ovis][core][SceneObject]") {
  SECTION("object name without number") {
    auto result = SceneObject::ParseName("Test name");
    REQUIRE(result.first == "Test name");
    REQUIRE(!result.second.has_value());
  }

  SECTION("object name with number at the end") {
    auto result = SceneObject::ParseName("Test name123");
    REQUIRE(result.first == "Test name");
    REQUIRE(result.second.has_value());
    REQUIRE(result.second == 123);
  }

  SECTION("object name with number in the midle") {
    auto result = SceneObject::ParseName("Test 123 name");
    REQUIRE(result.first == "Test 123 name");
    REQUIRE(!result.second.has_value());
  }

  SECTION("object name with number in the middle and at the end") {
    auto result = SceneObject::ParseName("Test 234 name123");
    REQUIRE(result.first == "Test 234 name");
    REQUIRE(result.second.has_value());
    REQUIRE(result.second == 123);
  }
}

TEST_CASE("Create Scene Object", "[ovis][core][SceneObject]") {
  Scene test_scene;
  SceneObject* object = test_scene.CreateObject("TestObject", R"({
    "components": {
      "Core.Transform": {
        "scale": [2, 1, 2]
      }
    }
  })"_json);

  REQUIRE(object != nullptr);
  Transform* transform = object->GetComponent<Transform>();
  REQUIRE(transform != nullptr);
  REQUIRE(transform->local_position() == Vector3::Zero());
  REQUIRE(transform->local_scale() == Vector3(2.0, 1.0, 2.0));
}

TEST_CASE("Add component via template", "[ovis][core][SceneObject]") {
  Scene test_scene;
  SceneObject* object = test_scene.CreateObject("TestObject");

  REQUIRE(object != nullptr);
  Transform* transform = object->GetComponent<Transform>();
  REQUIRE(transform == nullptr);

  transform = object->AddComponent<Transform>();
  REQUIRE(transform != nullptr);

  REQUIRE(transform == object->GetComponent<Transform>());
}

TEST_CASE("Create Scene Object with Template", "[ovis][core][SceneObject]") {
  Scene test_scene;
  SceneObject* object = test_scene.CreateObject("TestObject", R"({
    "template": "template",
    "components": {
      "Core.Transform": {
        "scale": [2, 1, 2]
      }
    }
  })"_json);

  REQUIRE(object != nullptr);
  Transform* transform = object->GetComponent<Transform>();
  REQUIRE(transform != nullptr);
  REQUIRE(transform->local_position() == Vector3(1.0, 2.0, 3.0));
  REQUIRE(transform->local_scale() == Vector3(2.0, 1.0, 2.0));
}

std::string FullPrecision(double d) {
    auto s = std::ostringstream{};
    s << std::setprecision( std::numeric_limits<double>::max_digits10 ) << d;
    return s.str();
}

TEST_CASE("Create Scene Object with Animation", "[ovis][core][SceneObject]") {
  Scene test_scene;
  SceneObject* object = test_scene.CreateObject("TestObject", R"({
    "template": "animation_test",
    "components": {
      "Transform": {
        "scale": [2, 1, 2]
      }
    }
  })"_json);

  REQUIRE(object != nullptr);
  Transform* transform = object->GetComponent<Transform>("Transform");
  REQUIRE(transform != nullptr);
  REQUIRE(transform->local_scale() == Vector3(2.0, 1.0, 2.0));
  REQUIRE(SceneObject::GetAnimation("animation_test", "Some Movement") != nullptr);

  SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(50, object);
  REQUIRE(transform->local_position().x == 0.0f);
  REQUIRE(transform->local_position().y == 0.0f);
  REQUIRE(transform->local_position().z == 0.0f);

  SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(150, object);
  REQUIRE(transform->local_position().x == 100.0f);
  REQUIRE(transform->local_position().y == 0.0f);
  REQUIRE(transform->local_position().z == -10.0f);

  SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(100, object);
  REQUIRE(transform->local_position().x == 50.0f);
  REQUIRE(transform->local_position().y == 0.0f); 
  REQUIRE(transform->local_position().z == -5.0f);

  // Extrapolation:
  SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(0, object);
  REQUIRE(transform->local_position().x == -50.0f);
  REQUIRE(transform->local_position().y == 0.0f);
  REQUIRE(transform->local_position().z == 5.0f);

  SceneObject::GetAnimation("animation_test", "Some Movement")->Animate(200, object);
  REQUIRE(transform->local_position().x == 150.0f);
  REQUIRE(transform->local_position().y == 0.0f);
  REQUIRE(transform->local_position().z == -15.0f);
}
