#include "test.h"

TEST(simple);
TEST(teapot);
TEST(teddy);

int
main (void) {
  RUN(simple);
  RUN(teapot);
  RUN(teddy);
  return 0;
}

