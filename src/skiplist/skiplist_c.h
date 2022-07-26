

#include "skiplist.h"
#include "skiplistnode_c.h"
#include "skiplistitertor_c.h"
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

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key,
                                                Node** prev) const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            Node* next = x->Next(level);
            if (KeyIsAfterNode(key, next)) {
            // Keep searching in this list
            x = next;
            } else {
            if (prev != nullptr) prev[level] = x;
            if (level == 0) {
                return next;
            } else {
                // Switch to next list
                level--;
            }
            }
        }
    }

    template <typename Key, class Comparator>
    bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* n) const {
        // null n is considered infinite
        return (n != nullptr) && (compare_(n->key, key) < 0);
    }


    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            assert(x == head_ || compare_(x->key, key) < 0);
            Node* next = x->Next(level);
            if (next == nullptr || compare_(next->key, key) >= 0) {
            if (level == 0) {
                return x;
            } else {
                // Switch to next list
                level--;
            }
            } else {
            x = next;
            }
        }
    }


    template <typename Key, class Comparator>
    int SkipList<Key, Comparator>::RandomHeight() {
        // Increase height with probability 1 in kBranching
        static const unsigned int kBranching = 4;
        int height = 1;
        while (height < kMaxHeight && rnd_.OneIn(kBranching)) {
            height++;
        }
        assert(height > 0);
        assert(height <= kMaxHeight);
        return height;
    }


    template <typename Key, class Comparator>
    void SkipList<Key, Comparator>::Insert(const Key& key) {
        // TODO(opt): We can use a barrier-free variant of FindGreaterOrEqual()
        // here since Insert() is externally synchronized.
        Node* prev[kMaxHeight];
        Node* x = FindGreaterOrEqual(key, prev);

        // Our data structure does not allow duplicate insertion
        assert(x == nullptr || !Equal(key, x->key));

        int height = RandomHeight();
        if (height > GetMaxHeight()) {
            for (int i = GetMaxHeight(); i < height; i++) {
            prev[i] = head_;
            }
            // It is ok to mutate max_height_ without any synchronization
            // with concurrent readers.  A concurrent reader that observes
            // the new value of max_height_ will see either the old value of
            // new level pointers from head_ (nullptr), or a new value set in
            // the loop below.  In the former case the reader will
            // immediately drop to the next level since nullptr sorts after all
            // keys.  In the latter case the reader will use the new node.
            max_height_.store(height, std::memory_order_relaxed);
        }

        x = NewNode(key, height);
        for (int i = 0; i < height; i++) {
            // NoBarrier_SetNext() suffices since we will add a barrier when
            // we publish a pointer to "x" in prev[i].
            x->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
            prev[i]->SetNext(i, x);
        }
    }

};