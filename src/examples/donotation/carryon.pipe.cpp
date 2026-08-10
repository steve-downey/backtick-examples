// src/examples/donotation/carryon.pipe.cpp                          -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// A Haskell `do` block, desugared by the standard rules and translated call
// for call onto the Monad typeclass from beman.transpose. The source
// program is the closing example of the wikibooks do-notation chapter
// (https://en.wikibooks.org/wiki/Haskell/do_notation), chosen there because
// every desugaring rule fires at least once:
//
//   nameReturnAndCarryOn = do putStr "What is your first name? "
//                             first <- getLine
//                             putStr "And your last name? "
//                             last <- getLine
//                             let full = first++" "++last
//                             putStrLn ("Pleased to meet you, "++full++"!")
//                             return full
//                             putStrLn "I am not finished yet!"
//
// The Haskell 2010 report (section 3.14) rewrites a do block one statement
// at a time:
//
//   do {e}              =  e
//   do {e; stmts}       =  e >> do {stmts}
//   do {v <- e; stmts}  =  e >>= \v -> do {stmts}    -- v a plain variable,
//                                                    -- so no MonadFail case
//   do {let ds; stmts}  =  let ds in do {stmts}
//
// Step by step, always rewriting the first statement:
//
//   1. the `;` rule:
//        putStr "What is your first name? " >>
//        do { first <- getLine; ... }
//
//   2. the `<-` rule; the lambda swallows the rest of the block:
//        putStr "What is your first name? " >>
//        (getLine >>= \first ->
//         do { putStr "And your last name? "; ... })
//
//   3, 4. the same two rules again, binding `last`:
//        ... putStr "And your last name? " >>
//            (getLine >>= \last -> do { let full = ...; ... })
//
//   5. the `let` rule:
//        ... let full = first ++ " " ++ last
//            in do { putStrLn (...); return full; putStrLn "..." }
//
//   6, 7. the `;` rule twice more:
//        ... putStrLn ("Pleased to meet you, " ++ full ++ "!") >>
//            (return full >> do { putStrLn "I am not finished yet!" })
//
//   8. one statement left, so `do {e} = e`, and no `do` remains:
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
// Everything left standing is Monad, not Applicative: the continuations
// handed to >>= use the values bound by earlier statements, which is the
// dependency Applicative cannot express. So the translation targets
// monad_typeclass, and the pieces map as
//
//   >>=            mbind       (bind, the typeclass basis)
//   >>             then        (m >> k  =  m >>= \_ -> k, its default
//                               definition in the Monad class)
//   return         pure        (the operation's modern name, and `return`
//                               is spoken for in C++)
//   \v -> e        a by-value lambda
//   let v = e in   a local variable in the continuation
//
// This half is that translation, calls spelled prefix, exactly as the
// desugared term nests. carryon.backtick.cpp is the same term with the two
// binary operators written between their arguments.

#include <examples/donotation/io_monad.hpp>

#include <string>

using beman::transpose::mbind;
using examples::IO;
using examples::pure;
using examples::then;
using examples::Unit;
using examples::World;
using examples::getLine;
using examples::putStr;
using examples::putStrLn;

// d2b6e88f-3a9e-4601-8574-b4ef8561bcf8
// IO<Unit>, not IO<std::string>: a do block has the type of its last
// statement, and `return full` is just one more statement. It neither ends
// the computation nor sets its result.
IO<Unit> nameReturnAndCarryOn() {
    return then(
        putStr("What is your first name? "),
        mbind(getLine(), [](std::string first) {
            return then(
                putStr("And your last name? "),
                mbind(getLine(), [first](std::string last) {
                    auto full = first + " " + last;
                    return then(
                        putStrLn("Pleased to meet you, " + full + "!"),
                        then(pure(full),
                             putStrLn("I am not finished yet!")));
                }));
        }));
}

int main() {
    World world{.input = {"John", "Smith"}};
    nameReturnAndCarryOn().run(world);
}
// d2b6e88f-3a9e-4601-8574-b4ef8561bcf8 end
