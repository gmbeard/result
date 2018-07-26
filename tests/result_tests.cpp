#include "result/result.hpp"
#include "result/traits.hpp"
#include "catch2/catch.hpp"
#include <system_error>
#include <vector>
#include <algorithm>
#include <numeric>
#include <memory>
#include <string>

using IoResult = result::Result<size_t, std::error_code>;
TEST_CASE("Should construct ok value", "[result]") {


    IoResult r = result::ok(42ul);

    REQUIRE_NOTHROW(r.value());
    REQUIRE(42 == r.value());
}

TEST_CASE("Should construct err value", "[result]") {

    IoResult r = result::err(
        std::make_error_code(std::errc::operation_would_block));

    REQUIRE_NOTHROW(r.error());
    REQUIRE(r.error().value() == 
            static_cast<int>(std::errc::operation_would_block));
}

TEST_CASE("Should throw on bad access", "[result]") {

    IoResult r = result::ok(42ul);
    REQUIRE_THROWS_AS(
        r.error(),
        result::BadResultAccess
    );

    r = result::err(
        std::make_error_code(std::errc::operation_would_block));

    REQUIRE_THROWS_AS(
        r.value(),
        result::BadResultAccess
    );
}

TEST_CASE("Should compare equality", "[result]") {
    IoResult r = result::ok(42ul);

    REQUIRE(result::ok(42ul) == r);
    REQUIRE(
        result::err(
            std::make_error_code(
                std::errc::operation_would_block)) != r
    );

    REQUIRE(r != result::ok(43ul));
}

TEST_CASE("Should handle void result", "[result]") {

    using NoValue = result::Result<void, std::error_code>;

    NoValue v = 
        result::err(
            std::make_error_code(std::errc::operation_would_block));

    REQUIRE(!v.is_ok());
    REQUIRE(
        result::err(
            std::make_error_code(
                std::errc::operation_would_block)) == v
    );


    v = result::ok();
    REQUIRE(v.is_ok());
}

TEST_CASE("Should have movable result", "[result]") {

    using R = result::Result<std::vector<uint8_t>, 
                             std::error_code>;

    R r = result::ok(std::vector<uint8_t> { 1, 2, 3, 4});
    auto v = std::move(r.value());
    auto sum = std::accumulate(v.begin(), v.end(), 0);

    REQUIRE(10 == sum);
}

TEST_CASE("Should hold move-only types", "[result]") {
   
    using namespace std::literals;

    using R = result::Result<std::unique_ptr<uint8_t>,
                             std::string>;

    REQUIRE(!std::is_copy_constructible<R>::value);
    REQUIRE(std::is_move_constructible<R>::value);

    R r = result::ok(std::make_unique<uint8_t>(static_cast<uint8_t>(42)));

    REQUIRE(r.is_ok());
    REQUIRE(42 == *r.value());

    r = result::err("Error!"s);
    REQUIRE(!r.is_ok());
    REQUIRE(r.error() == "Error!");
}

namespace type_tests {
    template<typename... >
    using VoidT = void;

    template<typename From, typename To>
    using ConvertsValueTo =
        decltype(To { result::value(std::declval<From>()) });

    template<typename From, typename To>
    using ConvertsErrorTo =
        decltype(To { result::error(std::declval<From>()) });

    template<typename T, 
             template <typename...> class Op, 
             typename... Args>
    struct Test 
        : std::false_type 
    { };

    template<template <typename...> class Op, 
             typename... Args>
    struct Test<VoidT<Op<Args...>>, Op, Args...> 
        : std::true_type 
    { };
}

template<typename From, typename To>
constexpr bool ConvertsValue = 
    type_tests::Test<void, type_tests::ConvertsValueTo, From, To> { };

template<typename From, typename To>
constexpr bool ConvertsError = 
    type_tests::Test<void, type_tests::ConvertsErrorTo, From, To> { };

