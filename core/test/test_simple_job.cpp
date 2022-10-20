#include <string>

#include <catch2/catch.hpp>

#include "ovis/core/scene.hpp"
#include "ovis/core/simple_job.hpp"
#include "ovis/core/vm_bindings.hpp"

using namespace ovis;

struct Speed {
  float x = 1;
  float y = 2;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

OVIS_VM_DEFINE_TYPE_BINDING(Test, Speed) {
  Speed_type->AddAttribute("Core.EntityComponent");
  Speed_type->AddProperty<&Speed::x>("x");
  Speed_type->AddProperty<&Speed::y>("y");
}
  
struct Position {
  float x = 0;
  float y = 0;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

OVIS_VM_DEFINE_TYPE_BINDING(Test, Position) {
  Position_type->AddAttribute("Core.EntityComponent");
}

void Move(const Speed& speed, Position* position) {
  LogI("Hello from move");
  position->x += speed.x;
  position->y += speed.y;
}

OVIS_CREATE_SIMPLE_JOB(Move);

TEST_CASE("Test SimpleSceneController", "[ovis][core][SimpleSceneController]") {
  REQUIRE(main_vm->GetType<Position>()->attributes().contains("Core.EntityComponent"));
  REQUIRE(main_vm->GetType<Speed>()->attributes().contains("Core.EntityComponent"));

  {
    MoveJob move_job;

    REQUIRE(!move_job.read_access().contains(main_vm->GetTypeId<Position>()));
    REQUIRE(move_job.read_access().contains(main_vm->GetTypeId<Speed>()));
    REQUIRE(move_job.write_access().contains(main_vm->GetTypeId<Position>()));
    REQUIRE(!move_job.write_access().contains(main_vm->GetTypeId<Speed>()));
  }

  Scene s;
  s.frame_scheduler().AddJob<MoveJob>();

  s.Prepare();

  REQUIRE(s.frame_scheduler().GetUsedEntityComponents().contains(main_vm->GetTypeId<Speed>()));
  REQUIRE(s.frame_scheduler().GetUsedEntityComponents().contains(main_vm->GetTypeId<Position>()));

  auto speed_storage = s.GetComponentStorage<Speed>();
  auto position_storage = s.GetComponentStorage<Position>();
  REQUIRE(speed_storage);
  REQUIRE(position_storage);

  auto entity = s.CreateEntity("Obj");

  REQUIRE(speed_storage.AddComponent(entity->id));
  REQUIRE(position_storage.AddComponent(entity->id));

  {
    Speed speed = speed_storage[entity->id];
    REQUIRE(speed.x == 1);
    REQUIRE(speed.y == 2);

    Position pos = position_storage[entity->id];
    REQUIRE(pos.x == 0);
    REQUIRE(pos.y == 0);
  }

  s.Play();

  s.Update(0.1);
  {
    Speed speed = speed_storage[entity->id];
    REQUIRE(speed.x == 1);
    REQUIRE(speed.y == 2);

    Position pos = position_storage[entity->id];
    REQUIRE(pos.x == 1);
    REQUIRE(pos.y == 2);
  }

  s.Update(0.1);
  {
    Speed speed = speed_storage[entity->id];
    REQUIRE(speed.x == 1);
    REQUIRE(speed.y == 2);

    Position pos = position_storage[entity->id];
    REQUIRE(pos.x == 2);
    REQUIRE(pos.y == 4);
  }

  s.Stop();
}
