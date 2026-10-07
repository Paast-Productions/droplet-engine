#include "core/ThreadSafeQueue.hpp"
#include <gtest/gtest.h>

#include <thread>
#include <vector>
#include <atomic>

using namespace Droplet;

TEST(ThreadSafeQueueTest, StartsEmpty)
{
    ThreadSafeQueue<int> queue;
    EXPECT_TRUE(queue.IsEmpty());
    
    int item;
    EXPECT_FALSE(queue.Pop(item));
}

TEST(ThreadSafeQueueTest, PushAndPopSingleItem)
{
    ThreadSafeQueue<int> queue;
    
    queue.Push(42);
    EXPECT_FALSE(queue.IsEmpty());
    
    int item = 0;
    EXPECT_TRUE(queue.Pop(item));
    EXPECT_EQ(item, 42);
    EXPECT_TRUE(queue.IsEmpty());
}

TEST(ThreadSafeQueueTest, MaintainsFIFOOrder)
{
    ThreadSafeQueue<int> queue;
    
    queue.Push(1);
    queue.Push(2);
    queue.Push(3);
    
    int item;
    ASSERT_TRUE(queue.Pop(item)); EXPECT_EQ(item, 1);
    ASSERT_TRUE(queue.Pop(item)); EXPECT_EQ(item, 2);
    ASSERT_TRUE(queue.Pop(item)); EXPECT_EQ(item, 3);
    
    EXPECT_FALSE(queue.Pop(item));
}

TEST(ThreadSafeQueueTest, ConcurrentPush)
{
    ThreadSafeQueue<int> queue;
    constexpr int numThreads = 10;
    constexpr int itemsPerThread = 1000;
    
    std::vector<std::thread> threads;
    
    // Spawn 10 threads, each pushing 1000 items simultaneously
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&queue]()
        {
            for (int j = 0; j < itemsPerThread; ++j)
            {
                queue.Push(1); // Push the number 1 to count them later
            }
        });
    }
    
    // Wait for all threads to finish pushing
    for (auto& t : threads) t.join();
    
    // Verify no items were lost to race conditions
    int totalItems = 0;
    int item;
    while (queue.Pop(item))
    {
        totalItems++;
    }
    
    EXPECT_EQ(totalItems, numThreads * itemsPerThread);
}

TEST(ThreadSafeQueueTest, ConcurrentPop)
{
    ThreadSafeQueue<int> queue;
    constexpr int totalItems = 10000;
    
    // Pre-fill the queue
    for (int i = 0; i < totalItems; ++i)
    {
        queue.Push(1);
    }
    
    std::atomic<int> successfullyPopped{0};
    constexpr int numThreads = 10;
    std::vector<std::thread> threads;
    
    // Spawn 10 threads trying to pop the same items simultaneously
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&queue, &successfullyPopped]()
        {
            int item;
            while (queue.Pop(item))
            {
                successfullyPopped.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) t.join();
    
    // Verify exactly 10,000 items were popped, no double-pops occurred, and the queue is empty
    EXPECT_EQ(successfullyPopped.load(), totalItems);
    EXPECT_TRUE(queue.IsEmpty());
}