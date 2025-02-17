



#pragma once 
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include "slice/slice.h"
#include <iostream>


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

        static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
            return Status(kNotSupported, msg, msg2);
        }

        static Status NotSupported(SubCode msg = kNone) {
            return Status(kNotSupported, msg);
        }

        // Return error status of an appropriate type.
        static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
            return Status(kNotFound, msg, msg2);
        }
        // Fast path for not found without malloc;
        static Status NotFound(SubCode msg = kNone) { return Status(kNotFound, msg); }

        static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
            return Status(kInvalidArgument, msg, msg2);
        }

        static Status InvalidArgument(SubCode msg = kNone) {
            return Status(kInvalidArgument, msg);
        }

        static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
            return Status(kCorruption, msg, msg2);
        }

        static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
            return Status(kIOError, msg, msg2);
        }
        static Status IOError(SubCode msg = kNone) { return Status(kIOError, msg); }

        std::string ToString() const;
        public:
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
            MarkChecked();
            return code_;
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

        // Returns true iff the status indicates a NotFound error.
        bool IsNotFound() const {
            MarkChecked();
            return code() == kNotFound;
        }
        
        // Returns true iff the status indicates an IOError.
        bool IsIOError() const {
            MarkChecked();
            return code() == kIOError;
        }

        // Returns true iff the status indicates a Corruption error.
        bool IsCorruption() const {
            MarkChecked();
            return code() == kCorruption;
        }
        

        // Status& operator=(Status&& s) noexcept;

        // bool operator==(const Status& rhs) const;

        // bool operator!=(const Status& rhs) const;

        public:
            bool ok() const { return (state_ == nullptr); }
        
            static Status OK() { return Status(); }

            explicit Status(Code _code, SubCode _subcode = kNone)
                : code_(_code),
                    subcode_(_subcode),
                    sev_(kNoError),
                    retryable_(false),
                    data_loss_(false),
                    scope_(0) {}

            explicit Status(Code _code, SubCode _subcode, bool retryable, bool data_loss,
                    unsigned char scope)
                : code_(_code),
                    subcode_(_subcode),
                    sev_(kNoError),
                    retryable_(retryable),
                    data_loss_(data_loss),
                    scope_(scope) {}
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
    };

    
    // inline Status& Status::operator=(Status&& s) noexcept {
    //     if (this != &s) {
    //         s.MarkChecked();
    //         MustCheck();
    //         code_ = std::move(s.code_);
    //         s.code_ = kOk;
    //         subcode_ = std::move(s.subcode_);
    //         s.subcode_ = kNone;
    //         sev_ = std::move(s.sev_);
    //         s.sev_ = kNoError;
    //         retryable_ = std::move(s.retryable_);
    //         s.retryable_ = false;
    //         data_loss_ = std::move(s.data_loss_);
    //         s.data_loss_ = false;
    //         scope_ = std::move(s.data_loss_);
    //         s.scope_ = 0;
    //         state_ = std::move(s.state_);
    //     }
    //     return *this;
    // }

    // inline bool Status::operator==(const Status& rhs) const {
    //     MarkChecked();
    //     rhs.MarkChecked();
    //     return (code_ == rhs.code_);
    // }

    // inline bool Status::operator!=(const Status& rhs) const {
    //     MarkChecked();
    //     rhs.MarkChecked();
    //     return !(*this == rhs);
    // }


} // namespace latte
