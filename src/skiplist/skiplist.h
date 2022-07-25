
#include <atomic>
#include <cassert>
#include <cstdlib>

#include "arena/arena.h"
#include "arena/random.h"

namespace latte {
    template <typename Key, class Comparator>
    class SkipList {
        private:
            struct Node;
        public: //struct
            struct Iterator;
        public: //method
            explicit SkipList(Comparator cmp, Arena* arena);
            SkipList(const SkipList&) = delete;
            bool Contains(const Key& key) const;
            void Insert(const Key& key);

        private: //method
            Node* NewNode(const Key& key, int height);


        private: //enum
            enum { kMaxHeight = 12 };
        private:
            Comparator const compare_;
            Arena * const arena_;
            Node* const head_;
            std::atomic<int> max_height_;
            Random rnd_;
    };

    template <typename Key, class Comparator> 
    struct SkipList<Key, Comparator>::Iterator {
        public:
            explicit Iterator(const SkipList* SkipList);
            bool Valid() const;
            const Key& key() const;
            void Next();
            void Prev();
            void Seek(const Key& target);
            void SeekToFirst();
            void SeekToLast();
        private:
        const SkipList* list_;
        Node* node_;
    };

    template <typename Key, class Comparator>
    struct SkipList<Key, Comparator>::Node {
        explicit Node(const Key& k) : key(k) {}
        Key const key;

        private:
            // Array of length equal to the node height.  next_[0] is lowest level link.
            std::atomic<Node*> next_[1];
        public:
            void SetNext(int n, Node* x);
    };
    
}