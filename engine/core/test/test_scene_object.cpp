#include <catch2/catch.hpp>

#include <ovis/core/scene.hpp>
#include <ovis/core/scene_object.hpp>
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
