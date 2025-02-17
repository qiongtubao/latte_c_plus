



#ifndef __LATTE_C_PLUS_RECOVERY_CONTEXT_H
#define __LATTE_C_PLUS_RECOVERY_CONTEXT_H

#include <vector>
#include <string>
#include "../shared/autovector.h"
#include "column_family/column_family_mem_tables.h"
namespace latte
{
    namespace rocksdb
    {
        class RecoveryContext {
            public:
                RecoveryContext() {};
                void UpdateVersionEdits(ColumnFamilyData* cfd, const VersionEdit& edit) {
                    assert(cfd != nullptr);
                    if (map_.find(cfd->GetID()) == map_.end()) {
                        uint32_t size = static_cast<uint32_t>(map_.size());
                        map_.emplace(cfd->GetID(), size);
                        cfds_.emplace_back(cfd);
                        mutable_cf_opts_.emplace_back(cfd->GetLatestMutableCFOptions());
                        edit_lists_.emplace_back(autovector<VersionEdit*>());
                    }
                    uint32_t i = map_[cfd->GetID()];
                    edit_lists_[i].emplace_back(new VersionEdit(edit));
                }
                std::unordered_map<uint32_t, uint32_t> map_;  // cf_id to index;
                autovector<ColumnFamilyData*> cfds_;
                autovector<const MutableCFOptions*> mutable_cf_opts_;
                autovector<autovector<VersionEdit*>> edit_lists_;
                // All existing data files (SST files and Blob files) found during DB::Open.
                std::vector<std::string> existing_data_files_;
                bool is_new_db_ = false;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif