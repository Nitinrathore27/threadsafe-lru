#include <gtest/gtest.h>
#include "LRUCache.h"
#include <string>
#include <thread>
#include <vector>

// Test Instantiation & Empty Cache ---
TEST(LRUCacheTest, EmptyCacheReturnsNullopt)
{
    LRUCache<int, std::string> cache(2);

    // Using structured bindings or auto for cleaner C++17 code
    auto result = cache.get(1);

    EXPECT_FALSE(result.has_value());
}

// Test Basic Put and Get ---
TEST(LRUCacheTest, PutAndGetSingleElement)
{
    LRUCache<int, std::string> cache(2);
    cache.put(1, "one");

    auto result = cache.get(1);

    // Use ASSERT_TRUE because if it's false, calling .value() will crash the test
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "one");
}

// Test LRU Eviction Policy ---
TEST(LRUCacheTest, EvictsLeastRecentlyUsed)
{
    LRUCache<int, int> cache(2); // Capacity of 2

    cache.put(1, 10); // Cache: [1]
    cache.put(2, 20); // Cache: [2, 1]
    cache.put(3, 30); // Cache over capacity! Evicts 1. Cache: [3, 2]

    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_TRUE(cache.get(2).has_value());
    EXPECT_TRUE(cache.get(3).has_value());
}

// Test Updating an Existing Key ---
TEST(LRUCacheTest, UpdateExistingKeyUpdatesValueAndMovesToFront)
{
    LRUCache<int, int> cache(2);

    cache.put(1, 10);
    cache.put(2, 20); // Cache: [2, 1]

    // Update key 1. This should change its value AND make it Most Recently Used
    cache.put(1, 15); // Cache: [1, 2]

    // Add a 3rd key. Because 1 was recently updated, 2 should be evicted!
    cache.put(3, 30); // Cache: [3, 1]

    auto result = cache.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 15); // Verify value was updated

    EXPECT_FALSE(cache.get(2).has_value()); // Verify 2 was the one evicted
}

// Test Concurrent Puts ---
TEST(LRUCacheTest, ConcurrentPutsDoNotCrash)
{
    // Create a cache large enough to hold all items so no eviction happens yet.
    // We just want to test if simultaneous inserts corrupt the data structures.
    const int num_threads = 10;
    const int inserts_per_thread = 100;
    LRUCache<int, std::string> cache(num_threads * inserts_per_thread);

    std::vector<std::thread> threads;

    // 1. Spawn multiple threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&cache, i, inserts_per_thread]() {
            for (int j = 0; j < inserts_per_thread; ++j) {
                // Generate a unique key for every single insertion
                int key = i * inserts_per_thread + j;
                cache.put(key, "value_" + std::to_string(key));
            }
        });
    }

    // 2. Wait for all threads to finish their work
    for (auto& t : threads) {
        t.join();
    }

    // 3. Verify the data is completely intact
    for (int i = 0; i < num_threads; ++i) {
        for (int j = 0; j < inserts_per_thread; ++j) {
            int key = i * inserts_per_thread + j;
            auto result = cache.get(key);
            
            // If the cache isn't thread-safe, some of these might mysteriously be missing
            ASSERT_TRUE(result.has_value()) << "Missing key: " << key;
            EXPECT_EQ(result.value(), "value_" + std::to_string(key));
        }
    }
}