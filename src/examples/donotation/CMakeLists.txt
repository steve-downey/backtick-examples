# src/examples/donotation/CMakeLists.txt                            -*-cmake-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

# The typeclass vocabulary -- Monad, mbind, monad_typeclass -- comes from
# beman.transpose. Not a vcpkg port, so fall back to FetchContent at a pin,
# the same arrangement the sender examples use for beman.execution.
#
# To build against a local checkout instead:
#   -DFETCHCONTENT_SOURCE_DIR_BEMAN_TRANSPOSE=$HOME/src/transpose
find_package(beman_transpose QUIET)

if(NOT TARGET beman::transpose)
    include(FetchContent)
    FetchContent_Declare(
        beman_transpose
        GIT_REPOSITORY https://github.com/steve-downey/transpose.git
        GIT_TAG 6be88d8859ba4f5a47863fad03550ad74c4d0892
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(beman_transpose)
endif()

if(NOT TARGET beman::transpose)
    message(
        WARNING
        "No provider for beman_transpose. Skipping the do-notation example."
    )
    return()
endif()

add_paired_example(carryon LIBS beman::transpose)
