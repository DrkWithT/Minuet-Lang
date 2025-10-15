#include <format>
#include "runtime/fast_value.hpp"

namespace Minuet::Runtime {
    auto FastValue::negate() & -> bool {
        switch (tag()) {
        case FVTag::boolean:
            m_data.scalar_v ^= 0b1;
            return true;
        default:
            return false;
        }
    }

    [[nodiscard]] auto FastValue::operator*(const FastValue& arg) & noexcept -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v * arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v * arg.m_data.dbl_v;
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator/(const FastValue& arg) & -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            if (arg) {
                return m_data.scalar_v / arg.m_data.scalar_v;
            }

            return {};
        case FVTag::flt64:
            if (arg) {
                return m_data.dbl_v / arg.m_data.dbl_v;
            }

            return {};
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator%(const FastValue& arg) & -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            if (arg) {
                return m_data.scalar_v % arg.m_data.scalar_v;
            }

            return {};
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator+(const FastValue& arg) & noexcept -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v + arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v + arg.m_data.dbl_v;
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator-(const FastValue& arg) & noexcept -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v - arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v - arg.m_data.dbl_v;
        default:
            return {};
        }
    }

    auto FastValue::operator*=(const FastValue& arg) & noexcept -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            m_data.dud = 0;
            m_tag = FVTag::dud;

            return *this;
        }

        switch (self_tag) {
        case FVTag::int32:
            m_data.scalar_v *= arg.m_data.scalar_v;
            break;
        case FVTag::flt64:
            m_data.dbl_v *= arg.m_data.dbl_v;
            break;
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator/=(const FastValue& arg) & -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            m_data.dud = 0;
            m_tag = FVTag::dud;

            return *this;
        }

        switch (self_tag) {
        case FVTag::int32:
            if (arg) {
                m_data.scalar_v /= arg.m_data.scalar_v;
            } else {
                m_data.dud = 0;
                m_tag = FVTag::dud;
            }
            break;
        case FVTag::flt64:
            if (arg) {
                m_data.dbl_v /= arg.m_data.dbl_v;
            } else {
                m_data.dud = 0;
                m_tag = FVTag::dud;
            }
            break;
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator%=(const FastValue& arg) & -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            m_data.dud = 0;
            m_tag = FVTag::dud;

            return *this;
        }

        switch (self_tag) {
        case FVTag::int32:
            if (arg) {
                m_data.scalar_v /= arg.m_data.scalar_v;
            } else {
                m_data.dud = 0;
                m_tag = FVTag::dud;
            }
            break;
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator+=(const FastValue& arg) & noexcept -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            m_data.dud = 0;
            m_tag = FVTag::dud;

            return *this;
        }

        switch (self_tag) {
        case FVTag::int32:
            m_data.scalar_v += arg.m_data.scalar_v;
            break;
        case FVTag::flt64:
            m_data.dbl_v += arg.m_data.dbl_v;
            break;
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator-=(const FastValue& arg) & noexcept -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            m_data.dud = 0;
            m_tag = FVTag::dud;

            return *this;
        }

        switch (self_tag) {
        case FVTag::int32:
            m_data.scalar_v -= arg.m_data.scalar_v;
            break;
        case FVTag::flt64:
            m_data.dbl_v -= arg.m_data.dbl_v;
            break;
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    [[nodiscard]] auto FastValue::operator==(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return false;
        }

        switch (self_tag) {
        case FVTag::boolean:
            return m_data.scalar_v == arg.m_data.scalar_v;
        case FVTag::int32:
            return m_data.scalar_v == arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v == arg.m_data.dbl_v;
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator<(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v < arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v < arg.m_data.dbl_v;
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator>(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v > arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v > arg.m_data.dbl_v;
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator<=(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v <= arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v <= arg.m_data.dbl_v;
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator>=(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag()) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v >= arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v >= arg.m_data.dbl_v;
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::to_string() const& -> std::string {
        switch (tag()) {
        case FVTag::boolean:
            return std::format("{}", m_data.scalar_v != 0);
        case FVTag::int32:
            return std::format("{}", m_data.scalar_v);
        case FVTag::flt64:
            return std::format("{}", m_data.dbl_v);
        case FVTag::dud:
            return "(dud)";
        }
    }
}
