


#include <cctype>

namespace latte
{

    std::shared_ptr<ObjectRegistry> ObjectRegistry::NewInstance() {
        return std::make_shared<ObjectRegistry>(Default());
    }    

    std::shared_ptr<ObjectRegistry> ObjectRegistry::NewInstance(
        const std::shared_ptr<ObjectRegistry> &parent
    ) {
        return std::make_shared<ObjectRegistry>(parent);
    }
} // namespace latte
