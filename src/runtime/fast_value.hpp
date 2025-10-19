#ifndef MINUET_FAST_VALUE_HPP
#define MINUET_FAST_VALUE_HPP

#include <cstdint>
#include <optional>
#include <string>

namespace Minuet::Runtime {
    /// NOTE: forward declaration of FastValue for HeapValueBase declaration
    class FastValue;

    enum class SequenceOpPolicy : int8_t {
        front,
        back,
    };

    enum class ObjectTag : uint8_t {
        dud,
        sequence,
    };

    class HeapValueBase {
    public:
        virtual ~HeapValueBase() = default;

        virtual auto get_memory_score() const& noexcept -> std::size_t = 0;
        virtual auto get_tag() const& noexcept -> ObjectTag = 0;

        // virtual auto size() const& noexcept -> int = 0;
        virtual auto push_value(FastValue arg) -> bool = 0;
        virtual auto pop_value(SequenceOpPolicy mode) -> FastValue = 0;
        virtual auto set_value(FastValue arg, std::size_t pos) -> bool = 0;
        virtual auto get_value(std::size_t pos) -> std::optional<FastValue*> = 0;

        virtual void freeze() noexcept = 0;

        virtual auto as_fast_value() noexcept -> FastValue = 0;
        virtual auto to_string() const& noexcept -> std::string = 0;
    };

    /// NOTE: Convenience alias of a type-erased pointer to `HeapValueBase`.
    using HeapValuePtr = HeapValueBase*;


    enum class FVTag : uint8_t {
        dud,
        boolean,
        int32,
        flt64,
        val_ref,
        sequence,
    };

    class FastValue {
    private:
        union {
            uint8_t dud;
            int scalar_v;
            double dbl_v;
            FastValue* fv_p;
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

        constexpr FastValue(FastValue* ref_p) noexcept
        : m_data {}, m_tag {FVTag::val_ref} {
            m_data.fv_p = ref_p;
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

        [[nodiscard]] auto emplace_other(const FastValue& arg) & noexcept -> bool;

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
