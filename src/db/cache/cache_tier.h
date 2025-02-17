


#ifndef __LATTE_C_PLUS_CACHE_TIER_H
#define __LATTE_C_PLUS_CACHE_TIER_H


#include <cstdint>
namespace latte
{
    // The control option of how the cache tiers will be used. Currently rocksdb
    // support block cache (volatile tier), secondary cache (non-volatile tier).
    // In the future, we may add more caching layers.
    enum class CacheTier : uint8_t {
        kVolatileTier = 0,
        kVolatileCompressedTier = 0x01,
        kNonVolatileBlockTier = 0x02,
    };
} // namespace latte


#endif