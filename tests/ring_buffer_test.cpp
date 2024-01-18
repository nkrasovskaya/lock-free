#include "lock/ring_buffer.h"

#include <gtest/gtest.h>

struct A {
  int val = 0;
};

TEST(Config, RingBufferEmpty) {
  locks::RingBuffer<A> buf(5);
  EXPECT_TRUE(buf.Empty());
  EXPECT_FALSE(buf.Full());
}

TEST(Config, RingBufferAddElements) {
  locks::RingBuffer<A> buf(5);
  EXPECT_TRUE(buf.Enqueue(A({1})));

  EXPECT_FALSE(buf.Empty());
  EXPECT_FALSE(buf.Full());

  EXPECT_EQ(0, buf.GetHeadIdx());
  EXPECT_EQ(0, buf.GetTailIdx());

  EXPECT_TRUE(buf.Enqueue(A({2})));

  EXPECT_FALSE(buf.Empty());
  EXPECT_FALSE(buf.Full());

  EXPECT_EQ(0, buf.GetHeadIdx());
  EXPECT_EQ(1, buf.GetTailIdx());
}

TEST(Config, RingBufferFull1) {
  locks::RingBuffer<A> buf(5);
  for (int i : {1, 2, 3, 4, 5}) {
    EXPECT_TRUE(buf.Enqueue(A({i})));
  }
  EXPECT_FALSE(buf.Empty());
  EXPECT_TRUE(buf.Full());

  EXPECT_EQ(0, buf.GetHeadIdx());
  EXPECT_EQ(4, buf.GetTailIdx());
}

TEST(Config, RingBufferFull2) {
  locks::RingBuffer<A> buf(5);
  for (int i : {1, 2, 3, 4, 5}) {
    EXPECT_TRUE(buf.Enqueue(A({i})));
  }

  A a;
  EXPECT_TRUE(buf.Dequeue(a));
  EXPECT_TRUE(buf.Enqueue(A({6})));

  EXPECT_FALSE(buf.Empty());
  EXPECT_TRUE(buf.Full());

  EXPECT_EQ(1, buf.GetHeadIdx());
  EXPECT_EQ(0, buf.GetTailIdx());
}

TEST(Config, RingBufferEnqueueToFull) {
  locks::RingBuffer<A> buf(5);
  for (int i : {1, 2, 3, 4, 5}) {
    EXPECT_TRUE(buf.Enqueue(A({i})));
  }

  EXPECT_FALSE(buf.Enqueue(A({6})));
}

TEST(Config, RingBufferDequeueFromEmpty) {
  locks::RingBuffer<A> buf(5);

  A a;
  EXPECT_FALSE(buf.Dequeue(a));
}

TEST(Config, RingBufferDequeueElements) {
  locks::RingBuffer<A> buf(5);
  for (int i : {1, 2, 3}) {
    buf.Enqueue(A({i}));
  }

  EXPECT_EQ(0, buf.GetHeadIdx());
  EXPECT_EQ(2, buf.GetTailIdx());

  A a;
  EXPECT_TRUE(buf.Dequeue(a));
  EXPECT_EQ(1, buf.GetHeadIdx());
  EXPECT_EQ(2, buf.GetTailIdx());
  EXPECT_EQ(1, a.val);

  EXPECT_TRUE(buf.Dequeue(a));
  EXPECT_EQ(2, buf.GetHeadIdx());
  EXPECT_EQ(2, buf.GetTailIdx());
  EXPECT_EQ(2, a.val);
  EXPECT_FALSE(buf.Empty());
  EXPECT_FALSE(buf.Full());

  EXPECT_TRUE(buf.Dequeue(a));
  EXPECT_EQ(3, a.val);
  EXPECT_TRUE(buf.Empty());
  EXPECT_FALSE(buf.Full());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
