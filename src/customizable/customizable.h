
#ifndef __LATTE_C_PLUS_CUSTOMIZABLE_H
#define __LATTE_C_PLUS_CUSTOMIZABLE_H


namespace latte
{
    /**
    * 可自定义 rocksdb 使用的基类，描述配置和创建对象的标准方式。可自定义对象是可从 ObjectRegistry 创建的可配置对象。
    *
    * 当 RocksDB 有多个潜在类的实现（例如 Table、Cache、
    * MergeOperator 等）时，使用可自定义类。抽象基类应定义一个方法，声明其类型，并定义一个工厂方法用于创建其中一个，例如：
    * static const char *Type() { return "Table"; }
    * static Status CreateFromString(const ConfigOptions& options,
    * const std::string& id,
    * std::shared_ptr<TableFactory>* result);
    * “Type”字符串应是唯一的（没有两个基类是相同的
    * 类型）。根据选项和 ID，此工厂应创建并
    * 返回可自定义类的适当派生类型（例如
    * BlockBasedTableFactory、PlainTableFactory 等）。对于扩展开发人员，
    * 提供了用于编写此工厂的辅助类和方法。
    *
    * 可自定义类的实例需要定义：
    * - “static const char *kClassName()”方法。此方法定义类实例的名称（例如 BlockBasedTable、LRUCache），并由
    * CheckedCast 方法使用。
    * - 对象的 Name()。此名称用于创建和保存
    * 此类的实例。通常，此名称与
    * kClassName() 相同。
    *
    * 此外，可自定义类应注册用于使用可配置子系统配置自身的任何选项。
    *
    * 创建可自定义类时，“name”属性指定
    * 正在创建的实例的名称。
    * 对于自定义对象，其配置和名称可以通过以下方式指定：
    * [prop]={name=X;option 1 = value1[; option2=value2...]}
    *
    * [prop].name=X
    * [prop].option1 = value1
    *
    * [prop].name=X
    * X.option1 =value1
    */
    class Customizable {
        public:
            // ~Customizable() override {}

            // Returns the name of this class of Customizable
            virtual const char* Name() const = 0;

            // Returns an identifier for this Customizable.
            // This could be its name or something more complex (like its URL/pattern).
            // Used for pretty printing.
            virtual std::string GetId() const {
                std::string id = Name();
                return id;
            }
            virtual const char* NickName() const { return ""; }

            virtual bool IsInstanceOf(const std::string& name) const {
                if (name.empty()) {
                    return false;
                } else if (name == Name()) {
                    return true;
                } else {
                    const char* nickname = NickName();
                    if (nickname != nullptr && name == nickname) {
                        return true;
                    } else {
                        return false;
                    }
                }
            };

        // protected:
            
    };
} // namespace latte


#endif