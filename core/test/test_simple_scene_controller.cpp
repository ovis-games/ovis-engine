#include <string>
#include <catch2/catch.hpp>

#include "ovis/core/scene.hpp"
#include "ovis/core/simple_scene_controller.hpp"
#include "ovis/core/vm_bindings.hpp"

using namespace ovis;

struct Speed {
  float x = 1;
  float y = 2;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

OVIS_VM_DEFINE_TYPE_BINDING(Test, Speed) {
  Speed_type->attributes.insert("SceneObjectComponent");
  Speed_type->AddProperty<&Speed::x>("x");
  Speed_type->AddProperty<&Speed::y>("y");
}
  
struct Position {
  float x = 0;
  float y = 0;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

OVIS_VM_DEFINE_TYPE_BINDING(Test, Position) {
  Position_type->attributes.insert("SceneObjectComponent");
}

void Move(const Speed& speed, Position* position) {
  LogI("Hello from move");
  position->x += speed.x;
  position->y += speed.y;
}

OVIS_MAKE_SIMPLE_SCENE_CONTROLLER(Move);

TEST_CASE("Test SimpleSceneController", "[ovis][core][SimpleSceneController]") {
  REQUIRE(main_vm->GetType<Position>()->attributes().contains("SceneObjectComponent"));
  REQUIRE(main_vm->GetType<Speed>()->attributes().contains("SceneObjectComponent"));

  {
    MoveController move_controller;

    REQUIRE(!move_controller.read_access_components().contains(main_vm->GetTypeId<Position>()));
    REQUIRE(move_controller.read_access_components().contains(main_vm->GetTypeId<Speed>()));
    REQUIRE(move_controller.write_access_components().contains(main_vm->GetTypeId<Position>()));
    REQUIRE(!move_controller.write_access_components().contains(main_vm->GetTypeId<Speed>()));
  }

  Scene s;
  s.AddController<MoveController>();

  REQUIRE(s.GetUsedObjectComponentTypes().contains(main_vm->GetTypeId<Speed>()));
  REQUIRE(s.GetUsedObjectComponentTypes().contains(main_vm->GetTypeId<Position>()));

  s.Prepare();

  auto speed_storage = s.GetComponentStorage<Speed>();
  auto position_storage = s.GetComponentStorage<Position>();
  REQUIRE(speed_storage);
  REQUIRE(position_storage);

  auto object = s.CreateObject("Obj");

  REQUIRE(speed_storage->AddComponent(object->id()));
  REQUIRE(position_storage->AddComponent(object->id()));

  {
    Speed speed = speed_storage->GetComponent<Speed>(object->id());
    REQUIRE(speed.x == 1);
    REQUIRE(speed.y == 2);

    Position pos = position_storage->GetComponent<Position>(object->id());
    REQUIRE(pos.x == 0);
    REQUIRE(pos.y == 0);
  }

  s.Play();

  s.Update(std::chrono::microseconds(100));
  {
    Speed speed = speed_storage->GetComponent<Speed>(object->id());
    REQUIRE(speed.x == 1);
    REQUIRE(speed.y == 2);

    Position pos = position_storage->GetComponent<Position>(object->id());
    REQUIRE(pos.x == 1);
    REQUIRE(pos.y == 2);
  }

  s.Update(std::chrono::microseconds(100));
  {
    Speed speed = speed_storage->GetComponent<Speed>(object->id());
    REQUIRE(speed.x == 1);
    REQUIRE(speed.y == 2);

    Position pos = position_storage->GetComponent<Position>(object->id());
    REQUIRE(pos.x == 2);
    REQUIRE(pos.y == 4);
  }

  s.Stop();
}
