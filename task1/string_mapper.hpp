#include <algorithm>
#include <array>
#include <concepts>
#include <optional>
#include <string_view>
#include <typeinfo>

template <size_t max_length>
class String {
public:
    std::array<char, max_length> symbols;
    size_t len;

    constexpr String(const char* string, size_t length) : symbols(), len(length) {
        std::copy(string, string + length, symbols.begin());
    }
    constexpr operator std::string_view() const {
        return std::string_view(symbols.data(), len);
    };
};

constexpr String<256> operator ""_cstr(const char* string, size_t length) {
    return String<256>(string, length);
}

template <class From, auto target>
struct Mapping {
    template <class Base>
    static auto getTarget(const Base& object) requires std::derived_from<From, Base> {
        dynamic_cast<const From&>(object);
        return target;
    }
};

template<class T, class U>
inline std::optional<T> operator |(const std::optional<T>& lhs, const std::optional<U>& rhs) {
    if (lhs.has_value()) {
        return lhs;
    }
    return rhs;
}

template <class Base, class Target, class... Mappings> 
struct ClassMapper {
private:
    template <class Mapping>
    static std::optional<Target> getTarget(const Base& object) {
        try {
            return {Mapping::getTarget(object)};
        } catch(const std::bad_cast& e) {
            return std::optional<Target>{};
        }        
    }
public:
    static std::optional<Target> map(const Base& object) {
        std::optional<Target> t = (getTarget<Mappings>(object) | ... | std::optional<Target>{});
        return t.has_value() ? t : std::nullopt;
    }
};
