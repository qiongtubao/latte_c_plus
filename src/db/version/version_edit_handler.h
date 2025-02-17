



#ifndef __LATTE_C_PLUS_VERSION_EDIT_HANDLER_H
#define __LATTE_C_PLUS_VERSION_EDIT_HANDLER_H

#include <vector>
#include "column_family/column_family_descriptor.h"
#include "io/io_tracer.h"
#include "version_set.h"
#include "version_edit.h"
#include "version_edit_handler.h"
#include "./base_referenced_version_builder.h"

namespace latte
{
    namespace rocksdb {

        using VersionBuilderUPtr = std::unique_ptr<BaseReferencedVersionBuilder>;

        // VersionEdit is always supposed to be valid and it is used to point at
        // entries in Manifest. Ideally it should not be used as a container to
        // carry around few of its fields as function params because it can cause
        // readers to think it's a valid entry from Manifest. To avoid that confusion
        // introducing VersionEditParams to simply carry around multiple VersionEdit
        // params. It need not point to a valid record in Manifest.
        using VersionEditParams = VersionEdit;
        class VersionEditHandler {
            public:
                explicit VersionEditHandler(
                    bool read_only,
                    const std::vector<ColumnFamilyDescriptor>& column_families,
                    VersionSet* version_set, bool track_found_and_missing_files,
                    bool no_error_if_files_missing,
                    const std::shared_ptr<IOTracer>& io_tracer,
                    const ReadOptions& read_options, bool allow_incomplete_valid_version,
                    EpochNumberRequirement epoch_number_requirement =
                        EpochNumberRequirement::kMustPresent)
                    : VersionEditHandler(read_only, column_families, version_set,
                                        track_found_and_missing_files,
                                        no_error_if_files_missing, io_tracer, read_options,
                                        /*skip_load_table_files=*/false,
                                        allow_incomplete_valid_version,
                                        epoch_number_requirement) {}

            protected:
                explicit VersionEditHandler(
                    bool read_only, std::vector<ColumnFamilyDescriptor> column_families,
                    VersionSet* version_set, bool track_found_and_missing_files,
                    bool no_error_if_files_missing,
                    const std::shared_ptr<IOTracer>& io_tracer,
                    const ReadOptions& read_options, bool skip_load_table_files,
                    bool allow_incomplete_valid_version,
                    EpochNumberRequirement epoch_number_requirement =
                        EpochNumberRequirement::kMustPresent);
                

            public: //properties
                const bool read_only_;
                std::vector<ColumnFamilyDescriptor> column_families_;
                VersionSet* version_set_;
                std::unordered_map<uint32_t, VersionBuilderUPtr> builders_;
                std::unordered_map<std::string, ColumnFamilyOptions> name_to_options_;
                const bool track_found_and_missing_files_;
                // Keeps track of column families in manifest that were not found in
                // column families parameters. Namely, the user asks to not open these column
                // families. In non read only mode, if those column families are not dropped
                // by subsequent manifest records, Recover() will return failure status.
                std::unordered_map<uint32_t, std::string> do_not_open_column_families_;
                VersionEditParams version_edit_params_;
                bool no_error_if_files_missing_;
                std::shared_ptr<IOTracer> io_tracer_;
                bool skip_load_table_files_;
                bool initialized_;
                std::unique_ptr<std::unordered_map<uint32_t, std::string>> cf_to_cmp_names_;
                // If false, only a complete Version for which all files consisting it can be
                // found is considered a valid Version. If true, besides complete Version, an
                // incomplete Version with only a suffix of L0 files missing is also
                // considered valid if the Version is never edited in an atomic group.
                const bool allow_incomplete_valid_version_;
                EpochNumberRequirement epoch_number_requirement_;
                std::unordered_set<uint32_t> cfds_to_mark_no_udt_;
        };
    }; //namspace rocksdb

   
} // namespace latte


#endif