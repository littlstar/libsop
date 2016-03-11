#ifndef SOP_TEST_H
#define SOP_TEST_H

// macro to define test
#define TEST(name) int test_ ## name (void)

// macro to run a test
#define RUN(name) test_ ## name ();

// macro to define OBJ file format as a string
#define XOBJ(X) #X
#define OBJ(src) XOBJ(src)

#endif
