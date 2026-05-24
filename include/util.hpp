#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>

using namespace std::chrono;
using namespace std::this_thread;
using namespace std::chrono_literals;

// Get string literal length
constexpr std::size_t operator""_len(const char*, std::size_t ln) noexcept
{
    return ln;
}

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// shotout to the better c++ server for these helper structs
template <typename T = bool>
struct Ok
{
    using value_type = T;
    value_type value;
};

// fire-and-forget result
template <>
struct Ok<void>
{
};

// Deduction guide
template <typename T>
Ok(T) -> Ok<T>;
Ok() -> Ok<void>;

template <typename E = std::string>
struct Err
{
    using value_type = E;
    value_type value;
};
template <typename E>
Err(E) -> Err<E>;

template <typename T = Ok<void>, typename E = Err<std::string>>
struct [[nodiscard]] Result
{
    template <typename U, typename = std::enable_if_t<!std::is_void_v<U>>>
    Result(Ok<U> const& v) : m_value(std::in_place_index<0>, v.value)
    {}

    template <typename U, typename = std::enable_if_t<!std::is_void_v<U>>>
    Result(Ok<U>&& v) : m_value(std::in_place_index<0>, std::move(v.value))
    {}

    template <typename U>
    Result(const Err<U>& e) : m_value(std::in_place_index<1>, e.value)
    {}
    template <typename U>
    Result(Err<U>&& e) : m_value(std::in_place_index<1>, std::move(e.value))
    {}

    bool     ok() const { return std::holds_alternative<T>(m_value); }
    T&       get() { return std::get<T>(m_value); }
    E&       error() { return std::get<E>(m_value); }
    const T& get() const { return std::get<T>(m_value); }
    const E& error() const { return std::get<E>(m_value); }

    template <typename U = T, typename = typename U::value_type>
    typename U::value_type& get_v()
    {
        return std::get<T>(m_value).value;
    }

    template <typename U = T, typename = typename U::value_type>
    const typename U::value_type& get_v() const
    {
        return std::get<T>(m_value).value;
    }

    template <typename U = E, typename = typename U::value_type>
    typename U::value_type& error_v()
    {
        return std::get<E>(m_value).value;
    }

    template <typename U = E, typename = typename U::value_type>
    const typename U::value_type& error_v() const
    {
        return std::get<E>(m_value).value;
    }

private:
    std::variant<T, E> m_value;
};

template <typename E>
struct Result<Ok<void>, E>
{
    Result() : m_ok(true) {}
    Result(Ok<void>&&) : m_ok(true) {}

    template <typename U>
    Result(const Err<U>& err) : m_ok(false), m_value(err.value)
    {}

    template <typename U>
    Result(Err<U>&& err) : m_ok(false), m_value(std::move(err.value))
    {}

    bool     ok() const { return m_ok; }
    E&       error() { return m_value; }
    const E& error() const { return m_value; }

    template <typename U = E, typename = typename U::value_type>
    typename U::value_type& error_v()
    {
        return m_value.value;
    }

    template <typename U = E, typename = typename U::value_type>
    const typename U::value_type& error_v() const
    {
        return m_value.value;
    }

private:
    bool m_ok;
    E    m_value;
};

template <typename E>
constexpr size_t idx(E e) noexcept
{
    static_assert(std::is_enum_v<E>);
    return static_cast<size_t>(e);
}

template <typename Func>
void for_2d(int width, int height, Func&& fun)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            fun(x, y);  // x = col, y = row
}

template <typename Func>
bool for_2d_until(int width, int height, Func&& func)
{
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            if (func(x, y))  // Return true to stop
                return true;
    return false;
}

// In game.cpp
void show_error(const std::string& title, const std::string& msg);

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

inline std::string str_tolower(std::string str)
{
    for (char& x : str)
        x = std::tolower(x);

    return str;
}

inline std::string str_toupper(std::string str)
{
    for (char& x : str)
        x = std::toupper(x);

    return str;
}

// for some reason isalpha() doesn't do its job here
inline bool is_alnum(const uint32_t key)
{
    return key >= 0x20 && key <= 0x7E;
}

inline bool is_alpha(const uint32_t key)
{
    return (key | 32) - 'a' < 26;
}

#endif
