#! /usr/bin/env bash

set -euo pipefail

HERE=$(dirname "$BASH_SOURCE")

FLAGS_STANDARD='-std=c++23'

FLAGS_STRICT='-Werror -Wextra -Wall -pedantic -Wfatal-errors'

FLAGS_LIBRARIES='-lseccomp'
# not that we can't have both `-static` and `-lseccomp`

FLAGS_OPTIMISATION="-Ofast"

FLAGS="$FLAGS_STANDARD $FLAGS_STRICT $FLAGS_LIBRARIES $FLAGS_OPTIMISATION"

clear
g++ $FLAGS -o "$HERE/minq-sandbox" "$HERE/minq-sandbox.cpp"
