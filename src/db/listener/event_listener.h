




#ifndef __LATTE_C_PLUS_DB_EVENT_LISTENER_H
#define __LATTE_C_PLUS_DB_EVENT_LISTENER_H

#include "write_stall_info.h"

namespace latte
{
    namespace rocksdb
    {
        class EventListener {
            public:
                // If true, the OnFile*Finish functions will be called. If
                // false, then they won't be called.
                virtual bool ShouldBeNotifiedOnFileIO() { return false; }

                // A callback function for RocksDB which will be called whenever a change
                // of superversion triggers a change of the stall conditions.
                //
                // Note that the this function must be implemented in a way such that
                // it should not run for an extended period of time before the function
                // returns.  Otherwise, RocksDB may be blocked.
                virtual void OnStallConditionsChanged(const WriteStallInfo& /*info*/) {}
        };
    } // namespace rocksdb
    
    
} // namespace latte


#endif