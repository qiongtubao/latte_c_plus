


#ifndef __LATTE_C_PLUS_TABLE_PROPERTIES_COLLECTOR_FACTORY_H
#define __LATTE_C_PLUS_TABLE_PROPERTIES_COLLECTOR_FACTORY_H


#include "customizable/customizable.h"
#include <cstdint>
#include "../types.h"
#include "status/status.h"
#include "io/config_options.h"



namespace latte
{
    namespace rocksdb
    {
        class TablePropertiesCollectorFactory: public Customizable {
            public:
                struct Context {
                    uint32_t column_family_id;
                    // The level at creating the SST file (i.e, table), of which the
                    // properties are being collected.
                    int level_at_creation = kUnknownLevelAtCreation;
                    int num_levels = kUnknownNumLevels;
                    // In the tiering case, data with seqnos smaller than or equal to this
                    // cutoff sequence number will be considered by a compaction job as eligible
                    // to be placed on the last level. When this is the maximum sequence number,
                    // it indicates tiering is disabled.
                    SequenceNumber last_level_inclusive_max_seqno_threshold;
                    static const uint32_t kUnknownColumnFamily;
                    static const int kUnknownLevelAtCreation = -1;
                    static const int kUnknownNumLevels = -1;

                    Context() {}

                    Context(uint32_t _column_family_id, int _level_at_creation, int _num_levels,
                            SequenceNumber _last_level_inclusive_max_seqno_threshold)
                        : column_family_id(_column_family_id),
                        level_at_creation(_level_at_creation),
                        num_levels(_num_levels),
                        last_level_inclusive_max_seqno_threshold(
                            _last_level_inclusive_max_seqno_threshold) {}
                };

                ~TablePropertiesCollectorFactory() override {}
                static const char* Type() { return "TablePropertiesCollectorFactory"; }
                static Status CreateFromString(
                    const ConfigOptions& options, const std::string& value,
                    std::shared_ptr<TablePropertiesCollectorFactory>* result);

                // To collect properties of a table with the given context, returns
                // a new object inheriting from TablePropertiesCollector. The caller
                // is responsible for deleting the object returned. Alternatively,
                // nullptr may be returned to decline collecting properties for the
                // file (and reduce callback overheads).
                // MUST be thread-safe.
                virtual TablePropertiesCollector* CreateTablePropertiesCollector(
                    TablePropertiesCollectorFactory::Context context) = 0;

                // The name of the properties collector can be used for debugging purpose.
                const char* Name() const override = 0;

                // Can be overridden by sub-classes to return the Name, followed by
                // configuration info that will // be logged to the info log when the
                // DB is opened
                virtual std::string ToString() const { return Name(); }
        };
    } // namespace rocksdb
    
} // namespace latte


#endif