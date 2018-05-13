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

template<typename T>
struct ResultIsCopyConstructible {

    template<typename _T, typename _E>
    static std::true_type  test(result::Result<_T, _E>);

    static std::false_type test(...);

    static constexpr bool value = 
        std::is_same<std::true_type, 
            decltype(test(std::declval<T const&>()))>::value;
};

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
