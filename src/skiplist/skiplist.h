
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
            Node* FindGreaterOrEqual(const Key& key, Node** prev) const;
            bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }
            Node* FindLast() const;
            Node* FindLessThan(const Key& key) const;
        private: //method
            Node* NewNode(const Key& key, int height);
            inline int GetMaxHeight() const {
                return max_height_.load(std::memory_order_relaxed);
            }
            bool KeyIsAfterNode(const Key& key, Node* n) const;
        private: //enum
            enum { kMaxHeight = 12 };
            int RandomHeight();
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
            // Node* Next(int n);
            Node* Next(int n) {
                assert(n >= 0);
                // Use an 'acquire load' so that we observe a fully initialized
                // version of the returned Node.
                return next_[n].load(std::memory_order_acquire);
            };

            // No-barrier variants that can be safely used in a few locations.
            Node* NoBarrier_Next(int n) {
                assert(n >= 0);
                return next_[n].load(std::memory_order_relaxed);
            }

            void NoBarrier_SetNext(int n, Node* x) {
                assert(n >= 0);
                next_[n].store(x, std::memory_order_relaxed);
            }
    };


    
}