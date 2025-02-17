



#ifndef __LATTE_C_PLUS_SUPER_VERSION_H
#define __LATTE_C_PLUS_SUPER_VERSION_H

#include "column_family/column_family_mem_tables.h"
#include "memtable/read_only_mem_table.h"
#include "memtable/mem_table_list_version.h"
#include "version/version.h"
#include "../types.h"
#include "seqno_to_time_mapping.h"
#include "../shared/unowned_ptr.h"

namespace latte
{
    namespace rocksdb
    {
        // 保存对 memtable、所有不可变 memtable 和版本的引用
        struct SuperVersion {
            // Accessing members of this class is not thread-safe and requires external
            // synchronization (ie db mutex held or on write thread).
            ColumnFamilyData* cfd;
            ReadOnlyMemTable* mem;
            MemTableListVersion* imm;
            Version* current;
            MutableCFOptions mutable_cf_options;
            // Version number of the current SuperVersion
            uint64_t version_number;
            WriteStallCondition write_stall_condition;
            // 每次 `full_history_ts_low` 折叠历史记录时，都会安装一个新的 SuperVersion。
            // 此字段跟踪该 SuperVersion 的有效 `full_history_ts_low`，
            // 供读取 API 用于健全性检查。安装 SuperVersion 后，此字段将不可变。
            // 对于未启用 UDT 功能的列系列，这是一个空字符串。
            std::string full_history_ts_low;

            // DB 的 seqno 到时间映射的共享副本。
            std::shared_ptr<const SeqnoToTimeMapping> seqno_to_time_mapping{nullptr};

            // 应该在互斥锁之外调用
            SuperVersion() = default;
            ~SuperVersion();
            SuperVersion* Ref();
            // 如果 Unref() 返回 true，则应在删除此 
            // SuperVersion 之前调用 Cleanup() 并保持互斥量。
            bool Unref();

            // 在持有 db mutex 的情况下调用这两个方法
            // 清理取消引用 mem、imm 和 current。此外，它存储所有需要删除的 memtables
            // 在 to_delete 向量中。取消引用这些
            // 对象需要在 mutex 中完成
            void Cleanup();
            void Init(
                ColumnFamilyData* new_cfd, MemTable* new_mem,
                MemTableListVersion* new_imm, Version* new_current,
                std::shared_ptr<const SeqnoToTimeMapping> new_seqno_to_time_mapping);

            // 共享此处引用的 seqno 到时间映射对象的所有权
            // 超级版本。供在此之后安装的新超级版本使用
            // 一个，如果 seqno 到时间映射在这两个
            // 超级版本之间没有改变。或者与 FlushJob 共享映射的所有权。
            std::shared_ptr<const SeqnoToTimeMapping> ShareSeqnoToTimeMapping() {
                return seqno_to_time_mapping;
            }

            // Access the seqno to time mapping object in this SuperVersion.
            UnownedPtr<const SeqnoToTimeMapping> GetSeqnoToTimeMapping() const {
                return seqno_to_time_mapping.get();
            }

            // dummy 的值实际上并未使用。kSVInUse 将其地址作为线程本地存储中的标记，以指示 SuperVersion 正在被线程使用。
            // 这样，kSVInUse 的值就保证不会与 SuperVersion 对象地址冲突，
            // 并且可以在不同平台上移植。
            static int dummy;
            static void* const kSVInUse;
            static void* const kSVObsolete;

            private:
                std::atomic<uint32_t> refs;
                // 我们需要 to_delete，因为在 Cleanup() 期间，imm->Unref() 返回
                // 我们需要通过此向量释放的所有内存表。然后，我们在销毁期间
                // 删除互斥锁之外的所有内存表
                autovector<ReadOnlyMemTable*> to_delete;
            };
    } // namespace rocksdb
    
} // namespace latte

#endif