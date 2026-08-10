// src/examples/donotation/carryon.backtick.cpp                      -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// carryon.pipe.cpp with the calls spelled infix. That file holds the whole
// derivation: the wikibooks do block, the Haskell 2010 rules, and the
// desugared term
//
//   nameReturnAndCarryOn =
//     putStr "What is your first name? " >>
//     (getLine >>= \first ->
//      putStr "And your last name? " >>
//      (getLine >>= \last ->
//       let full = first ++ " " ++ last
//       in putStrLn ("Pleased to meet you, " ++ full ++ "!") >>
//          (return full >>
//           putStrLn "I am not finished yet!")))
//
// whose prefix translation this file respells. The two operators the sugar
// compiled down to, >>= and >>, are binary -- that is what an operator is
// in Haskell, an infix name for a two-argument function -- so under this
// proposal their C++ names go back between their arguments:
//
//   m >>= \v -> e      m `mbind` [](V v) { return e; }
//   m >> k             m `then` k
//
// return/pure takes one argument, so it stays a call, the same boundary the
// scorecard draws. The parentheses around each right-hand side are the one
// piece of Haskell that does not carry over: there the trailing lambda
// extends to the end of the expression, while the backtick is
// left-associative and binds tightest, so the grouping is spelled. And the
// syntax is no coincidence -- `mbind` between its arguments is Haskell's
// own backtick notation, x `div` y, come home with the semantics attached.

#include <examples/donotation/io_monad.hpp>

#include <string>

using beman::transpose::mbind;
using examples::getLine;
using examples::IO;
using examples::pure;
using examples::putStr;
using examples::putStrLn;
using examples::then;
using examples::Unit;
using examples::World;

// 3c0c7dba-b924-45ce-960f-bab25956a4fc
// IO<Unit>, not IO<std::string>: a do block has the type of its last
// statement, and `return full` is just one more statement. It neither ends
// the computation nor sets its result.
IO<Unit> nameReturnAndCarryOn() {
    return putStr("What is your first name? ") `then`
        (getLine() `mbind` [](std::string first) {
            return putStr("And your last name? ") `then`
                (getLine() `mbind` [first](std::string last) {
                    auto full = first + " " + last;
                    return putStrLn("Pleased to meet you, " + full + "!") `then`
                        (pure(full) `then` putStrLn("I am not finished yet!"));
                });
        });
}

int main() {
    World world{.input = {"John", "Smith"}};
    nameReturnAndCarryOn().run(world);
}
// 3c0c7dba-b924-45ce-960f-bab25956a4fc end
