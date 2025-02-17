

#include <vector>
#include "object_library.h"
#include "customizable.h"



namespace latte
{
    using RegistrarFunc = std::function<int(ObjectLibrary&, const std::string&)>;
    template <typename T>
    using ConfigureFunc = std::function<Status(T*)>;

    //注册对象
    class ObjectRegistry {
        public:
            static std::shared_ptr<ObjectRegistry> NewInstance();

        private:
            std::vector<std::shared_ptr<ObjectLibrary>> libraries_;
            std::vector<std::string> plugins_;
            static std::unordered_map<std::string, RegistrarFunc> builtins_;
            std::map<std::string, std::weak_ptr<Customizable>> managed_objects_;
            std::shared_ptr<ObjectRegistry> parent_;
            mutable std::mutex objects_mutex_;
            mutable std::mutex library_mutex_;
    };
} // namespace latte
