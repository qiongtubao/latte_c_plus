

#ifndef __LATTE_C_PLUS_COMPRESSION_TYPE_H
#define __LATTE_C_PLUS_COMPRESSION_TYPE_H


namespace latte
{
    // DB 内容存储在一组块中，每个块都包含一个
    // 键值对序列。每个块在存储到文件之前可能会被压缩。以下枚举描述了使用哪种
    // 压缩方法（如果有）来压缩块。

    enum CompressionType : unsigned char {
        // 注意：不要更改现有条目的值，因为它们是
        // 磁盘上持久格式的一部分。
        kNoCompression = 0x0,
        kSnappyCompression = 0x1,
        kZlibCompression = 0x2,
        kBZip2Compression = 0x3,
        kLZ4Compression = 0x4,
        kLZ4HCCompression = 0x5,
        kXpressCompression = 0x6,
        kZSTD = 0x7,

        // 仅当您必须使用早于 0.8.0 的 ZSTD lib 
        //或考虑降级服务或将数据库文件复制到运行没有 kZSTD 的旧版 
        //RocksDB 的另一个服务时才使用 kZSTDNotFinalCompression。
        //否则，您应该使用 kZSTD。我们最终会从公共 API 中删除该选项。
        kZSTDNotFinalCompression = 0x40,

        // kDisableCompressionOption 用于禁用一些压缩选项。
        kDisableCompressionOption = 0xff,
    };
    namespace rocksdb
    {
        
    } // namespace rocksdb
    
} // namespace latte



#endif