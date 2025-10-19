#include <format>
#include "runtime/fast_value.hpp"

namespace Minuet::Runtime {
    auto FastValue::to_scalar() noexcept -> std::optional<int> {
        if (m_tag == FVTag::int32) {
            return m_data.scalar_v;
        }

        return {};
    }

    auto FastValue::to_object_ptr() noexcept -> HeapValuePtr {
        return (m_tag == FVTag::sequence)
            ? m_data.obj_p
            : nullptr;
    }

    auto FastValue::negate() & -> bool {
        switch (tag()) {
        case FVTag::boolean:
            m_data.scalar_v ^= 0b1;
            return true;
        case FVTag::val_ref:
            return m_data.fv_p->negate();
        default:
            return false;
        }
    }

    auto FastValue::emplace_other(const FastValue& arg) & noexcept -> bool {        
        const auto self_tag = tag();
        const auto arg_tag = arg.tag();

        if (self_tag == FVTag::val_ref && arg_tag == FVTag::val_ref) {
            m_data.fv_p = arg.m_data.fv_p;
            return true;
        } else if (arg_tag != FVTag::val_ref) {
            if (m_data.fv_p != nullptr) {
                return m_data.fv_p->emplace_other(arg);
            }
            return false;
        }

        switch (arg_tag) {
        case FVTag::dud:
            m_data.dud = 0;
            break;
        case FVTag::boolean:
        case FVTag::int32:
            m_data.scalar_v = arg.m_data.scalar_v;
            break;
        case FVTag::flt64:
            m_data.dbl_v = arg.m_data.dbl_v;
            break;
        case FVTag::val_ref:
            if (!emplace_other(*arg.m_data.fv_p)) {
                return false;
            }
            break;
        case FVTag::sequence:
        default:
            m_data.obj_p = arg.m_data.obj_p;
            break;
        }

        m_tag = arg_tag;
        return true;
    }

    [[nodiscard]] auto FastValue::operator*(const FastValue& arg) & noexcept -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v * arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v * arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator*(arg);
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator/(const FastValue& arg) & -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
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
        case FVTag::val_ref:
            return m_data.fv_p->operator/(arg);
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator%(const FastValue& arg) & -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            if (arg) {
                return m_data.scalar_v % arg.m_data.scalar_v;
            }

            return {};
        case FVTag::val_ref:
            return m_data.fv_p->operator%(arg);
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator+(const FastValue& arg) & noexcept -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v + arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v + arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator+(arg);
        default:
            return {};
        }
    }

    [[nodiscard]] auto FastValue::operator-(const FastValue& arg) & noexcept -> FastValue {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return {};
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v - arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v - arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator-(arg);
        default:
            return {};
        }
    }

    auto FastValue::operator*=(const FastValue& arg) & noexcept -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
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
        case FVTag::val_ref:
            return m_data.fv_p->operator*=(arg);
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator/=(const FastValue& arg) & -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
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
        case FVTag::val_ref:
            return m_data.fv_p->operator/=(arg);
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator%=(const FastValue& arg) & -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
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
        case FVTag::val_ref:
            return m_data.fv_p->operator%=(arg);
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator+=(const FastValue& arg) & noexcept -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
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
        case FVTag::val_ref:
            return m_data.fv_p->operator+=(arg);
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    auto FastValue::operator-=(const FastValue& arg) & noexcept -> FastValue& {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
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
        case FVTag::val_ref:
            return m_data.fv_p->operator-=(arg);
        default:
            m_data.dud = 0;
            m_tag = FVTag::dud;
            break;
        }

        return *this;
    }

    [[nodiscard]] auto FastValue::operator==(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return false;
        }

        switch (self_tag) {
        case FVTag::boolean:
            return m_data.scalar_v == arg.m_data.scalar_v;
        case FVTag::int32:
            return m_data.scalar_v == arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v == arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator==(arg);
        // case FVTag::sequence:
        // break;
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator<(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v < arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v < arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator<(arg);
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator>(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v > arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v > arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator>(arg);
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator<=(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v <= arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v <= arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator<=(arg);
        default:
            break;
        }

        return false;
    }

    [[nodiscard]] auto FastValue::operator>=(const FastValue& arg) const& -> bool {
        const auto self_tag = tag();

        if (self_tag != arg.tag() && self_tag != FVTag::val_ref) {
            return false;
        }

        switch (self_tag) {
        case FVTag::int32:
            return m_data.scalar_v >= arg.m_data.scalar_v;
        case FVTag::flt64:
            return m_data.dbl_v >= arg.m_data.dbl_v;
        case FVTag::val_ref:
            return m_data.fv_p->operator>=(arg);
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
        case FVTag::val_ref:
            return std::format("ref(FastValue({}))", m_data.fv_p->to_string());
        case FVTag::sequence:
            return m_data.obj_p->to_string();
        case FVTag::dud:
            return "(dud)";
        }
    }
}
