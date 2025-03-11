
#pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include "slice/slice.h"


namespace latte
{
    class Status {
        public:
            Status(): code_(kOk),
                subcode_(kNone),
                sev_(kNoError),
                retryable_(false),
                data_loss_(false),
                scope_(0),
                state_(nullptr) {

            }
            Status(const Status& s);
            Status& operator=(const Status& s);
            Status(Status&& s) noexcept;
            Status& operator=(Status&& s) noexcept;
            bool operator==(const Status& rhs) const;
            bool operator!=(const Status& rhs) const;

            
            // In case of intentionally swallowing an error, user must explicitly call
            // this function. That way we are easily able to search the code to find where
            // error swallowing occurs.
            inline void PermitUncheckedError() const { MarkChecked(); }
            inline void MustCheck() const {
            #ifdef ROCKSDB_ASSERT_STATUS_CHECKED
                checked_ = false;
            #endif  // ROCKSDB_ASSERT_STATUS_CHECKED
            }

            inline void MarkChecked() const {
            #ifdef ROCKSDB_ASSERT_STATUS_CHECKED
                checked_ = true;
            #endif  // ROCKSDB_ASSERT_STATUS_CHECKED
            }
            

        enum Code : unsigned char { //error 状态  
            kOk = 0,
            kNotFound = 1,
            kCorruption = 2,
            kNotSupported = 3,
            kInvalidArgument = 4,
            kIOError = 5,
            kMergeInProgress = 6,
            kIncomplete = 7,
            kShutdownInProgress = 8,
            kTimedOut = 9,
            kAborted = 10,
            kBusy = 11,
            kExpired = 12,
            kTryAgain = 13,
            kCompactionTooLarge = 14,
            kColumnFamilyDropped = 15,
            kMaxCode
        };

        enum SubCode : unsigned char {
            kNone = 0,
            kMutexTimeout = 1,
            kLockTimeout = 2,
            kLockLimit = 3,
            kNoSpace = 4,
            kDeadlock = 5,
            kStaleFile = 6,
            kMemoryLimit = 7,
            kSpaceLimit = 8,
            kPathNotFound = 9,
            KMergeOperandsInsufficientCapacity = 10,
            kManualCompactionPaused = 11,
            kOverwritten = 12,
            kTxnNotPrepared = 13,
            kIOFenced = 14,
            kMergeOperatorFailed = 15,
            kMergeOperandThresholdExceeded = 16,
            kMaxSubCode
        };

        enum Severity : unsigned char {
            kNoError = 0,
            kSoftError = 1,
            kHardError = 2,
            kFatalError = 3,
            kUnrecoverableError = 4,
            kMaxSeverity
        };
        bool IsIOFenced() const {
            MarkChecked();
            return (code() == kIOError) && (subcode() == kIOFenced);
        }

        std::string ToString() const;
        protected:
            Code code_;
            SubCode subcode_;
            Severity sev_;
            bool retryable_;
            bool data_loss_;
            unsigned char scope_;
            // nullptr state_（至少对于 OK 而言是这种情况）表示额外的
            // 消息为空。
            std::unique_ptr<const char[]> state_;
        
        Code code() const {
            return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
        }

        SubCode subcode() const {
            MarkChecked();
            return subcode_;
        }

        
        static std::unique_ptr<const char[]> CopyState(const char* s);

        // OK status has a null state_.  Otherwise, state_ is a new[] array
        // of the following form:
        //    state_[0..3] == length of message
        //    state_[4]    == code
        //    state_[5..]  == message
        // static std::unique_ptr<const char[]> state_;

        Status(Code _code, SubCode _subcode, const Slice& msg,
               const Slice& msg2, Severity sev = kNoError);
        Status(Code _code, const Slice& msg, const Slice& msg2)
            : Status(_code, kNone, msg, msg2) {};

        Status(const Status& s, Severity sev);
    };

    inline Status::Status(const Status& s)
        : code_(s.code_),
          subcode_(s.subcode_),
          sev_(s.sev_),
          retryable_(s.retryable_),
          data_loss_(s.data_loss_),
          scope_(s.scope_) {
        s.MarkChecked();
        state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_.get());
    }

    


} // namespace latte
