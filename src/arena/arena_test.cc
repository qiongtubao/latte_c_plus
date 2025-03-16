

#include <gtest/gtest.h>
#include "./arena.h"

namespace latte
{
    class Random {
        private:
        uint32_t seed_;

        public:
        explicit Random(uint32_t s) : seed_(s & 0x7fffffffu) {
            // Avoid bad seeds.
            if (seed_ == 0 || seed_ == 2147483647L) {
            seed_ = 1;
            }
        }
        uint32_t Next() {
            static const uint32_t M = 2147483647L;  // 2^31-1
            static const uint64_t A = 16807;        // bits 14, 8, 7, 5, 2, 1, 0
            // We are computing
            //       seed_ = (seed_ * A) % M,    where M = 2^31-1
            //
            // seed_ must not be zero or M, or else all subsequent computed values
            // will be zero or M respectively.  For all other values, seed_ will end
            // up cycling through every number in [1,M-1]
            uint64_t product = seed_ * A;

            // Compute (product % M) using the fact that ((x << 31) % M) == x.
            seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
            // The first reduction may overflow by 1 bit, so we may need to
            // repeat.  mod == M is not possible; using > allows the faster
            // sign-bit-based test.
            if (seed_ > M) {
            seed_ -= M;
            }
            return seed_;
        }
        // Returns a uniformly distributed value in the range [0..n-1]
        // REQUIRES: n > 0
        uint32_t Uniform(int n) { return Next() % n; }

        // Randomly returns true ~"1/n" of the time, and false otherwise.
        // REQUIRES: n > 0
        bool OneIn(int n) { return (Next() % n) == 0; }

        // Skewed: pick "base" uniformly from range [0,max_log] and then
        // return "base" random bits.  The effect is to pick a number in the
        // range [0,2^max_log-1] with exponential bias towards smaller numbers.
        uint32_t Skewed(int max_log) { return Uniform(1 << Uniform(max_log + 1)); }
        };

    TEST(Arena_TEST, Empty){
        std::vector<std::pair<size_t, char*>> allocated;
        Arena arena;
        const int N = 100000;
        size_t bytes = 0;
        Random rnd(301);
        for (int i = 0; i < N; i++) {
            size_t s;
            if (i % (N / 10) == 0) {
                s = i;
            } else {
                s = rnd.OneIn(4000)
                        ? rnd.Uniform(6000)
                        : (rnd.OneIn(10) ? rnd.Uniform(100) : rnd.Uniform(20));
            }
            if (s == 0) {
                // Our arena disallows size 0 allocations.
                s = 1;
            }
            char* r;
            if (rnd.OneIn(10)) {
                r = arena.AllocateAligned(s);
            } else {
                r = arena.Allocate(s);
            }

                for (size_t b = 0; b < s; b++) {
                // Fill the "i"th allocation with a known bit pattern
                    r[b] = i % 256;
                }
                bytes += s;
                allocated.push_back(std::make_pair(s, r));
                ASSERT_GE(arena.MemoryUsage(), bytes);
            if (i > N / 10) {
                ASSERT_LE(arena.MemoryUsage(), bytes * 1.10);
            }
        }
        for (size_t i = 0; i < allocated.size(); i++) {
            size_t num_bytes = allocated[i].first;
            const char* p = allocated[i].second;
            for (size_t b = 0; b < num_bytes; b++) {
                // Check the "i"th allocation for the known bit pattern
                ASSERT_EQ(int(p[b]) & 0xff, i % 256);
            }
        }
    }
} // namespace latte
