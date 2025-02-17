
#ifndef __LATTE_C_PLUS_ENV_POSIX_H
#define __LATTE_C_PLUS_ENV_POSIX_H

#include <vector>
#include <mutex>
#include "env.h"
#include "thread_pool/thread_pool_imp.h"

namespace latte
{
    class PosixEnv {
        public:
            struct JoinThreadsOnExit {
                explicit JoinThreadsOnExit(PosixEnv& _deflt) : deflt(_deflt) {}
                ~JoinThreadsOnExit() {
                    for (const auto tid : deflt.threads_to_join_) {
                        pthread_join(tid, nullptr);
                    }
                    for (int pool_id = 0; pool_id < Priority::TOTAL; ++pool_id) {
                        deflt.thread_pools_[pool_id].JoinAllThreads();
                    }
                    // Do not delete the thread_status_updater_ in order to avoid the
                    // free after use when Env::Default() is destructed while some other
                    // child threads are still trying to update thread status. All
                    // PosixEnv instances use the same thread_status_updater_, so never
                    // explicitly delete it.
                }
                PosixEnv& deflt;
            }; 

        private:
            // friend Env* Env::Default();
            // Constructs the default Env, a singleton
            PosixEnv();

            // The below 4 members are only used by the default PosixEnv instance.
            // Non-default instances simply maintain references to the backing
            // members in te default instance
            std::vector<ThreadPoolImpl> thread_pools_storage_;
            pthread_mutex_t mu_storage_;
            std::vector<pthread_t> threads_to_join_storage_;
            bool allow_non_owner_access_storage_;

            std::vector<ThreadPoolImpl>& thread_pools_;
            pthread_mutex_t& mu_;
            std::vector<pthread_t>& threads_to_join_;
            // If true, allow non owner read access for db files. Otherwise, non-owner
            //  has no access to db files.
            bool& allow_non_owner_access_;
    };
} // namespace latte


#endif