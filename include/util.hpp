#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// Get string literal length
constexpr std::size_t operator""_len(const char*, std::size_t ln) noexcept
{
    return ln;
}

template <typename T = bool>
struct Ok
{
    T value;
};

template <typename E = std::string>
struct Err
{
    E value;
};

template <typename T = Ok<>, typename E = Err<>>
class Result
{
    std::variant<T, E> value;

public:
    template <typename U>
    Result(Ok<U> const& v) : value(std::in_place_index<0>, v.value)
    {}
    template <typename U>
    Result(Ok<U>&& v) : value(std::in_place_index<0>, std::move(v.value))
    {}

    template <typename U>
    Result(Err<U> const& e) : value(std::in_place_index<1>, e.value)
    {}
    template <typename U>
    Result(Err<U>&& e) : value(std::in_place_index<1>, std::move(e.value))
    {}

    bool     ok() const { return std::holds_alternative<T>(value); }
    T&       get() { return std::get<T>(value); }
    E&       error() { return std::get<E>(value); }
    const T& get() const { return std::get<T>(value); }
    const E& error() const { return std::get<E>(value); }
};

/* Spilt a string into a vector using a delimeter
 * @param text The string to split
 * @param delim The delimeter used for spliting the text
 */
inline std::vector<std::string> split(const std::string_view text, const char delim)
{
    std::string              line;
    std::vector<std::string> vec;
    std::stringstream        ss(text.data());
    while (std::getline(ss, line, delim))
    {
        vec.push_back(line);
    }

    return vec;
}

#endif
