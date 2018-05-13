#include "result/result.hpp"
#include "catch.hpp"
#include <system_error>

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

    NoValue v1 = result::ok();

    v = v1;

    REQUIRE(!v.is_ok());
    REQUIRE(
        result::err(
            std::make_error_code(
                std::errc::operation_would_block)) == v
    );


    v = result::ok();
    REQUIRE(v.is_ok());
}
