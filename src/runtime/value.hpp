#ifndef VALUE_HPP
#define VALUE_HPP

#include <variant>

namespace Minuet::Runtime {
    enum class ValTag {
        none,
        boolean,
        int32,
        flt64,
    };

    struct ValDud {};

    class Value {
    public:
        Value();
        Value(bool b);
        Value(int i);
        Value(double d);
        // Value(ObjHandle h);

        explicit operator bool(this auto&& self) noexcept {
            switch (self.m_tag) {
            case ValTag::boolean:
                return std::get<bool>(self.m_data);
            case ValTag::int32:
                return std::get<int>(self.m_data) != 0;
            case ValTag::flt64:
                return std::get<double>(self.m_data) != 0.0;
            default:
                return false;
            }
        }

        [[nodiscard]] auto operator*(const Value& arg) const& -> Value;
        [[nodiscard]] auto operator/(const Value& arg) const& -> Value;
        [[nodiscard]] auto operator%(const Value& arg) const& -> Value;
        [[nodiscard]] auto operator+(const Value& arg) const& -> Value;
        [[nodiscard]] auto operator-(const Value& arg) const& -> Value;

        [[nodiscard]] auto operator*=(const Value& arg) -> Value&;
        [[nodiscard]] auto operator/=(const Value& arg) -> Value&;
        [[nodiscard]] auto operator%=(const Value& arg) -> Value&;
        [[nodiscard]] auto operator+=(const Value& arg) -> Value&;
        [[nodiscard]] auto operator-=(const Value& arg) -> Value&;

    private:
        std::variant<ValDud, bool, int, double> m_data;
        ValTag m_tag;
    };
}

#endif
