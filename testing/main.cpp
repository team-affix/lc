#include "test_utils.hpp"

extern void lambda_test_main();

void unit_test_main() {
  constexpr bool ENABLE_DEBUG_LOGS = true;

  TEST(lambda_test_main);
}

int main() {
  unit_test_main();

  return 0;
}
