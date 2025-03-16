
#include <string>
#include <atomic>
namespace latte
{
    class AllocTracker {
        public:
            // explicit AllocTracker(WriteBufferManager* write_buffer_manager);
            explicit AllocTracker();

            AllocTracker(const AllocTracker&) = delete;
            void operator=(const AllocTracker&) = delete;
            ~AllocTracker();
            void Allocate(size_t bytes);

            void DoneAllocating();
            void FreeMem();
            bool is_freed() const { return freed_; }

        private:
            // WriteBufferManager* write_buffer_manager_;
            std::atomic<size_t> bytes_allocated_;
            bool done_allocating_;
            bool freed_;

    };
} // namespace latte
