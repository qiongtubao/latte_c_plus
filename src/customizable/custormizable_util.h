



#include "status/status.h"
#include "options.h"
#include <map>
#include <unordered_map>


namespace latte
{

    template <typename T>
    static Status NewSharedObject(
        const ConfigOptions& config_options, const std::string& id,
        const std::unordered_map<std::string, std::string>& opt_map,
        std::shared_ptr<T>* result) {
        if (!id.empty()) {
            Status status;
            status = config_options.registry->NewSharedObject(id, result);
            if (config_options.ignore_unsupported_options && status.IsNotSupported()) {
                status = Status::OK();
            } else if (status.ok()) {
                status = Customizble::ConfigureNewObject(config_options, result->get(), opt_map);
            }
            return status;
        } else if (opt_map.empty()) {
            result->reset();
            return Status::OK();
        } else {
            return Status::NotSupported("Cannot reset object ");
        }
    }

    template <typename T>
        static Status LoadSharedObject(const ConfigOptions& config_options,
                                    const std::string& value,
                                    std::shared_ptr<T>* result) {
        std::string id;
        std::unordered_map<std::string, std::string> opt_map;

        Status status = Customizable::GetOptionsMap(config_options, result->get(),
                                                value, &id, &opt_map);
        if (!status.ok()) {
            return status;
        } else {
            return NewSharedObject(config_options, id, opt_map, result);
        }
    }   
} // namespace latte


