



#ifndef __LATTE_C_PLUS_WRITE_H
#define __LATTE_C_PLUS_WRITE_H

#include "../batch/write_batch.h"

#include "env/env.h"
#include "util/aligned_storage.h"
#include "../types.h"
#include "../callback/write_callback.h"
#include "../callback/user_write_callback.h"
#include "../callback/post_mem_table_callback.h"
#include "../callback/pre_release_callback.h"
#include "../options/write_options.h"
#include "write_state.h"
#include "format/config.h"

namespace latte
{
    namespace rocksdb
    {
        struct WriteGroup;
        // Information kept for every waiting writer.
        struct Writer {
            WriteBatch* batch;
            bool sync;
            bool no_slowdown;
            bool disable_wal;
            Env::IOPriority rate_limiter_priority;
            bool disable_memtable;
            size_t batch_cnt;  // if non-zero, number of sub-batches in the write batch
            size_t protection_bytes_per_key;
            PreReleaseCallback* pre_release_callback;
            PostMemTableCallback* post_memtable_callback;
            uint64_t log_used;  // log number that this batch was inserted into
            uint64_t log_ref;   // log number that memtable insert should reference
            WriteCallback* callback;
            UserWriteCallback* user_write_cb;
            bool made_waitable;          // records lazy construction of mutex and cv
            std::atomic<uint8_t> state;  // write under StateMutex() or pre-link
            WriteGroup* write_group;
            SequenceNumber sequence;  // the sequence number to use for the first key
            Status status;
            Status callback_status;  // status returned by callback->Callback()

            aligned_storage<std::mutex>::type state_mutex_bytes;
            aligned_storage<std::condition_variable>::type state_cv_bytes;
            Writer* link_older;  // read/write only before linking, or as leader
            Writer* link_newer;  // lazy, read/write only before linking, or as leader

            Writer()
                : batch(nullptr),
                sync(false),
                no_slowdown(false),
                disable_wal(false),
                rate_limiter_priority(Env::IOPriority::IO_TOTAL),
                disable_memtable(false),
                batch_cnt(0),
                protection_bytes_per_key(0),
                pre_release_callback(nullptr),
                post_memtable_callback(nullptr),
                log_used(0),
                log_ref(0),
                callback(nullptr),
                user_write_cb(nullptr),
                made_waitable(false),
                state(Write_State::STATE_INIT),
                write_group(nullptr),
                sequence(kMaxSequenceNumber),
                link_older(nullptr),
                link_newer(nullptr) {}

            Writer(const WriteOptions& write_options, WriteBatch* _batch,
                WriteCallback* _callback, UserWriteCallback* _user_write_cb,
                uint64_t _log_ref, bool _disable_memtable, size_t _batch_cnt = 0,
                PreReleaseCallback* _pre_release_callback = nullptr,
                PostMemTableCallback* _post_memtable_callback = nullptr)
                : batch(_batch),
                // TODO: store a copy of WriteOptions instead of its seperated data
                // members
                // sync(write_options.sync),
                // no_slowdown(write_options.no_slowdown),
                // disable_wal(write_options.disableWAL),
                // rate_limiter_priority(write_options.rate_limiter_priority),
                disable_memtable(_disable_memtable),
                batch_cnt(_batch_cnt),
                // protection_bytes_per_key(_batch->GetProtectionBytesPerKey()),
                pre_release_callback(_pre_release_callback),
                post_memtable_callback(_post_memtable_callback),
                log_used(0),
                log_ref(_log_ref),
                callback(_callback),
                user_write_cb(_user_write_cb),
                made_waitable(false),
                state(STATE_INIT),
                write_group(nullptr),
                sequence(kMaxSequenceNumber),
                link_older(nullptr),
                link_newer(nullptr) {}

            ~Writer() {
                if (made_waitable) {
                    StateMutex().~mutex();
                    StateCV().~condition_variable();
                }
                status.PermitUncheckedError();
                callback_status.PermitUncheckedError();
            }

            bool CheckCallback(DB* db) {
                if (callback != nullptr) {
                    callback_status = callback->Callback(db);
                }
                return callback_status.ok();
            }

            void CheckWriteEnqueuedCallback() {
                if (user_write_cb != nullptr) {
                    user_write_cb->OnWriteEnqueued();
                }
            }

            void CheckPostWalWriteCallback() {
                if (user_write_cb != nullptr) {
                    user_write_cb->OnWalWriteFinish();
                }
            }

            void CreateMutex() {
                if (!made_waitable) {
                    // Note that made_waitable is tracked separately from state
                    // transitions, because we can't atomically create the mutex and
                    // link into the list.
                    made_waitable = true;
                    new (&state_mutex_bytes) std::mutex;
                    new (&state_cv_bytes) std::condition_variable;
                }
            }

            // returns the aggregate status of this Writer
            Status FinalStatus() {
                if (!status.ok()) {
                    // a non-ok memtable write status takes presidence
                    assert(callback == nullptr || callback_status.ok());
                    return status;
                } else if (!callback_status.ok()) {
                    // if the callback failed then that is the status we want
                    // because a memtable insert should not have been attempted
                    assert(callback != nullptr);
                    assert(status.ok());
                    return callback_status;
                } else {
                    // if there is no callback then we only care about
                    // the memtable insert status
                    assert(callback == nullptr || callback_status.ok());
                    return status;
                }
            }

            bool CallbackFailed() {
                return (callback != nullptr) && !callback_status.ok();
            }

            bool ShouldWriteToMemtable() {
                return status.ok() && !CallbackFailed() && !disable_memtable;
            }

            bool ShouldWriteToWAL() {
                return status.ok() && !CallbackFailed() && !disable_wal;
            }

            // No other mutexes may be acquired while holding StateMutex(), it is
            // always last in the order
            std::mutex& StateMutex() {
            assert(made_waitable);
            return *static_cast<std::mutex*>(static_cast<void*>(&state_mutex_bytes));
            }

            std::condition_variable& StateCV() {
            assert(made_waitable);
            return *static_cast<std::condition_variable*>(
                static_cast<void*>(&state_cv_bytes));
            }
        };

        struct AdaptationContext {
            const char* name;
            std::atomic<int32_t> value;

            explicit AdaptationContext(const char* name0) : name(name0), value(0) {}
        };

        
    } // namespace rocksdb
    
} // namespace latte



#endif