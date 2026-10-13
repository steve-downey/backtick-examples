// src/examples/donotation/io_monad.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_EXAMPLES_DONOTATION_IO_MONAD
#define INCLUDED_EXAMPLES_DONOTATION_IO_MONAD

// Scaffolding shared by the do-notation pair: the teaching model of IO -- a
// function that is given the world and hands back a value -- instantiated
// for the Monad typeclass from beman.transpose. It lives here so that
// carryon.pipe.cpp and carryon.backtick.cpp differ only in how they spell
// their calls, which is the claim the pair is there to check.
//
// The world is what is left of Haskell's RealWorld once the session must be
// deterministic: the lines the user has yet to type. Output goes straight
// to stdout, and getLine echoes what it consumes, so the transcript reads
// exactly as the terminal session would.

#include <beman/transpose/monad.hpp>

#include <deque>
#include <functional>
#include <print>
#include <string>
#include <utility>

namespace examples {

// 5bcec8e5-8043-45e0-8175-a7b0166028f7
/// Haskell's unit type, ().
struct Unit {};

/// The scripted world: the lines the user is going to type.
struct World {
    std::deque<std::string> input;
};

/// IO a: a computation that, given the world, produces an `a`.
template <class VALUE_TYPE>
struct IO {
    std::function<VALUE_TYPE(World &)> run;
};

/// Monad instance for IO: pure produces the value and leaves the world
/// alone; bind runs `ma`, then runs the action the continuation builds from
/// its result, threading the one world through both.
template <class VALUE_TYPE>
struct IOMonadImpl {
    using element_type = VALUE_TYPE;

    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) -> IO<std::remove_cvref_t<VALUE>> {
        return {
            [value = std::forward<VALUE>(value)](World &) { return value; }};
    }

    template <class A, class F>
    auto bind(this auto &&, IO<A> ma, F &&f) -> std::invoke_result_t<F &, A> {
        using MB = std::invoke_result_t<F &, A>;
        return MB{[ma = std::move(ma), f = std::forward<F>(f)](World &world) {
            return std::invoke(f, ma.run(world)).run(world);
        }};
    }
};

template <class VALUE_TYPE>
struct IOMonadMap : beman::transpose::Monad<IOMonadImpl<VALUE_TYPE>> {
    using IOMonadImpl<VALUE_TYPE>::bind;
    using IOMonadImpl<VALUE_TYPE>::pure;
};

} // namespace examples

namespace beman::transpose {
/// Register the instance so mbind and the generic machinery find it.
template <class VALUE_TYPE>
inline constexpr auto monad_typeclass<examples::IO<VALUE_TYPE>> =
    examples::IOMonadMap<VALUE_TYPE>{};
} // namespace beman::transpose

namespace examples {

// -- The Prelude, one name at a time --

inline IO<Unit> putStr(std::string text) {
    return {[text = std::move(text)](World &) {
        std::print("{}", text);
        return Unit{};
    }};
}

inline IO<Unit> putStrLn(std::string line) {
    return {[line = std::move(line)](World &) {
        std::println("{}", line);
        return Unit{};
    }};
}

/// Consumes the next scripted line, echoing it as the terminal would.
inline IO<std::string> getLine() {
    return {[](World &world) {
        std::string line = std::move(world.input.front());
        world.input.pop_front();
        std::println("{}", line);
        return line;
    }};
}

/// Haskell's return: the operation's modern name, and `return` is spoken
/// for in C++.
template <class VALUE>
IO<VALUE> pure(VALUE value) {
    return beman::transpose::monad_typeclass<IO<VALUE>>.pure(std::move(value));
}

/// (>>): sequence two actions, discarding the first result.
/// m >> k  =  m >>= \_ -> k    -- its default definition in the Monad class
template <class A, class B>
IO<B> then(IO<A> action, IO<B> continuation) {
    return beman::transpose::mbind(
        std::move(action), [continuation = std::move(continuation)](const A &) {
            return continuation;
        });
}
// 5bcec8e5-8043-45e0-8175-a7b0166028f7 end

} // namespace examples

#endif // INCLUDED_EXAMPLES_DONOTATION_IO_MONAD
