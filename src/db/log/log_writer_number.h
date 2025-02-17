#ifndef __LATTE_C_PLUS_LOG_WRITER_NUMBER_H
#define __LATTE_C_PLUS_LOG_WRITER_NUMBER_H


#include "log_writer.h"

namespace latte
{
    namespace rocksdb
    {
        struct LogWriterNumber {
            // pass ownership of _writer
            LogWriterNumber(uint64_t _number, LogWriter* _writer)
                : number(_number), writer(_writer) {}

            LogWriter* ReleaseWriter() {
                auto* w = writer;
                writer = nullptr;
                return w;
            }
            Status ClearWriter() {
                Status s;
                if (writer->file()) {
                    // TODO: plumb Env::IOActivity, Env::IOPriority
                    s = writer->WriteBuffer(WriteOptions());
                }
                delete writer;
                writer = nullptr;
                return s;
            }

            bool IsSyncing() { return getting_synced; }

            uint64_t GetPreSyncSize() {
                assert(getting_synced);
                return pre_sync_size;
            }

            void PrepareForSync() {
                assert(!getting_synced);
                // Ensure the head of logs_ is marked as getting_synced if any is.
                getting_synced = true;
                // If last sync failed on a later WAL, this could be a fully synced
                // and closed WAL that just needs to be recorded as synced in the
                // manifest.
                if (writer->file()) {
                    // Size is expected to be monotonically increasing.
                    assert(writer->file()->GetFlushedSize() >= pre_sync_size);
                    pre_sync_size = writer->file()->GetFlushedSize();
                }
            }

            void FinishSync() {
                assert(getting_synced);
                getting_synced = false;
            }

            uint64_t number;
            // Visual Studio doesn't support deque's member to be noncopyable because
            // of a std::unique_ptr as a member.
            LogWriter* writer;  // own

        private:
            // true for some prefix of logs_
            bool getting_synced = false;
            // The size of the file before the sync happens. This amount is guaranteed
            // to be persisted even if appends happen during sync so it can be used for
            // tracking the synced size in MANIFEST.
            uint64_t pre_sync_size = 0;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif