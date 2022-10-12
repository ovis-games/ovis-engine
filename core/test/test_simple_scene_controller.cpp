#include <string>
#include <catch2/catch.hpp>

#include "ovis/core/scene.hpp"
#include "ovis/core/simple_scene_controller.hpp"
#include "ovis/core/vm_bindings.hpp"

using namespace ovis;

struct Speed {
  float x;
  float y;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

OVIS_VM_DEFINE_TYPE_BINDING(Test, Speed) {
  Speed_type->attributes.insert("SceneObjectComponent");
}
  
struct Position {
  float x;
  float y;
  OVIS_VM_DECLARE_TYPE_BINDING();
};
OVIS_VM_DEFINE_TYPE_BINDING(Test, Position) {
  Position_type->attributes.insert("SceneObjectComponent");
}

void Move(const Speed& speed, Position* position) {
  position->x += speed.x;
  position->y += speed.y;
}

class MoveController : public SimpleSceneController<&Move> {
 public:
  MoveController() : SimpleSceneController("Move") {}
};

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
  s.Play();
  s.Stop();
}
