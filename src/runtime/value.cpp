#include "runtime/value.hpp"

namespace Minuet::Runtime {
    Value::Value()
    : m_data {}, m_tag {ValTag::none} {}

    Value::Value(bool b)
    : m_data {b}, m_tag {ValTag::boolean} {}

    Value::Value(int i)
    : m_data {i}, m_tag {ValTag::int32} {}

    Value::Value(double d)
    : m_data {d}, m_tag {ValTag::flt64} {}

    [[nodiscard]] auto Value::operator*(const Value& arg) const& -> Value {
        if (m_tag != arg.m_tag) {
            return {};
        }

        switch (m_tag) {
        case ValTag::int32:
            return std::get<int>(m_data) * std::get<int>(arg.m_data);
        case ValTag::flt64:
            return std::get<double>(m_data) * std::get<double>(arg.m_data);
        default:
            return {};
        }
    }

    [[nodiscard]] auto Value::operator/(const Value& arg) const& -> Value {
        if (m_tag != arg.m_tag) {
            return {};
        }

        switch (m_tag) {
        case ValTag::int32:
            if (const auto divisor = std::get<int>(arg.m_data); divisor != 0) {
                return std::get<int>(m_data) / std::get<int>(arg.m_data);
            }

            return {};
        case ValTag::flt64:
            if (const auto divisor = std::get<double>(arg.m_data); divisor != 0.0) {
                return std::get<double>(m_data) / std::get<double>(arg.m_data);
            }

            return {};
        default:
            return {};
        }
    }

    [[nodiscard]] auto Value::operator%(const Value& arg) const& -> Value {
        if (m_tag != arg.m_tag) {
            return {};
        }

        switch (m_tag) {
        case ValTag::int32:
            if (const auto divisor = std::get<int>(arg.m_data); divisor != 0) {
                return std::get<int>(m_data) % std::get<int>(arg.m_data);
            }

            return {};
        default:
            return {};
        }
    }

    [[nodiscard]] auto Value::operator+(const Value& arg) const& -> Value {
        if (m_tag != arg.m_tag) {
            return {};
        }

        switch (m_tag) {
        case ValTag::int32:
            return std::get<int>(m_data) + std::get<int>(arg.m_data);
        case ValTag::flt64:
            return std::get<double>(m_data) + std::get<double>(arg.m_data);
        default:
            return {};
        }
    }

    [[nodiscard]] auto Value::operator-(const Value& arg) const& -> Value {
        if (m_tag != arg.m_tag) {
            return {};
        }

        switch (m_tag) {
        case ValTag::int32:
            return std::get<int>(m_data) - std::get<int>(arg.m_data);
        case ValTag::flt64:
            return std::get<double>(m_data) - std::get<double>(arg.m_data);
        default:
            return {};
        }
    }


    [[nodiscard]] auto Value::operator*=(const Value& arg) -> Value& {
        if (m_tag != arg.m_tag) {
            m_data = {};
            m_tag = ValTag::none;

            return *this;
        }

        switch (m_tag) {
        case ValTag::int32:
            std::get<int>(m_data) *= std::get<int>(arg.m_data);
            break;
        case ValTag::flt64:
            std::get<double>(m_data) *= std::get<double>(arg.m_data);
            break;
        default:
            m_data = {};
            m_tag = ValTag::none;
            break;
        }

        return *this;
    }

    [[nodiscard]] auto Value::operator/=(const Value& arg) -> Value& {
        if (m_tag != arg.m_tag) {
            m_data = {};
            m_tag = ValTag::none;

            return *this;
        }

        switch (m_tag) {
        case ValTag::int32:
            if (const auto divisor = std::get<int>(arg.m_data); divisor != 0) {
                std::get<int>(m_data) /= divisor;
            } else {
                m_data = {};
                m_tag = ValTag::none;
            }
            break;
        case ValTag::flt64:
            if (const auto divisor = std::get<double>(arg.m_data); divisor != 0.0) {
                std::get<double>(m_data) /= divisor;
            } else {
                m_data = {};
                m_tag = ValTag::none;
            }
            break;
        default:
            m_data = {};
            m_tag = ValTag::none;
            break;
        }

        return *this;
    }

    [[nodiscard]] auto Value::operator%=(const Value& arg) -> Value& {
        if (m_tag != arg.m_tag) {
            m_data = {};
            m_tag = ValTag::none;

            return *this;
        }

        switch (m_tag) {
        case ValTag::int32:
            if (const auto divisor = std::get<int>(arg.m_data); divisor != 0) {
                std::get<int>(m_data) %= divisor;
            } else {
                m_data = {};
                m_tag = ValTag::none;
            }
            break;
        default:
            m_data = {};
            m_tag = ValTag::none;
            break;
        }

        return *this;
    }

    [[nodiscard]] auto Value::operator+=(const Value& arg) -> Value& {
        if (m_tag != arg.m_tag) {
            m_data = {};
            m_tag = ValTag::none;

            return *this;
        }

        switch (m_tag) {
        case ValTag::int32:
            std::get<int>(m_data) += std::get<int>(arg.m_data);
            break;
        case ValTag::flt64:
            std::get<double>(m_data) += std::get<double>(arg.m_data);
            break;
        default:
            m_data = {};
            m_tag = ValTag::none;
            break;
        }

        return *this;
    }

    [[nodiscard]] auto Value::operator-=(const Value& arg) -> Value& {
        if (m_tag != arg.m_tag) {
            m_data = {};
            m_tag = ValTag::none;

            return *this;
        }

        switch (m_tag) {
        case ValTag::int32:
            std::get<int>(m_data) -= std::get<int>(arg.m_data);
            break;
        case ValTag::flt64:
            std::get<double>(m_data) -= std::get<double>(arg.m_data);
            break;
        default:
            m_data = {};
            m_tag = ValTag::none;
            break;
        }

        return *this;
    }

    auto Value::to_string() const& -> std::string {
        if (auto bool_p = std::get_if<bool>(&m_data); bool_p) {
            return std::format("{}", *bool_p);
        } else if (auto i32_p = std::get_if<int>(&m_data); i32_p) {
            return std::format("{}", *i32_p);
        } else if (auto f64_p = std::get_if<double>(&m_data); f64_p) {
            return std::format("{}", *f64_p);
        } else {
            return "(dud)";
        }
    }
}
