

#include "env.h"
#include "../monitoring/thread/thread_local.h"

namespace latte {
    // Status WriteStringToFile(Env* env, const Slice& data, const std::string& fname,
    //                      bool should_sync, const IOOptions* io_options) {
    //     const auto& fs = env->GetFileSystem();
    //     return WriteStringToFile(fs.get(), data, fname, should_sync,
    //                             io_options ? *io_options : IOOptions());
    // }

    // Status ReadFileToString(Env* env, const std::string& fname, std::string* data) {
    //     const auto& fs = env->GetFileSystem();
    //     return ReadFileToString(fs.get(), fname, data);
    // }



    Env* Env::Default() {
        // 以下函数调用在静态 default_env 之前初始化 ThreadLocalPtr 的单例。这保证 default_env 将始终在 ThreadLocalPtr 单例被破坏之前被破坏，因为 C++ 保证静态变量的破坏顺序是其构造的反向顺序。
        // 由于静态成员的破坏顺序是其构造的反向顺序，因此在此处进行此调用可保证静态 PosixEnv 的析构函数将首先执行，然后是 ThreadLocalPtr 的单例。
        ThreadLocalPtr::InitSingletons();
        // CompressionContextCache::InitSingleton(); ?????
        // INIT_SYNC_POINT_SINGLETONS(); ???
        // Avoid problems with accessing most members of Env::Default() during
        // static destruction.
        // STATIC_AVOID_DESTRUCTION(PosixEnv, default_env); ????
        // This destructor must be called on exit
        static PosixEnv::JoinThreadsOnExit thread_joiner(default_env);
        return &default_env;
    }
} //namespace latte


