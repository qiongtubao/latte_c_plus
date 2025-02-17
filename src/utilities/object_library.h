
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <vector>


namespace latte
{
    template<typename T>
    using FactoryFunc = 
        std::function<T*(const std::string&, std::unique_ptr<T>*, std::string*)>;

    class ObjectLibrary {
        private:
            class Entry {
                public:
                    virtual ~Entry() {}
                    virtual bool Matches(const std::string& target) const = 0;
                    virtual const char* Name() const = 0;
            };

            class PatternEntry : public Entry {
                private:
                    enum Quantifier {
                        kMatchZeroOrMore,  // [suffix].*
                        kMatchAtLeastOne,  // [suffix].+
                        kMatchExact,       // [suffix]
                        kMatchInteger,     // [suffix][0-9]+
                        kMatchDecimal,     // [suffix][0-9]+[.][0-9]+
                    };
                private:
                    std::string name_;
                    size_t nlength_;
                    std::vector<std::string> names_;
                    bool optional_;
                    size_t slength_;
                    std::vector<std::pair<std::string, Quantifier>> separators;
            };
        public:
            explicit ObjectLibrary(const std::string& id) { id_ = id; }


            // 将工厂注册到条目的库中。
            // 如果条目与目标匹配，则可以使用工厂创建新的
            // 对象。
            // @see PatternEntry 了解匹配规则。
            // 注意：此函数取代了旧的 ObjectLibrary::Register()
            template <typename T>  //添加工厂 , 名字和方法
            const FactoryFunc<T>& AddFactory(const std::string& name, 
                const FactoryFunc<T>& func) {
                std::unique_ptr<Entry> entry(
                    new FactoryEntry<T>(new PatternEntry(name), func)  
                );
                AddFactoryEntry(T::Type(), std::move(entry));
                return func;
            }
            // 返回为此库注册的工厂总数。
            // 此方法返回为所有类型注册的所有工厂的总和。
            // @param num_types 返回注册的唯一类型数。
            size_t GetFactoryCount(size_t* num_types) const;

            // 返回为此库注册的工厂数量
            // 用于输入类型。
            // @param num_types 返回已注册的唯一类型数量。
            size_t GetFactoryCount(const std::string &type) const;

            // 使用名称向库注册工厂。
            // 如果 name==target，则可以使用工厂来创建新对象。
            template <typename T>
            const FactoryFunc<T>& AddFactory(const std::string& name,
                                            const FactoryFunc<T>& func) {
                std::unique_ptr<Entry> entry(
                    new FactoryEntry<T>(new PatternEntry(name), func)
                );
                AddFactoryEntry(T::Type(), std::move(entry));
                return func;
            }
            
            // 将工厂注册到条目的库中。
            // 如果条目与目标匹配，则可以使用工厂创建新的
            // 对象。
            // @see PatternEntry 了解匹配规则。
            // 注意：此函数取代了旧的 ObjectLibrary::Register()
            template <typename T>
            const FactoryFunc<T>& AddFactory(const PatternEntry& entry,
                                            const FactoryFunc<T>& func) {
                std::unique_ptr<Entry> factory(
                    new FactoryEntry<T>(new PatternEntry(entry), func));
                AddFactoryEntry(T::Type(), std::move(factory));
                return func;
            }

        private:
            void AddFactoryEntry(const char* type, std::unique_ptr<Entry>&& entry) {
                std::unique_lock<std::mutex> lock(mu_);
                auto& factories = factories_[type];
                factories.emplace_back(std::move(entry));
            }
            // 保护入口地图
            mutable std::mutex mu_;
            // ** 此加载器的 FactoryFunctions，按类型组织
            std::unordered_map<std::string, std::vector<std::unique_ptr<Entry>>> factories_;
            // 该库的名称
            std::string id_;

    };
} // namespace latte
