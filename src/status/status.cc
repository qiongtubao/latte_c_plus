

#include "status.h"

namespace latte
{
    std::unique_ptr<const char[]> Status::CopyState(const char* s) {
        const size_t cch = std::strlen(s) + 1;  // +1 for the null terminator
        char* rv = new char[cch];
        std::strncpy(rv, s, cch);
        return std::unique_ptr<const char[]>(rv);
    }

    static const char* msgs[static_cast<int>(Status::kMaxSubCode)] = {
        "",                                                   // kNone
        "Timeout Acquiring Mutex",                            // kMutexTimeout
        "Timeout waiting to lock key",                        // kLockTimeout
        "Failed to acquire lock due to max_num_locks limit",  // kLockLimit
        "No space left on device",                            // kNoSpace
        "Deadlock",                                           // kDeadlock
        "Stale file handle",                                  // kStaleFile
        "Memory limit reached",                               // kMemoryLimit
        "Space limit reached",                                // kSpaceLimit
        "No such file or directory",                          // kPathNotFound
        // KMergeOperandsInsufficientCapacity
        "Insufficient capacity for merge operands",
        // kManualCompactionPaused
        "Manual compaction paused",
        " (overwritten)",         // kOverwritten, subcode of OK
        "Txn not prepared",       // kTxnNotPrepared
        "IO fenced off",          // kIOFenced
        "Merge operator failed",  // kMergeOperatorFailed
        "Number of operands merged exceeded threshold",  // kMergeOperandThresholdExceeded
    };

    Status::Status(Code _code, SubCode _subcode, const Slice& msg,
               const Slice& msg2, Severity sev)
               : code_(_code),
            subcode_(_subcode),
            sev_(sev),
            retryable_(false),
            data_loss_(false),
            scope_(0) {
        assert(subcode_ != kMaxSubCode);
        const size_t len1 = msg.size();
        const size_t len2 = msg2.size();
        const size_t size = len1 + (len2 ? (2 + len2) : 0);
        char* const result = new char[size + 1];  // +1 for null terminator
        memcpy(result, msg.data(), len1);
        if (len2) {
            result[len1] = ':';
            result[len1 + 1] = ' ';
            memcpy(result + len1 + 2, msg2.data(), len2);
        }
        result[size] = '\0';  // null terminator for C style string
        state_.reset(result);
    }    

    Status::Status(const Status& s, Severity sev)
        : code_(s.code_),
        subcode_(s.subcode_),
        sev_(sev),
        retryable_(s.retryable_),
        data_loss_(s.data_loss_),
        scope_(s.scope_) {
        s.MarkChecked();
        state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_.get());
    }    
} // namespace latte
