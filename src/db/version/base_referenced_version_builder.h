


#ifndef __LATTE_C_PLUS_BASE_REFERENCED_VERSION_BUILDER_H
#define __LATTE_C_PLUS_BASE_REFERENCED_VERSION_BUILDER_H

#include "column_family/column_family_mem_tables.h"
#include "./version_edit_handler.h"
#include "./version_builder.h"

namespace latte
{
    namespace rocksdb
    {
        // A wrapper of version builder which references the current version in
        // constructor and unref it in the destructor.
        // Both of the constructor and destructor need to be called inside DB Mutex.
        class BaseReferencedVersionBuilder {
            public:
                explicit BaseReferencedVersionBuilder(
                    ColumnFamilyData* cfd, VersionEditHandler* version_edit_handler = nullptr,
                    bool track_found_and_missing_files = false,
                    bool allow_incomplete_valid_version = false);
                BaseReferencedVersionBuilder(
                    ColumnFamilyData* cfd, Version* v,
                    VersionEditHandler* version_edit_handler = nullptr,
                    bool track_found_and_missing_files = false,
                    bool allow_incomplete_valid_version = false);
                ~BaseReferencedVersionBuilder();
                VersionBuilder* version_builder() const { return version_builder_.get(); }

            private:
                std::unique_ptr<VersionBuilder> version_builder_;
                Version* version_;
        };
    } // namespace rocksdb
    
} // namespace latte


#endif