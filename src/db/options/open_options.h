
#ifndef __LATTE_C_PLUS_ROCKSDB_OPEN_OPTIONS_H
#define __LATTE_C_PLUS_ROCKSDB_OPEN_OPTIONS_H

#include "log/logging.h"

namespace latte
{


    struct OpenOptions {
        // public:
        //     bool create_if_missing;
        //     bool enable_thread_tracking;
        //     // 如果为 true，则每 stats_persist_period_sec 秒自动将统计数据持久化到隐藏列族（列族名称：___rocksdb_stats_history___）；
        //     // 否则，写入内存结构。用户可以通过 `GetStatsHistory` API 进行查询。
        //     // 如果用户尝试在之前已将 persist_stats_to_disk 设置为 true 的 DB 上创建同名的列族，
        //     // 则列族创建将失败，但隐藏的列族以及之前持久化的统计数据将保留下来。
        //     // 将统计数据持久化到磁盘时，统计数据名称将限制为 100 个字节。
        //     bool persist_stats_to_disk = false;
    };

    namespace rocksdb
    {
        struct DBOptions {
            // 使用指定的对象与环境交互，
            // 例如读取/写入文件、安排后台工作等。
            // 默认值：Env::Default()
            Env* env = Env::Default();
            
            // 如果为 true，则在数据库缺失时会创建数据库。
            // 默认值：false
            bool create_if_missing = false;

            // 如果为 true，则缺失的列族将在
            // DB::Open() 上自动创建。
            // 默认值：false
            bool create_missing_column_families = false;

            // 如果为 true，则当数据库已存在时会引发错误。
            // 默认值：false
            bool error_if_exists = false;

            // 如果为 true，则每 stats_persist_period_sec 秒自动将统计数据持久化到隐藏列族（列族名称：___rocksdb_stats_history___）；
            // 否则，写入内存结构。用户可以通过 `GetStatsHistory` API 进行查询。
            // 如果用户尝试在之前已将 persist_stats_to_disk 设置为 true 的 DB 上创建同名的列族，
            // 则列族创建将失败，但隐藏的列族以及之前持久化的统计数据将保留下来。
            // 将统计数据持久化到磁盘时，统计数据名称将限制为 100 个字节。
            bool persist_stats_to_disk = false;  //会创建一个___rocksdb_stats_history___ 列族保存统计数据


            // 数据库生成的任何内部进度/错误信息将
            // 写入 info_log（如果非 nullptr）或存储到文件中
            // 如果 info_log 为 nullptr，则与数据库内容位于同一目录中。
            // 默认值：nullptr
            std::shared_ptr<latte::log::Logger> info_log = nullptr;
        };

        struct OpenOptions: public DBOptions , public ColumnFamilyOptions  {
            public: 
                OpenOptions() : DBOptions(), ColumnFamilyOptions() {}

                OpenOptions(const DBOptions& db_options,
                        const ColumnFamilyOptions& column_family_options)
                    : DBOptions(db_options), ColumnFamilyOptions(column_family_options) {}
            public:
                // 如果为 true，则此 DB 中涉及的线程的状态将
                // 被跟踪并通过 GetThreadList() API 获取。
                // 默认值：false
                bool enable_thread_tracking;
        };


        struct RocksdbOpenOptions: public OpenOptions {
            public:
               
        };

    } // namespace rocksdb
    

    
    
} // namespace latte

#endif