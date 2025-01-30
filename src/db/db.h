

#include "status/status.h"
#include "./options/open_options.h"
#include "./options/read_options.h"
namespace latte
{
    class DB {
        public:
            // 打开具有指定“名称”的数据库以进行读写。
            // 将指向堆分配数据库的指针存储在 *dbptr 中，并在成功时返回
            // OK。
            // 将 nullptr 存储在 *dbptr 中，并在出错时返回非 OK 状态，包括
            // 如果 DB 已被另一个 DB 对象打开（读写）。（此
            // 保证取决于 options.env->LockFile()，在自定义 Env 实现中可能无法提供
            // 此保证。）
            //
            // 当不再需要 *dbptr 时，调用者必须将其删除。
            // static Status Open(const OpenOptions& options, const std::string& name, DB** dbptr);


            // static Status Put(const WriteOptions& options, const Slice& key, const Slice& value) = 0;

            virtual Status Get(const ReadOptions& options, const Slice& key, std::string* value) = 0;

            // static Status Delete(const WriteOptions& options, const Slice& key) = 0;

            // batch can Put or delete 
            // Status Write(const WriteOptions& options, WriteBatch* updates) override;


    };
    
} // namespace latte
