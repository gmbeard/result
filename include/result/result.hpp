#ifndef RESULT_RESULT_HPP_INCLUDED
#define RESULT_RESULT_HPP_INCLUDED

#include "result/traits.hpp"
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

        struct VoidType { };

        template<
            typename T, 
            typename E,
            bool Copyable = 
                std::is_copy_constructible<T>::value &&
                std::is_copy_constructible<E>::value>
        struct Storage {

            Storage() noexcept :
                storage_{}
            ,   tag_{UnionTag::Empty}
            { }

            template<typename U>
            Storage(Ok<U> ok)
                noexcept(noexcept(T{std::declval<U>()})) :
                storage_{std::move(ok.get()), VALUE}
            ,   tag_{UnionTag::Value}
            { }

            template<typename U>
            Storage(Err<U> err)
                noexcept(noexcept(E{std::declval<U>()})) :
                storage_{std::move(err.get()), ERROR}
            ,   tag_{UnionTag::Error}
            { }

            Storage(Storage&& other)
                noexcept(
                    noexcept(T{std::declval<T>()}) &&
                    noexcept(E{std::declval<E>()})) :
                storage_{}
            ,   tag_{UnionTag::Empty}
            {
                storage_.destroy(tag_);
                move_construct(*this, std::move(other));
            }

            Storage(Storage const& other)
                noexcept(
                    noexcept(T{std::declval<T const&>()}) &&
                    noexcept(E{std::declval<E const&>()})) :
                storage_{}
            ,   tag_{UnionTag::Empty}
            {
                storage_.destroy(tag_);
                copy_construct(*this, other);
            }

            auto operator=(Storage&& other)
                noexcept(
                    noexcept(T{std::declval<T>()}) &&
                    noexcept(E{std::declval<E>()}))
                    -> Storage&
            {
                storage_.destroy(tag_);
                move_construct(*this, std::move(other));
                return *this;
            }

            auto operator=(Storage const& other)
                noexcept(
                    noexcept(T{std::declval<T const&>()}) &&
                    noexcept(E{std::declval<E const&>()}))
                    -> Storage&
            {
                storage_.destroy(tag_);
                copy_construct(*this, other);
                return *this;
            }

            ~Storage() {
                storage_.destroy(tag_);
            }

            static auto move_construct(Storage& _this,
                                       Storage&& other)
                noexcept(
                    noexcept(T{std::declval<T>()}) &&
                    noexcept(E{std::declval<E>()}))
            {
                try {
                    Union::move_construct(_this.storage_, 
                                          std::move(other.storage_),
                                          other.tag_);
                    _this.tag_ = other.tag_;
                }
                catch (...) {
                    new (&_this.storage_.empty) Default{};
                    throw;
                }
            }

            static auto copy_construct(Storage& _this,
                                       Storage const& other)
                noexcept(
                    noexcept(T{std::declval<T const&>()}) &&
                    noexcept(E{std::declval<E const&>()}))
            {
                try {
                    Union::copy_construct(_this.storage_, 
                                          other.storage_,
                                          other.tag_);
                    _this.tag_ = other.tag_;
                }
                catch (...) {
                    new (&_this.storage_.empty) Default{};
                    throw;
                }
            }

            struct Default { };
            struct ValueGuide { };
            struct ErrorGuide { };

            static constexpr ValueGuide VALUE { };
            static constexpr ErrorGuide ERROR { };

            enum class UnionTag {
                Empty,
                Value,
                Error
            };

            union Union {
                Default empty;
                T value;
                E error;

                Union() noexcept :
                    empty{}
                { }

                template<typename U>
                Union(U&& item, ValueGuide)
                    noexcept(noexcept(T{std::forward<U>(item)})) :
                    value{std::forward<U>(item)}
                { }

                template<typename U>
                Union(U&& item, ErrorGuide)
                    noexcept(noexcept(E{std::forward<U>(item)})) :
                    error{std::forward<U>(item)}
                { }

                ~Union() { }

                static auto move_construct(Union& u, 
                                           Union&& item, 
                                           UnionTag tag) 
                    noexcept(
                        noexcept(T{std::declval<T>()}) &&
                        noexcept(E{std::declval<E>()}))
                {
                    switch (tag) {
                        case UnionTag::Empty:
                            new (&u.empty) Default{std::move(item.empty)};
                            break;
                        case UnionTag::Value:
                            new (&u.value) T{std::move(item.value)};
                            break;
                        case UnionTag::Error:
                            new (&u.error) E{std::move(item.error)};
                            break;
                    }
                }

                static auto copy_construct(Union& u, 
                                           Union const& item, 
                                           UnionTag tag) 
                    noexcept(
                        noexcept(T{std::declval<T const&>()}) &&
                        noexcept(E{std::declval<E const&>()}))
                {
                    switch (tag) {
                        case UnionTag::Empty:
                            new (&u.empty) Default{item.empty};
                            break;
                        case UnionTag::Value:
                            new (&u.value) T{item.value};
                            break;
                        case UnionTag::Error:
                            new (&u.error) E{item.error};
                            break;
                    }
                }

                auto destroy(UnionTag tag) noexcept {
                    switch (tag) {
                        case UnionTag::Empty:
                            empty.~Default();
                            break;
                        case UnionTag::Value:
                            value.~T();
                            break;
                        case UnionTag::Error:
                            error.~E();
                            break;
                    }
                }
            };

            Union storage_;
            UnionTag tag_;
        };

        template<typename T, typename E>
        struct Storage<T, E, false> : Storage<T, E, true> {

            using Base = Storage<T, E, true>;

            template<typename U>
            Storage(Ok<U> ok)
                noexcept(noexcept(Base{std::declval<Ok<U>>()})) :
                Base{std::move(ok)}
            { }

            template<typename U>
            Storage(Err<U> err)
                noexcept(noexcept(Base{std::declval<Err<U>>()})) :
                Base{std::move(err)}
            { }

            Storage(Storage&&) = default;
            Storage(Storage const&) = delete;
            auto operator=(Storage&&) -> Storage& = default;
            auto operator=(Storage const&) -> Storage& = delete;
        };
    }

    template<typename T>
    auto ok(T&& value) 
        -> detail::Ok<typename std::remove_reference<T>::type> 
    {
        return { std::forward<T>(value) };
    }

    inline auto ok() -> detail::Ok<detail::VoidType> {
        return detail::Ok<detail::VoidType>{
            detail::VoidType{}};
    }

    template<typename E>
    auto err(E&& value) 
        -> detail::Err<typename std::remove_reference<E>::type> 
    {
        return { std::forward<E>(value) };
    }

    struct BadResultAccess : std::runtime_error {
        BadResultAccess(char const* what) :
            std::runtime_error { what }
        { }
    };

    template<typename T, typename E>
    struct Result : 
        private detail::Storage<T, E> 
    {

        using Base = detail::Storage<T, E>;

        template<typename U>
        Result(detail::Ok<U> ok) 
            noexcept(noexcept(Base{std::move(ok)})) :
            Base{std::move(ok)}
        { }
        
        template<typename U>
        Result(detail::Err<U> err) 
            noexcept(noexcept(Base{std::move(err)})) :
            Base{std::move(err)}
        { }

        Result() = delete;

        friend auto operator==(Result const& lhs,
                               Result const& rhs) noexcept 
            -> bool
        {
            return (lhs.tag_ == rhs.tag_) &&
                (lhs.tag_ == Base::UnionTag::Empty ||
                    (lhs.tag_ == Base::UnionTag::Value && 
                        lhs.storage_.value == rhs.storage_.value) ||
                    (lhs.tag_ == Base::UnionTag::Error &&
                        lhs.storage_.error == rhs.storage_.error));
        }

        friend auto operator!=(Result const& lhs,
                               Result const& rhs) noexcept
            -> bool
        {
            return !(lhs == rhs);
        }

        auto is_ok() const noexcept -> bool {
            return Base::tag_ == Base::UnionTag::Value;
        }

        operator bool() const {
            return is_ok();
        }

        auto value() & -> T& {
            if (Base::tag_ != Base::UnionTag::Value) {
                throw BadResultAccess { "The result contains an error" };
            }

            return Base::storage_.value;
        }

        auto value() const& -> const T& {
            return const_cast<Result&>(*this).value();
        }

        auto value() && -> T&& {
            return std::move(value());
        }

        auto error() & -> E& {
            if (Base::tag_ != Base::UnionTag::Error) {
                throw BadResultAccess { "The result doesn't contain an error" };
            }

            return Base::storage_.error;
        }

        auto error() const& -> const E& {
            return const_cast<Result&>(*this).error();
        }

        auto error() && -> E&& {
            return std::move(error());
        }

        template<typename F>
        auto map(F&& f) &&
            -> Result<typename std::result_of<F(T&&)>::type, E>
        {
            if (is_ok()) {
                return result::ok(
                    std::forward<F>(f)(std::move(*this).value()));
            }
            return result::err(std::move(*this).error());
        }

        template<typename F>
        auto map_err(F&& f) &&
            -> Result<T, typename std::result_of<F(E&&)>::type>
        {
            if (!is_ok()) {
                return result::err(
                    std::forward<F>(f)(std::move(*this).error()));
            }
            return result::ok(std::move(*this).value());
        }

        template<
            typename F,
            typename R = 
                typename std::remove_reference<
                    typename std::result_of<F(T&&)>::type>::type,
            typename 
                std::enable_if<traits::is_result<R>::value>::type* = nullptr
        >
        auto and_then(F&& f) &&
            -> Result<typename traits::result_traits<R>::value_type, E>
        {
            if (is_ok()) {
                return std::forward<F>(f)(std::move(*this).value());
            }
            return result::err(std::move(*this).error());
        }
    };

    template<typename E>
    struct Result<void, E> : 
        private detail::Storage<detail::VoidType, E> 
    {

        using Base = detail::Storage<detail::VoidType, E>;

        Result(detail::Ok<detail::VoidType> ok) noexcept :
            Base{ok}
        { }

        template<typename U>
        Result(detail::Err<U> err) 
            noexcept(noexcept(Base{std::move(err)})) :
            Base{std::move(err)}
        { }

        Result() = delete;

        friend auto operator==(Result const& lhs,
                               Result const& rhs) noexcept 
            -> bool
        {
            return (lhs.tag_ == rhs.tag_) &&
                (lhs.tag_ == Base::UnionTag::Empty ||
                lhs.tag_ == Base::UnionTag::Value ||
                (lhs.tag_ == Base::UnionTag::Error &&
                    lhs.storage_.error == rhs.storage_.error));
        }

        friend auto operator!=(Result const& lhs,
                               Result const& rhs) noexcept
            -> bool
        {
            return !(lhs == rhs);
        }

        auto is_ok() const noexcept -> bool {
            return Base::tag_ == Base::UnionTag::Value;
        }

        operator bool() const {
            return is_ok();
        }

        auto error() & -> E& {
            if (Base::tag_ != Base::UnionTag::Error) {
                throw BadResultAccess { "The result doesn't contain an error" };
            }

            return Base::storage_.error;
        }

        auto error() const& -> const E& {
            return const_cast<Result&>(*this).error();
        }

        auto error() && -> E&& {
            return std::move(error());
        }

        template<typename F>
        auto map(F&& f) &&
            -> Result<typename std::result_of<F()>::type, E>
        {
            if (is_ok()) {
                return result::ok(std::forward<F>(f)());
            }
            return result::err(std::move(*this).error());
        }

        template<typename F>
        auto map_err(F&& f) &&
            -> Result<void, typename std::result_of<F(E&&)>::type>
        {
            if (!is_ok()) {
                return result::err(
                    std::forward<F>(f)(std::move(*this).error()));
            }
            return result::ok(std::move(*this).value());
        }
    };

    template<typename T, typename E>
    auto& value(Result<T, E>& r) {
        return r.value();
    }

    template<typename T, typename E>
    auto&& value(Result<T, E>&& r) {
        return std::move(r).value();
    }

    template<typename T, typename E>
    auto const& value(Result<T, E> const& r) {
        return r.value();
    }

    template<typename T, typename E>
    auto& error(Result<T, E>& r) {
        return r.error();
    }

    template<typename T, typename E>
    auto&& error(Result<T, E>&& r) {
        return std::move(r).error();
    }

    template<typename T, typename E>
    auto const& error(Result<T, E> const& r) {
        return r.error();
    }

    template<typename T, typename E, typename F>
    auto map(Result<T, E>&& r, F&& f) {
        return std::move(r).map(std::forward<F>(f));
    }

    template<typename T, typename E, typename F>
    auto map_err(Result<T, E>&& r, F&& f) {
        return std::move(r).map_err(std::forward<F>(f));
    }
}
#endif //RESULT_RESULT_HPP_INCLUDED
