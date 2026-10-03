#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
workspace_root="$(cd "${repo_root}/.." && pwd)"
bootloader_root="${EKOPARTY_BOOTLOADER_ROOT:-${workspace_root}/eko-bootloader}"
board="ekoparty_badge_v1"
softdevice="${repo_root}/bin/s140_nrf52_7.3.0_softdevice.hex"
build_dir="${bootloader_root}/_build/build-${board}"
output_dir="${repo_root}/dist/${board}"
output_image="${output_dir}/EKO_FIRST_FLASH.hex"
bootloader_python="${bootloader_root}/.venv/bin/python"

if [[ ! -d "${bootloader_root}/src/boards/${board}" ]]; then
    echo "Missing ${board} bootloader variant in ${bootloader_root}" >&2
    exit 1
fi

if [[ ! -x "${bootloader_python}" ]]; then
    echo "Missing bootloader virtual environment: ${bootloader_python}" >&2
    echo "Create it and install intelhex plus adafruit-nrfutil before building." >&2
    exit 1
fi

if ! "${bootloader_python}" -c 'import intelhex' >/dev/null 2>&1; then
    echo "The bootloader environment is missing intelhex." >&2
    exit 1
fi

if [[ ! -x "${bootloader_root}/.venv/bin/adafruit-nrfutil" ]]; then
    echo "The bootloader environment is missing adafruit-nrfutil." >&2
    exit 1
fi

toolchain_prefix="${ARM_NONE_EABI_PREFIX:-}"
if [[ -z "${toolchain_prefix}" ]] && command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    compiler_path="$(command -v arm-none-eabi-gcc)"
    toolchain_prefix="${compiler_path%gcc}"
fi

if [[ -z "${toolchain_prefix}" ]]; then
    user_directory="$(cd ~ && pwd)"
    platformio_prefix="${user_directory}/.platformio/packages/toolchain-gccarmnoneeabi/bin/arm-none-eabi-"
    if [[ -x "${platformio_prefix}gcc" ]]; then
        toolchain_prefix="${platformio_prefix}"
    fi
fi

if [[ -z "${toolchain_prefix}" || ! -x "${toolchain_prefix}gcc" ]]; then
    echo "arm-none-eabi-gcc not found. Set ARM_NONE_EABI_PREFIX to the prefix ending in arm-none-eabi-." >&2
    exit 1
fi

export PATH="${bootloader_root}/.venv/bin:$(dirname "${toolchain_prefix}"):${PATH}"
if [[ -z "${SOURCE_DATE_EPOCH:-}" ]]; then
    SOURCE_DATE_EPOCH="$(git -C "${bootloader_root}" show -s --format=%ct HEAD)"
fi
export SOURCE_DATE_EPOCH

write_sha256() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1"
    elif command -v shasum >/dev/null 2>&1; then
        shasum -a 256 "$1"
    else
        echo "Neither sha256sum nor shasum is available." >&2
        return 1
    fi
}

make -C "${bootloader_root}" BOARD="${board}" clean
make -C "${bootloader_root}" \
    BOARD="${board}" \
    PYTHON="${bootloader_python}" \
    CROSS_COMPILE="${toolchain_prefix}" \
    all

candidates=()
while IFS= read -r candidate; do
    candidates+=("${candidate}")
done < <(find "${build_dir}" -maxdepth 1 -type f -name "*_s140_7.3.0.hex" -print)
if [[ ${#candidates[@]} -ne 1 ]]; then
    echo "Expected exactly one S140 7.3.0 merged HEX in ${build_dir}; found ${#candidates[@]}." >&2
    exit 1
fi

mkdir -p "${output_dir}"
cp "${candidates[0]}" "${output_image}"

"${repo_root}/scripts/verify-ekoparty-first-flash.py" \
    "${output_image}" \
    --softdevice "${softdevice}"

(
    cd "${output_dir}"
    write_sha256 "$(basename "${output_image}")" > "$(basename "${output_image}").sha256"
)
echo "First-flash image: ${output_image}"
echo "Checksum: ${output_image}.sha256"
