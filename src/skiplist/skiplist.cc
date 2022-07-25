

#include "skiplist.h"

namespace latte {

    template <typename Key, class Comparator>
    SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena)
        : compare_(cmp),
        arena_(arena),
        head_(NewNode(0 /* any key will do */, kMaxHeight)),
        max_height_(1),
        rnd_(0xdeadbeef) {
        for (int i = 0; i < kMaxHeight; i++) {
            head_->SetNext(i, nullptr);
        }
    }

    template <typename Key, class Comparator>
    bool SkipList<Key, Comparator>::Contains(const Key& key) const {
        Node* x = FindGreaterOrEqual(key, nullptr);
        if (x != nullptr && Equal(key, x->key)) {
            return true;
        } else {
            return false;
        }
    }

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(
        const Key& key, int height) {
        char* const node_memory = arena_->AllocateAligned(
            sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
        return new (node_memory) Node(key);
    }
};