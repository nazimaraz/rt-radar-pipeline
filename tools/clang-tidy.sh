#!/bin/sh
# Runs clang-tidy over the firmware's C++ sources with the arm-none-eabi
# toolchain's standard library headers. Needs a configured build: cmake --preset qemu
#
#   tools/clang-tidy.sh            all C++ files
#   tools/clang-tidy.sh app/logger.cpp
#
# CLANG_TIDY and ARM_GCC override the tool locations.
set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
tidy=${CLANG_TIDY:-$(command -v clang-tidy || echo /opt/homebrew/opt/llvm/bin/clang-tidy)}
gcc=${ARM_GCC:-$(ls "$HOME"/Toolchains/arm-gnu-toolchain-*/bin/arm-none-eabi-g++ 2>/dev/null | tail -n 1)}
[ -x "$gcc" ] || gcc=$(command -v arm-none-eabi-g++)

mcu="-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16"
# The compiler knows where its libstdc++ and newlib headers live; ask it.
includes=$(echo | "$gcc" $mcu -specs=nano.specs -x c++ -E -v - 2>&1 |
  sed -n '/#include <...> search starts here/,/End of search list/p' |
  grep '^ /' | sed 's/^ //')

args="--extra-arg=--target=arm-none-eabi --extra-arg=-mcpu=cortex-m4"
args="$args --extra-arg=-mfloat-abi=hard --extra-arg=-mfpu=fpv4-sp-d16"
for dir in $includes; do
  args="$args --extra-arg=-isystem$dir"
done

if [ $# -eq 0 ]; then
  set -- $(cd "$root" && find app bsp osal dsp -name '*.cpp')
fi
cd "$root"
# shellcheck disable=SC2086
exec "$tidy" -p build/qemu --quiet $args "$@"
