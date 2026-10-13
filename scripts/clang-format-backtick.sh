#!/usr/bin/env bash
# scripts/clang-format-backtick.sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Format with the prototype's clang-format rather than a stock one.
#
# The infix backtick was implemented in clang/lib/Format alongside the parser,
# so the compiler prefix ships a clang-format that knows the canonical style:
# spaces outside the pair, hug inside -- a `f` b. A stock clang-format has not
# been observed to mangle these files, but it has no rule for the operator, so
# it is only a matter of time.
#
# Override with CLANG_FORMAT=/path/to/clang-format.

set -euo pipefail

CLANG_FORMAT="${CLANG_FORMAT:-$HOME/install/clang-23-backtick/bin/clang-format}"

if [[ ! -x ${CLANG_FORMAT} ]]; then
    if [[ ${GITHUB_ACTIONS:-false} == true ]]; then
        echo "clang-format-backtick: ${CLANG_FORMAT} is unavailable on the hosted runner; skipping." >&2
        exit 0
    fi
    echo "clang-format-backtick: ${CLANG_FORMAT} not found." >&2
    echo "Build and install a prototype (ops/build/configure-clang23-backtick.sh" >&2
    echo "in the backtick repo), or set CLANG_FORMAT to one that understands" >&2
    echo "the infix backtick." >&2
    exit 1
fi

exec "${CLANG_FORMAT}" -i --style=file "$@"
