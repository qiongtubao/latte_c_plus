


#ifndef __LATTE_C_PLUS_COLUMN_FAMILY_MEM_TABLES_H
#define __LATTE_C_PLUS_COLUMN_FAMILY_MEM_TABLES_H


#include <iterator>
#include "../options/mutable_cf_options.h"
#include "../options/immutable_db_options.h"
#include "../options/immutable_options.h"
#include "../options/cf_options.h"
#include "../internal_stats.h"
#include "../memtable/mem_table.h"
#include "../memtable/mem_table_list.h"


namespace latte
{
    template <typename K, typename V>
    using UnorderedMap = std::unordered_map<K, V>;

    namespace rocksdb
    {
        // ColumnFamilySet 具有有趣的线程安全要求
        // * CreateColumnFamily() 或 RemoveColumnFamily() - 需要受 DB
        // 互斥锁保护并在写入线程中执行。
        // CreateColumnFamily() 应仅从 VersionSet::LogAndApply() 和
        // 单线程写入线程调用。它也在 Recovery 期间和
        // DumpManifest() 中调用。
        // RemoveColumnFamily() 仅从 SetDropped() 调用。需要
        // 保留 DB 互斥锁，并且需要从写入线程执行。SetDropped() 还
        // 保证它将仅从单线程 LogAndApply() 调用，
        // 但这个条件并不那么重要。
        // * 迭代 - 保留 DB 互斥锁。如果您想在迭代主体中释放 DB 互斥锁，请包装在 RefedColumnFamilySet 中。
        // * GetDefault() -- 线程安全
        // * GetColumnFamily() -- 位于 DB 互斥锁内或来自写入线程
        // * GetNextColumnFamilyID()、GetMaxColumnFamily()、UpdateMaxColumnFamily()、
        // NumberOfColumnFamilies -- 位于 DB 互斥锁内
        class ColumnFamilySet {
            public:
                // ColumnFamilySet(const std::string& dbname,
                //     const ImmutableDBOptions* db_options,
                //     const FileOptions& file_options, Cache* table_cache,
                //     WriteBufferManager* _write_buffer_manager,
                //     WriteController* _write_controller,
                //     BlockCacheTracer* const block_cache_tracer,
                //     const std::shared_ptr<IOTracer>& io_tracer,
                //     const std::string& db_id, const std::string& db_session_id);
                // ColumnFamilySet supports iteration
                class iterator {
                    public:
                        explicit iterator(ColumnFamilyData* cfd) : current_(cfd) {}
                        // NOTE: minimum operators for for-loop iteration
                        iterator& operator++() {
                            current_ = current_->next_;
                            return *this;
                        }
                        bool operator!=(const iterator& other) const {
                            return this->current_ != other.current_;
                        }
                        ColumnFamilyData* operator*() { return current_; }

                    private:
                        ColumnFamilyData* current_;
                };
                iterator begin() { return iterator(dummy_cfd_->next_); }
                iterator end() { return iterator(dummy_cfd_); }
                ColumnFamilyData* GetDefault() const;
                // GetColumnFamily() calls return nullptr if column family is not found
                ColumnFamilyData* GetColumnFamily(uint32_t id) const;
                ColumnFamilyData* GetColumnFamily(const std::string& name) const;
                
                ~ColumnFamilySet();

            public:
                ColumnFamilyData* dummy_cfd_;
                // column_families_ 和 column_family_data_ 需要受到保护：
                // * 改变时必须满足两个条件：
                // 1. DB 互斥锁已锁定
                // 2. 线程当前处于单线程写入线程中
                // * 读取时，至少需要满足一个条件：
                // 1. DB 互斥锁已锁定
                // 2. 从单线程写入线程访问
                UnorderedMap<std::string, uint32_t> column_families_;
                UnorderedMap<uint32_t, ColumnFamilyData*> column_family_data_;
                // 变异/读取 `running_ts_sz_` 和 `ts_sz_for_record_` 遵循
                // 与 `column_families_` 和 `column_family_data_` 相同的要求。
                // 将列族 ID 映射到所有
                // 正在运行的列族的用户定义时间戳大小。
                UnorderedMap<uint32_t, size_t> running_ts_sz_;
                // 将列族 ID 映射到用户定义的时间戳大小
                // 具有非零用户定义时间戳大小的列族。
                UnorderedMap<uint32_t, size_t> ts_sz_for_record_;

