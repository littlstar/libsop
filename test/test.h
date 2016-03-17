#ifndef SOP_TEST_H
#define SOP_TEST_H

// macro to define test
#define TEST(TEST_NAME) int test_ ## TEST_NAME (void)

// macro to run a test
#define RUN(TEST_NAME) test_ ## TEST_NAME ();

// macro to define OBJ file format as a string
#define XOBJ(X) #X
#define OBJ(src) XOBJ(src)

#endif
