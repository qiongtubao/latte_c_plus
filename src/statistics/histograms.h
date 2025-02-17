

#ifndef __LATTE_C_PLUS_HISTOGRAMS_H
#define __LATTE_C_PLUS_HISTOGRAMS_H

#include <string>
#include <mutex>

namespace latte
{
    struct HistogramData {
        double median;
        double percentile95;
        double percentile99;
        double average;
        double standard_deviation;
        // zero-initialize new members since old Statistics::histogramData()
        // implementations won't write them.
        double max = 0.0;
        uint64_t count = 0;
        uint64_t sum = 0;
        double min = 0.0;
    };

    class Histogram {
        public:
            Histogram() {}
            virtual ~Histogram(){}

            virtual void Clear() = 0;
            virtual bool Empty() const = 0;
            virtual void Add(uint64_t value) = 0;
            virtual void Merge(const Histogram&) = 0;

            virtual std::string ToString() const = 0;
            virtual const char* Name() const = 0;
            virtual uint64_t min() const = 0;
            virtual uint64_t max() const = 0;
            virtual uint64_t num() const = 0;
            virtual double Median() const = 0;
            virtual double Percentile(double p) const = 0;
            virtual double Average() const = 0;
            virtual double StandardDeviation() const = 0;
            virtual void Data(HistogramData* const data) const = 0;
    };

    struct HistogramStat {
        HistogramStat();
        ~HistogramStat() {}

        HistogramStat(const HistogramStat&) = delete;
        HistogramStat& operator=(const HistogramStat&) = delete;

        void Clear();
        bool Empty() const;
        void Add(uint64_t value);
        void Merge(const HistogramStat& other);

        inline uint64_t min() const { return min_.load(std::memory_order_relaxed); }
        inline uint64_t max() const { return max_.load(std::memory_order_relaxed); }
        inline uint64_t num() const { return num_.load(std::memory_order_relaxed); }
        inline uint64_t sum() const { return sum_.load(std::memory_order_relaxed); }
        inline uint64_t sum_squares() const {
            return sum_squares_.load(std::memory_order_relaxed);
        }
        inline uint64_t bucket_at(size_t b) const {
            return buckets_[b].load(std::memory_order_relaxed);
        }

        double Median() const;
        double Percentile(double p) const;
        double Average() const;
        double StandardDeviation() const;
        void Data(HistogramData* const data) const;
        std::string ToString() const;

        // To be able to use HistogramStat as thread local variable, it
        // cannot have dynamic allocated member. That's why we're
        // using manually values from BucketMapper
        std::atomic_uint_fast64_t min_;
        std::atomic_uint_fast64_t max_;
        std::atomic_uint_fast64_t num_;
        std::atomic_uint_fast64_t sum_;
        std::atomic_uint_fast64_t sum_squares_;
        std::atomic_uint_fast64_t buckets_[109];  // 109==BucketMapper::BucketCount()
        const uint64_t num_buckets_;
    };


    class HistogramImpl : public Histogram {
        public:
            HistogramImpl() { Clear(); }

            HistogramImpl(const HistogramImpl&) = delete;
            HistogramImpl& operator=(const HistogramImpl&) = delete;

            void Clear() override;
            bool Empty() const override;
            void Add(uint64_t value) override;
            void Merge(const Histogram& other) override;
            void Merge(const HistogramImpl& other);

            std::string ToString() const override;
            const char* Name() const override { return "HistogramImpl"; }
            uint64_t min() const override { return stats_.min(); }
            uint64_t max() const override { return stats_.max(); }
            uint64_t num() const override { return stats_.num(); }
            double Median() const override;
            double Percentile(double p) const override;
            double Average() const override;
            double StandardDeviation() const override;
            void Data(HistogramData* const data) const override;

            virtual ~HistogramImpl() {}

            inline HistogramStat& TEST_GetStats() { return stats_; }

        private:
            HistogramStat stats_;
            std::mutex mutex_;

    };

} // namespace latte



#endif