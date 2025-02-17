







namespace latte
{
    enum class OptionType {
        kBoolean,
        kInt,
        kInt32T,
        kInt64T,
        kUInt,
        kUInt8T,
        kUInt32T,
        kUInt64T,
        kSizeT,
        kDouble,
        kAtomicInt,
        kString,
        kCompactionStyle,
        kCompactionPri,
        kCompressionType,
        kCompactionStopStyle,
        kChecksumType,
        kEncodingType,
        kEnv,
        kEnum,
        kStruct,
        kVector,
        kConfigurable,
        kCustomizable,
        kEncodedString,
        kTemperature,
        kArray,
        kStringMap,  // Map of <std::string, std::string>
        kUnknown,
    };
} // namespace latte