TEST_CASE("Should perfectly forward values and errors on get", "[result]") {

    using R = result::Result<std::unique_ptr<uint8_t>,
                             std::string>;

    REQUIRE(ConvertsValue<R&&, std::unique_ptr<uint8_t>>);
    REQUIRE(!ConvertsValue<R&, std::unique_ptr<uint8_t>>);
    REQUIRE(!ConvertsValue<R const&, std::unique_ptr<uint8_t>>);

    REQUIRE(ConvertsError<R&&, std::string>);
    REQUIRE(ConvertsError<R&, std::string>);
    REQUIRE(ConvertsError<R const&, std::string>);
}

TEST_CASE("Should implicity cast results to bool") {

    using R = result::Result<void, std::string>;

    R result = result::ok();

    if (auto& r = result) {
        REQUIRE(true);
        return;
    }

    REQUIRE(false);
}

TEST_CASE("Result traits") {
    using namespace result::traits;
    using result::Result;
    using RecursiveValue = Result<Result<size_t, std::string>, int>;
    using RecursiveError = Result<int, Result<size_t, std::string>>;
    using NonRecursive = Result<size_t, std::string>;

    REQUIRE(
        is_convertible_to_result<decltype(result::ok(42)), int, size_t>::value);

    REQUIRE(
        std::is_same<
            size_t, 
            typename inner_result_value_traits<RecursiveValue>::value_type
        >::value);

    REQUIRE(
        std::is_same<
            std::string,
            typename inner_result_error_traits<RecursiveError>::error_type
        >::value);

    REQUIRE(
        std::is_same<
            size_t, 
            typename inner_result_value_traits<NonRecursive>::value_type
        >::value);

    REQUIRE(
        std::is_same<
            std::string,
            typename inner_result_error_traits<NonRecursive>::error_type
        >::value);

    REQUIRE(
        result_traits<RecursiveError>::error_is_result::value);

    REQUIRE(
        !result_traits<NonRecursive>::value_is_result::value);
}

TEST_CASE("Result mapping") {
    using result::Result; 
    using R = Result<int, size_t>;

    {
        auto r = R { result::ok(42) };

        auto r1 = map(std::move(r), [](auto val) { 
            return std::to_string(val); 
        });

        REQUIRE(value(std::move(r1)) == "42");
    }

    {
        bool called = false;
        auto r = R { result::err(42ul) };
        auto r1 = map(std::move(r), [&called](auto val) {
            called = true;
            return std::to_string(val);
        });

        REQUIRE(!called);
        REQUIRE(!r1);
    }
}

TEST_CASE("and_then") {
    using result::Result;

    using R = Result<int, size_t>;

    {
        auto r = R { result::ok(42) };

        auto r1 = result::and_then(std::move(r), [](auto int_val) {
            return result::ok(std::to_string(int_val * 2));
        })
        .and_then([](auto str_val) {
            return result::ok(str_val + " - Hello");
        });

        REQUIRE(value(std::move(r1)) == "84 - Hello");
    }

    {
        auto r = R { result::ok(42) };

        auto r1 = result::and_then(std::move(r), [](auto int_val) {
            return result::ok(std::to_string(int_val * 2));
        })
        .and_then([](auto str_val) {
            return result::err(42ul); //str_val + " - Hello");
        });

        REQUIRE(error(std::move(r1)) == 42ul);
    }
}

TEST_CASE("or_else") {
    using result::Result;

    using R = Result<int, size_t>;

    {
        auto r = R { result::err(42ul) };

        auto r1 = result::or_else(std::move(r), [](auto size_t_val) {
            return result::ok<int>(size_t_val * 2);
        });

        REQUIRE(value(std::move(r1)) == 84);
    }
}

TEST_CASE("value_or_else") {

    using R = result::Result<int, std::string>;

    {
        auto r = R { result::err(std::string { "An error occurred!" }) };
        auto val = result::value_or_else(std::move(r), []() { return 42; });
        REQUIRE(val == 42);
    }

    {
        auto r = R { result::ok(41) };
        auto val = result::value_or_else(std::move(r), []() { return 42; });
        REQUIRE(val == 41);
    }
}
