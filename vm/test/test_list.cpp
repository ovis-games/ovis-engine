#include "test_utils.hpp"
#include <iostream>

#include "catch2/catch.hpp"

#include "ovis/utils/log.hpp"
#include "ovis/vm/list.hpp"

using namespace ovis;

TEST_CASE("Construct list", "[ovis][vm][List]") {
  VirtualMachine vm;
  List number_list(vm.GetType<double>());

  REQUIRE(number_list.size() == 0);
  REQUIRE(number_list.capacity() == 0);

  number_list.Reserve(100);

  REQUIRE(number_list.size() == 0);
  REQUIRE(number_list.capacity() == 100);

  number_list.Resize(50);

  REQUIRE(number_list.size() == 50);
  REQUIRE(number_list.capacity() == 100);

  for (List::SizeType i = 0; i < 50; ++i) {
    REQUIRE(number_list.Get<double>(i) == 0);
    number_list.Set<double>(i, 100 + i);
    REQUIRE(number_list.Get<double>(i) == 100 + i);
  }

  number_list.Reserve(200);

  REQUIRE(number_list.size() == 50);
  REQUIRE(number_list.capacity() == 200);
  for (List::SizeType i = 0; i < 50; ++i) {
    REQUIRE(number_list.Get<double>(i) == 100 + i);
  }

  number_list.Resize(250);
}