                uint32_t max_column_family_;
                const FileOptions file_options_;

                ColumnFamilyData* dummy_cfd_;
                // 我们不在这里保存引用计数，因为默认列族始终存在
                // 我们也不负责清理 default_cfd_cache_。这只是一个缓存，使常见情况（访问默认列族）
                // 更快
                ColumnFamilyData* default_cfd_cache_;

                const std::string db_name_;
                const ImmutableDBOptions* const db_options_;
                // Cache* table_cache_;
                // WriteBufferManager* write_buffer_manager_;
                // WriteController* write_controller_;
                // BlockCacheTracer* const block_cache_tracer_;
                std::shared_ptr<IOTracer> io_tracer_;
                const std::string& db_id_;
                std::string db_session_id_;
        };

        class ColumnFamilyMemTables {

        };
        
        class ColumnFamilyData {
            public:
                // thread-safe
                uint32_t GetID() const { return id_; }
                // thread-safe
                const std::string& GetName() const { return name_; }

                // 验证 CF 选项与 DB 选项
                static Status ValidateOptions(const DBOptions& db_options,
                                                const ColumnFamilyOptions& cf_options);

                // 要求：DB 互斥锁已持有
                // 这将返回最新的 MutableCFOptions，但可能尚未生效。
                const MutableCFOptions* GetLatestMutableCFOptions() const {
                    return &mutable_cf_options_;
                }

                Version* current() { return current_; }
                const ImmutableOptions* ioptions() const { return &ioptions_; }

                // created_dirs remembers directory created, so that we don't need to call
                // the same data creation operation again.
                Status AddDirectories(
                    std::map<std::string, std::shared_ptr<FSDirectory>>* created_dirs);

                InternalStats* internal_stats() { return internal_stats_.get(); }

                void CreateNewMemtable(const MutableCFOptions& mutable_cf_options,
                        SequenceNumber earliest_seq);

                MemTable* mem() { return mem_; }
            public:
                const ImmutableOptions ioptions_;
                MutableCFOptions mutable_cf_options_;
                std::unique_ptr<InternalStats> internal_stats_;

                MemTable* mem_;
                MemTableList imm_;
                SuperVersion* super_version_;
                // 循环链表的指针。我们使用它来支持对所有处于活动状态的列族进行迭代
                // （注意：被删除的列族也可以
                // 只要客户端持有引用，它们就处于活动状态）
                ColumnFamilyData* next_;
                ColumnFamilyData* prev_;



                uint32_t id_;
                const std::string name_;
                Version* dummy_versions_;  // Head of circular doubly-linked list of versions.
                Version* current_;         // == dummy_versions->prev_

                std::atomic<int> refs_;  // outstanding references to ColumnFamilyData
                std::atomic<bool> initialized_;
                std::atomic<bool> dropped_;  // true if client dropped it
                
        };

        class ColumnFamilyHandleInternal {

        };

        class ColumnFamilyMemTablesImpl : public ColumnFamilyMemTables {
            public:
                explicit ColumnFamilyMemTablesImpl(ColumnFamilySet* column_family_set)
                : column_family_set_(column_family_set), current_(nullptr) {}
            private:
                ColumnFamilySet* column_family_set_;
                ColumnFamilyData* current_;
                ColumnFamilyHandleInternal handle_;
        };
    } // namespace rocksdb
    
} // namespace latte



#endif