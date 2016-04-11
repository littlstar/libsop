#include "test.h"

TEST(material);
TEST(simple);
TEST(teapot);
TEST(teddy);

int
main (void) {
  RUN(material);
  //RUN(simple);
  //RUN(teapot);
  //RUN(teddy);
  return 0;
}

