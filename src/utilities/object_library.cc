

#include "object_library.h"

namespace latte 
{
    size_t ObjectLibrary::GetFactoryCount(size_t *types) const {
        std::unique_lock<std::mutex> lock(mu_);
        *types = factories_.size();
        size_t factories = 0;
        for (const auto &e : factories_) {
            factories += e.second.size();
        }
        return factories;
    }       

    size_t ObjectLibrary::GetFactoryCount(const std::string &type) const {
        std::unique_lock<std::mutex> lock(mu_);
        auto iter = factories_.find(type);
        if (iter != factories_.end()) {
            return iter->second.size();
        } else {
            return 0;
        }
    }
} // namespace latte
