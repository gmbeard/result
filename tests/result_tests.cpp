#include "result/result.hpp"
#include "catch.hpp"
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

    R r = result::ok(std::make_unique<uint8_t>(42));

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
