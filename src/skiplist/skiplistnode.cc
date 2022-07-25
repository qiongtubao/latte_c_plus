
#include "skiplist.h"
#include <assert.h>

namespace latte {
    template <typename Key, class Comparator>
    void SkipList<Key, Comparator>::Node::SetNext(int n, Node* x) {
        assert(n >= 0);
        // Use a 'release store' so that anybody who reads through this
        // pointer observes a fully initialized version of the inserted node.
        next_[n].store(x, std::memory_order_release);
    }
}