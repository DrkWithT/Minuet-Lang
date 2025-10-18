#ifndef MINUET_FAST_VALUE_HPP
#define MINUET_FAST_VALUE_HPP

#include <cstdint>
#include <optional>
#include <string>

namespace Minuet::Runtime {
    class HeapValueBase;

    using HeapValuePtr = HeapValueBase*;
};

namespace Minuet::Runtime {
    enum class FVTag : uint8_t {
        dud,
        boolean,
        int32,
        flt64,
        sequence,
    };

    class FastValue {
    private:
        union {
            uint8_t dud;
            int scalar_v;
            double dbl_v;
            HeapValueBase* obj_p;
        } m_data;
        FVTag m_tag;

    public:
        constexpr FastValue() noexcept
        : m_data {}, m_tag {FVTag::dud} {
            m_data.dud = 0;
        }

        constexpr FastValue(bool b) noexcept
        : m_data {}, m_tag {FVTag::boolean} {
            m_data.scalar_v = static_cast<int>(b);
        }

        constexpr FastValue(int i) noexcept
        : m_data {}, m_tag {FVTag::int32} {
            m_data.scalar_v = i;
        }

        constexpr FastValue(double d) noexcept
        : m_data {}, m_tag {FVTag::flt64} {
            m_data.dbl_v = d;
        }

        constexpr FastValue(HeapValuePtr obj_p) noexcept
        : m_data {}, m_tag {FVTag::sequence} {
            m_data.obj_p = obj_p;
        }

        [[nodiscard]] constexpr auto tag() const& noexcept -> FVTag {
            return m_tag;
        }

        [[nodiscard]] auto to_scalar() noexcept -> std::optional<int>;
        [[nodiscard]] auto to_object_ptr() noexcept -> HeapValuePtr;

        [[nodiscard]] constexpr auto is_none() const& -> bool {
            return m_tag == FVTag::dud;
        }

        [[nodiscard]] auto negate() & -> bool;

        [[nodiscard]] explicit operator bool(this auto&& self) noexcept {
            switch (self.tag()) {
            case FVTag::boolean:
            case FVTag::int32:
                return self.m_data.scalar_v != 0;
            case FVTag::flt64:
                return self.m_data.dbl_v != 0.0;
            default:
                return false;
            }
        }

        [[nodiscard]] auto operator*(const FastValue& arg) & noexcept -> FastValue;
        [[nodiscard]] auto operator/(const FastValue& arg) & -> FastValue;
        [[nodiscard]] auto operator%(const FastValue& arg) & -> FastValue;
        [[nodiscard]] auto operator+(const FastValue& arg) & noexcept -> FastValue;
        [[nodiscard]] auto operator-(const FastValue& arg) & noexcept -> FastValue;

        auto operator*=(const FastValue& arg) & noexcept -> FastValue&;
        auto operator/=(const FastValue& arg) & -> FastValue&;
        auto operator%=(const FastValue& arg) & -> FastValue&;
        auto operator+=(const FastValue& arg) & noexcept -> FastValue&;
        auto operator-=(const FastValue& arg) & noexcept -> FastValue&;

        [[nodiscard]] auto operator==(const FastValue& arg) const& -> bool;
        [[nodiscard]] auto operator<(const FastValue& arg) const& -> bool;
        [[nodiscard]] auto operator>(const FastValue& arg) const& -> bool;
        [[nodiscard]] auto operator<=(const FastValue& arg) const& -> bool;
        [[nodiscard]] auto operator>=(const FastValue& arg) const& -> bool;

        [[nodiscard]] auto to_string() const& -> std::string;
    };
}

#endif
