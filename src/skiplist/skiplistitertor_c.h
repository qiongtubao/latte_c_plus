
namespace latte {
    template <typename Key, class Comparator>
    inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
        list_ = list;
        node_ = nullptr;
    }

    template <typename Key, class Comparator>
    inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
        return node_ != nullptr;
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
        node_ = list_->head_->Next(0);
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
        node_ = list_->FindGreaterOrEqual(target, nullptr);
    }


    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast() const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            Node* next = x->Next(level);
            if (next == nullptr) {
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
    inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
        node_ = list_->FindLast();
        if (node_ == list_->head_) {
            node_ = nullptr;
        }
    }

    template <typename Key, class Comparator>
    inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
        assert(Valid());
        return node_->key;
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Prev() {
        // Instead of using explicit "prev" links, we just search for the
        // last node that falls before key.
        assert(Valid());
        node_ = list_->FindLessThan(node_->key);
        if (node_ == list_->head_) {
            node_ = nullptr;
        }
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Next() {
        assert(Valid());
        node_ = node_->Next(0);
    }

    
}