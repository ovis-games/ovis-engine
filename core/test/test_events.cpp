#include <string>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"

#include "ovis/core/event_storage.hpp"
#include "ovis/core/scene.hpp"
#include "ovis/core/simple_job.hpp"
#include "ovis/core/vm_bindings.hpp"
#include "ovis/test/require_result.hpp"

using namespace ovis;

struct NumberEvent {
  double number;

  OVIS_VM_DECLARE_TYPE_BINDING();
};

OVIS_VM_DEFINE_TYPE_BINDING(Test, NumberEvent) {
  NumberEvent_type->AddAttribute("Core.Event");
}

void NumberEventEmitter(EventEmitter<NumberEvent> number_event_emitter) {
  for (int i = 0; i < 10; ++i) {
    number_event_emitter.Emit({
      .number = 100,
    });
  }
}
OVIS_CREATE_SIMPLE_JOB(NumberEventEmitter)

class NumberEventListener : public FrameJob {
  public:
    NumberEventListener() : FrameJob("NumberEventListener") {
      RequireReadAccess(main_vm->GetTypeId<NumberEvent>());
      ExecuteAfter("NumberEventEmitter");
    }

    Result<> Prepare(Scene* const& update) override { return Success; }
    Result<> Execute(const SceneUpdate& update) override {
      for (auto event : update.scene->GetEventStorage<NumberEvent>()) {
        sum_ += event->number;
      }

      return Success;
    }

    double sum() { return sum_; }

   private:
    double sum_ = 0.0;
};

TEST_CASE("Emit and receive events", "[ovis][core][Events]") {
  Scene scene;
  // Insert the jobs in the "wrong" order on purpose
  scene.frame_scheduler().AddJob<NumberEventListener>();
  scene.frame_scheduler().AddJob<NumberEventEmitterJob>();
  REQUIRE_RESULT(scene.Prepare());
  scene.Play();

  for (int i = 0; i < 10; ++i) {
    scene.Update(1.0);
  }
  REQUIRE(static_cast<NumberEventListener*>(scene.frame_scheduler().GetJob("NumberEventListener"))->sum() ==
          10 * 10 * 100);

  BENCHMARK_ADVANCED("Emit events")(Catch::Benchmark::Chronometer meter) {
    meter.measure([&]() {
      for (int i = 0; i < 1000; ++i) {
        scene.Update(1.0);
      }
    });
  };
}
