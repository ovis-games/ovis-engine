#include "catch2/catch_test_macros.hpp"

#include "ovis/vm/type.hpp"
#include "ovis/vm/value.hpp"
#include "ovis/vm/value_storage.hpp"
#include "ovis/vm/virtual_machine.hpp"

using namespace ovis;

bool MemoryIsAlignedTo(void* memory, std::size_t alignment) {
  auto memory_address = reinterpret_cast<std::uintptr_t>(memory);
  return memory_address % alignment == 0;
}

TEST_CASE("Use value storage", "[ovis][vm][ValueStorage]") {
  VirtualMachine vm;
  ValueStorage value_storage;

  SECTION("Can allocate storage") {
    void* memory = value_storage.Allocate(128, 128);
    REQUIRE(memory != nullptr);
    REQUIRE(value_storage.has_allocated_storage());
    REQUIRE(MemoryIsAlignedTo(memory, 128));

    value_storage.Deallocate();
    REQUIRE(!value_storage.has_allocated_storage());
  }

  SECTION("AllocateIfNecessary works as expected") {
    SECTION("alignment=4, size=4") {
      void* memory = value_storage.AllocateIfNecessary(4, 4);
      REQUIRE(memory != nullptr);
      REQUIRE(MemoryIsAlignedTo(memory, 4));
      REQUIRE(!value_storage.has_allocated_storage());
    }

    SECTION("alignment=16, size=4") {
      void* memory = value_storage.AllocateIfNecessary(16, 16);
      REQUIRE(memory != nullptr);
      REQUIRE(MemoryIsAlignedTo(memory, 16));
      REQUIRE(value_storage.has_allocated_storage());
      value_storage.Deallocate();
    }

    SECTION("alignment=4, size=16") {
      void* memory = value_storage.AllocateIfNecessary(4, 16);
      REQUIRE(memory != nullptr);
      REQUIRE(MemoryIsAlignedTo(memory, 4));
      REQUIRE(value_storage.has_allocated_storage());
      value_storage.Deallocate();
    }
  }

  SECTION("Store() stores value properly") {
    SECTION("Small type") {
      value_storage.Store(42.0);
      REQUIRE(!value_storage.has_allocated_storage());
      REQUIRE(!value_storage.destruct_function());
      REQUIRE(value_storage.as<double>() == 42.0);
    }

    SECTION("Big type") {
      struct BigType {
        double d1;
        double d2;
      };
      value_storage.Store(BigType { .d1 = 42.0, .d2 = 128.0 });
      REQUIRE(value_storage.has_allocated_storage());
      REQUIRE(!value_storage.destruct_function());
      REQUIRE(value_storage.as<BigType>().d1 == 42.0);
      REQUIRE(value_storage.as<BigType>().d2 == 128.0);
      value_storage.Deallocate();
    }
  }

  SECTION("Destruct function is called on Reset()") {
    struct DestructorTestType {
      ~DestructorTestType() {
        *pointer = 0;
      }

      int* pointer;
    };
    int zero_after_destruction;
    value_storage.Store(DestructorTestType { .pointer = &zero_after_destruction });
    REQUIRE(!value_storage.has_allocated_storage());
    REQUIRE(value_storage.destruct_function());

    zero_after_destruction = 42;
    value_storage.Reset(vm.main_execution_context());
    REQUIRE(zero_after_destruction == 0);
    REQUIRE(!value_storage.has_allocated_storage());
    REQUIRE(!value_storage.destruct_function());
  }

  SECTION("Construct from layout") {
    REQUIRE(!ValueStorage::IsTypeStoredInline(vm.GetType<std::string>()->description().memory_layout.alignment_in_bytes,
                                              vm.GetType<std::string>()->description().memory_layout.size_in_bytes));
    value_storage.Construct(vm.main_execution_context(), vm.GetType<std::string>()->description().memory_layout);
    REQUIRE(!ValueStorage::stored_inline<std::string>);
    REQUIRE(value_storage.has_allocated_storage());
    REQUIRE(value_storage.destruct_function());

    std::string& string = value_storage.as<std::string>();
    REQUIRE(string.length() == 0);
    string = "1234567890";
    REQUIRE(string.length() == 10);

    value_storage.Reset(vm.main_execution_context());
    REQUIRE(!value_storage.has_allocated_storage());
    REQUIRE(!value_storage.destruct_function());
  }

  SECTION("Copy") {
    SECTION("Copy trivially") {
      value_storage.Construct(vm.main_execution_context(), vm.GetType<double>()->description().memory_layout);
      REQUIRE(value_storage.as<double>() == 0.0);

      value_storage.as<double>() = 42.0;
      REQUIRE(value_storage.as<double>() == 42.0);

      ValueStorage second_storage;
      ValueStorage::CopyTrivially(&second_storage, &value_storage);

      REQUIRE(value_storage.as<double>() == 42.0);
      REQUIRE(second_storage.as<double>() == 42.0);
    }

    SECTION("Copy non-trivial value") {
      value_storage.Construct(vm.main_execution_context(), vm.GetType<std::string>()->description().memory_layout);
      REQUIRE(value_storage.as<std::string>() == "");

      value_storage.as<std::string>() = "Hello World";
      REQUIRE(value_storage.as<std::string>() == "Hello World");

      ValueStorage second_storage;
      second_storage.Construct(vm.main_execution_context(), vm.GetType<std::string>()->description().memory_layout);
      ValueStorage::Copy(vm.main_execution_context(), vm.GetType<std::string>()->description().memory_layout, &second_storage, &value_storage);

      REQUIRE(value_storage.as<std::string>() == "Hello World");
      REQUIRE(second_storage.as<std::string>() == "Hello World");
      REQUIRE(second_storage.has_allocated_storage());
      REQUIRE(second_storage.destruct_function());

      second_storage.Reset(vm.main_execution_context());
      value_storage.Reset(vm.main_execution_context());
    }
  }
}
