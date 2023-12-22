#include "lock/ring_buffer.h"

#include <gtest/gtest.h>

struct A {
  int val = 0;
};

TEST(Config, RingBufferEmpty) {
  locks::RingBuffer<A, 5> buf;
  EXPECT_TRUE(buf.empty());
  EXPECT_FALSE(buf.full());
}

TEST(Config, RingBufferAddElements) {
  locks::RingBuffer<A, 5> buf;
  EXPECT_TRUE(buf.push(A({1})));

  EXPECT_FALSE(buf.empty());
  EXPECT_FALSE(buf.full());

  EXPECT_EQ(0, buf.getHeadIdx());
  EXPECT_EQ(0, buf.getTailIdx());

  EXPECT_TRUE(buf.push(A({2})));

  EXPECT_FALSE(buf.empty());
  EXPECT_FALSE(buf.full());

  EXPECT_EQ(0, buf.getHeadIdx());
  EXPECT_EQ(1, buf.getTailIdx());
}

TEST(Config, RingBufferFull1) {
  locks::RingBuffer<A, 5> buf;
  for (int i : {1, 2, 3, 4, 5}) {
    EXPECT_TRUE(buf.push(A({i})));
  }
  EXPECT_FALSE(buf.empty());
  EXPECT_TRUE(buf.full());

  EXPECT_EQ(0, buf.getHeadIdx());
  EXPECT_EQ(4, buf.getTailIdx());
}

TEST(Config, RingBufferFull2) {
  locks::RingBuffer<A, 5> buf;
  for (int i : {1, 2, 3, 4, 5}) {
    EXPECT_TRUE(buf.push(A({i})));
  }

  A a;
  EXPECT_TRUE(buf.pop(a));
  EXPECT_TRUE(buf.push(A({6})));

  EXPECT_FALSE(buf.empty());
  EXPECT_TRUE(buf.full());

  EXPECT_EQ(1, buf.getHeadIdx());
  EXPECT_EQ(0, buf.getTailIdx());
}

TEST(Config, RingBufferPushToFull) {
  locks::RingBuffer<A, 5> buf;
  for (int i : {1, 2, 3, 4, 5}) {
    EXPECT_TRUE(buf.push(A({i})));
  }

  EXPECT_FALSE(buf.push(A({6})));
}

TEST(Config, RingBufferPopFromEmpty) {
  locks::RingBuffer<A, 5> buf;

  A a;
  EXPECT_FALSE(buf.pop(a));
}

TEST(Config, RingBufferPopElements) {
  locks::RingBuffer<A, 5> buf;
  for (int i : {1, 2, 3}) {
    buf.push(A({i}));
  }

  EXPECT_EQ(0, buf.getHeadIdx());
  EXPECT_EQ(2, buf.getTailIdx());

  A a;
  EXPECT_TRUE(buf.pop(a));
  EXPECT_EQ(1, buf.getHeadIdx());
  EXPECT_EQ(2, buf.getTailIdx());
  EXPECT_EQ(1, a.val);

  EXPECT_TRUE(buf.pop(a));
  EXPECT_EQ(2, buf.getHeadIdx());
  EXPECT_EQ(2, buf.getTailIdx());
  EXPECT_EQ(2, a.val);
  EXPECT_FALSE(buf.empty());
  EXPECT_FALSE(buf.full());

  EXPECT_TRUE(buf.pop(a));
  EXPECT_EQ(3, a.val);
  EXPECT_TRUE(buf.empty());
  EXPECT_FALSE(buf.full());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
