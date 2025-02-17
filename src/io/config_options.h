#include "options.h"

namespace latte
{
    enum Depth {
        kDepthDefault,  // Traverse nested options that are not flagged as "shallow"
        kDepthShallow,  // Do not traverse into any nested options
        kDepthDetailed, // Traverse nested options, overriding the options shallow
                     // setting
    };

    struct ConfigOptions {
        ConfigOptions();

        explicit ConfigOptions(const DBOptions&);

    };


} // namespace latte


