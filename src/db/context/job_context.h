


#ifndef __LATTE_C_PLUS_JOB_CONTEXT_H
#define __LATTE_C_PLUS_JOB_CONTEXT_H


#include <memory>
#include "../version/super_version.h"
#include "../listener/write_stall_info.h"

namespace latte
{
    namespace rocksdb
    {
        struct SuperVersionContext {
            public:
                struct WriteStallNotification {
                    WriteStallInfo write_stall_info;
                    const ImmutableOptions* immutable_options;
                };
                explicit SuperVersionContext(bool create_superversion = false)
                    : new_superversion(create_superversion ? new SuperVersion() : nullptr) {}

                void Clean() {
                    #if !defined(ROCKSDB_DISABLE_STALL_NOTIFICATION)
                        // notify listeners on changed write stall conditions
                        for (auto& notif : write_stall_notifications) {
                            for (auto& listener : notif.immutable_options->listeners) {
                                listener->OnStallConditionsChanged(notif.write_stall_info);
                            }
                        }
                        write_stall_notifications.clear();
                    #endif
                    // free superversions
                    for (auto s : superversions_to_free) {
                        delete s;
                    }
                    superversions_to_free.clear();
                }
            public:
                std::unique_ptr<SuperVersion>
                    new_superversion;  // if nullptr no new superversion
                #ifndef ROCKSDB_DISABLE_STALL_NOTIFICATION
                    autovector<WriteStallNotification> write_stall_notifications;
                #endif

                autovector<SuperVersion*> superversions_to_free;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif