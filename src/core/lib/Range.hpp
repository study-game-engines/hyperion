#ifndef HYPERION_RANGE_H
#define HYPERION_RANGE_H

#include <math/MathUtil.hpp>

#include <Types.hpp>

#include <atomic>

namespace hyperion {

template <class T>
class Range {
public:
    using Iterator = T;

    Range() : m_start{}, m_end{} {}
    Range(const T &start, const T &end) : m_start(start), m_end(end) {}
    Range(T &&start, T &&end) : m_start(std::move(start)), m_end(std::move(end)) {}
    Range(const Range &other) = default;
    Range &operator=(const Range &other) = default;
    Range(Range &&other) = default;
    Range &operator=(Range &&other) = default;
    ~Range() = default;

    explicit operator bool() const { return Distance() > 0; }

    const T &GetStart() const { return m_start; }
    void SetStart(const T &start) { m_start = start; }
    const T &GetEnd() const { return m_end; }
    void SetEnd(const T &end) { m_end = end; }

    Int64 Distance() const { return static_cast<Int64>(m_end) - static_cast<Int64>(m_start); }
    Int64 Step() const { return MathUtil::Sign(Distance()); }
    bool Includes(const T &value) const { return value >= m_start && value < m_end; }

    Range operator|(const Range &other) const
    {
        return {
            MathUtil::Min(m_start, other.m_start),
            MathUtil::Max(m_end, other.m_end)
        };
    }

    Range &operator|=(const Range &other)
    {
        m_start = MathUtil::Min(m_start, other.m_start);
        m_end = MathUtil::Max(m_end, other.m_end);

        return *this;
    }

    Range operator&(const Range &other) const
    {
        return {
            MathUtil::Max(m_start, other.m_start),
            MathUtil::Min(m_end, other.m_end)
        };
    }

    Range &operator&=(const Range &other)
    {
        m_start = MathUtil::Max(m_start, other.m_start);
        m_end = MathUtil::Min(m_end, other.m_end);

        return *this;
    }

    bool operator<(const Range &other) const { return Distance() < other.Distance(); }
    bool operator>(const Range &other) const { return Distance() > other.Distance(); }

    bool operator==(const Range &other) const
    {
        if (this == &other) {
            return true;
        }

        return m_start == other.m_start
            && m_end == other.m_end;
    }

    bool operator!=(const Range &other) const { return !operator==(other); }

    void Reset()
    {
        m_start = MathUtil::MaxSafeValue<T>();
        m_end = MathUtil::MinSafeValue<T>();
    }

    bool Valid() const
        { return m_start != MathUtil::MaxSafeValue<T>() || m_end != MathUtil::MinSafeValue<T>(); }
    
    Iterator begin() const { return m_start; }
    Iterator end() const { return m_end; }

private:
    T m_start, m_end;
};

} // namespace hyperion

#endif
