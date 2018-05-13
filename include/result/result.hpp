#ifndef RESULT_RESULT_HPP_INCLUDED
#define RESULT_RESULT_HPP_INCLUDED

#include <stdexcept>
#include <type_traits>

namespace result {

    namespace detail {
        template<typename T>
        struct Ok {
            Ok(T value) : value_{std::move(value)}
            { }

            auto get() -> T& { return value_; }
        private:
            T value_;
        };

        template<typename E>
        struct Err {
            Err(E value) : value_{std::move(value)}
            { }

            auto get() -> E& { return value_; }
        private:
            E value_;
        };

        struct OkGuide { };
        struct ErrGuide { };

        constexpr OkGuide OK_GUIDE { };
        constexpr ErrGuide ERR_GUIDE { };
    }

    template<typename T>
    auto ok(T&& value) -> detail::Ok<T> {
        return detail::Ok<T>{std::forward<T>(value)};
    }

    template<typename E>
    auto err(E&& value) -> detail::Err<E> {
        return detail::Err<E>{std::forward<E>(value)};
    }

    struct BadResultAccess : std::runtime_error {
        BadResultAccess(char const* what) :
            std::runtime_error { what }
        { }
    };

    template<typename T, typename E>
    struct Result {

        static_assert(
            std::is_nothrow_default_constructible<T>::value,
            "`T` must be nothrow default constructible"
        );

        template<typename U>
        Result(detail::Ok<U> ok) 
            noexcept(noexcept(T{std::declval<U>()})) :
            good_result_{true}
        ,   storage_{std::move(ok.get()), detail::OK_GUIDE}
        { }

        template<typename U>
        Result(detail::Err<U> err) 
            noexcept(noexcept(E{std::declval<U>()})):
            good_result_{false}
        ,   storage_{std::move(err.get()), detail::ERR_GUIDE}
        { }

        Result(Result&& other) 
            noexcept(
                noexcept(T{std::declval<T>()}) &&
                noexcept(E{std::declval<E>()})
            ) :
            good_result_{other.good_result_}
        ,   storage_{T{}}
        { 
            storage_.destroy(true);
            move_construct(*this, std::forward<Result>(other));
        }

        Result(Result const& other)
            noexcept(
                noexcept(T{std::declval<T const&>()}) &&
                noexcept(E{std::declval<E const&>()})
            ) :
            good_result_{other.good_result_}
        ,   storage_{T{}}
        { 
            storage_.destroy(true);
            copy_construct(*this, other);
        }

        auto operator=(Result&& other)
            noexcept(
                noexcept(T{std::declval<T>()}) &&
                noexcept(E{std::declval<E>()})
            ) -> Result&
        {
            storage_.destroy(good_result_);
            move_construct(*this, std::forward<Result>(other));
            return *this;
        }

        auto operator=(Result const& other)
            noexcept(
                noexcept(T{std::declval<T const&>()}) &&
                noexcept(E{std::declval<E const&>()})
            ) -> Result&
        {
            storage_.destroy(good_result_);
            copy_construct(*this, other);
            return *this;
        }

        ~Result() {
            storage_.destroy(good_result_);
        }

        auto is_ok() const noexcept -> bool {
            return good_result_;
        }

        auto value() -> T& {
            if (!is_ok()) {
                throw BadResultAccess{
                    "Result contains an error"
                };
            }

            return storage_.success;
        }

        auto value() const -> T const& {
            return const_cast<Result&>(*this).value();
        }

        auto error() -> E& {
            if (is_ok()) {
                throw BadResultAccess{
                    "Result doesn't contain an error"
                };
            }

            return storage_.error;
        }

        auto error() const -> E const& {
            return const_cast<Result&>(*this).error();
        }

        friend auto operator==(Result const& lhs, 
                               Result const& rhs) noexcept -> bool 
        {
            return lhs.good_result_ == rhs.good_result_ && (
                (lhs.good_result_ && lhs.value() == rhs.value()) ||
                (!lhs.good_result_ && lhs.error() == rhs.error())
            );
        }

        friend auto operator!=(Result const& lhs, 
                               Result const& rhs) noexcept -> bool 
        {
            return !(lhs == rhs);
        }

    private:

        auto static move_construct(Result& _this, Result&& other) {
            _this.good_result_ = other.good_result_;

            try {
                Storage::move_construct(_this.storage_,
                                        std::move(other.storage_),
                                        _this.good_result_);
            }
            catch (...) {
                new (&_this.storage_.success) T{};
                _this.good_result_ = true;
                throw;
            }
        }

        auto static copy_construct(Result& _this, Result const& other) {
            _this.good_result_ = other.good_result_;
            try {
                Storage::copy_construct(_this.storage_,
                                        std::move(other.storage_),
                                        _this.good_result_);
            }
            catch (...) {
                new (&_this.storage_.success) T{};
                _this.good_result_ = true;
                throw;
            }
        }

        union Storage {
            T success;
            E error;

            Storage(T good) :
                success{std::move(good)}
            { }

            Storage(E bad) :
                error{std::move(bad)}
            { }

            template<typename U>
            Storage(U&& good, detail::OkGuide) :
                success{std::forward<U>(good)}
            { }

            template<typename U>
            Storage(U&& bad, detail::ErrGuide) :
                error{std::forward<U>(bad)}
            { }

            ~Storage() { }

            static auto copy_construct(Storage& _this,
                                       Storage const& other,
                                       bool other_is_good)
                noexcept(
                    noexcept(T{std::declval<T const&>()}) &&
                    noexcept(E{std::declval<E const&>()})
                )
            {
                if (other_is_good) {
                    return new (&_this.success) T{other.success};
                }
                else {
                    return new (&_this.error) E{other.error};
                }
            }

            static auto move_construct(Storage& _this, 
                                       Storage&& other,
                                       bool other_is_good)
                noexcept(
                    noexcept(T{std::declval<T>()}) &&
                    noexcept(E{std::declval<E>()})
                )
            {
                if (other_is_good) {
                    new (&_this.success) T{std::move(other.success)};
                }
                else {
                    new (&_this.error) E{std::move(other.error)};
                }
            }

            auto destroy(bool good) -> void {
                if (good) {
                    success.~T();
                }
                else {
                    error.~E();
                }
            }
        };

        bool good_result_;
        Storage storage_;
    };
}
#endif //RESULT_RESULT_HPP_INCLUDED
