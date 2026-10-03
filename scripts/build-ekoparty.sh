#!/usr/bin/env bash
set -euo pipefail

env_name="${1:-ekoparty_badge_v1}"
if [[ $# -gt 0 ]]; then
    shift
fi
pio_args=("$@")
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"
profile="${repo_root}/config/userPrefs.ekoparty.jsonc"
active_prefs="${repo_root}/userPrefs.jsonc"
# Keep the XXXXXX suffix at the end for both GNU and BSD/macOS mktemp.
backup="$(mktemp "${TMPDIR:-/tmp}/ekoparty-userPrefs.jsonc.XXXXXX")"
pio_bin="${PLATFORMIO:-}"
build_revision=""
build_state=""
prefs_backed_up=false

if [[ -z "${pio_bin}" ]]; then
    if command -v pio >/dev/null 2>&1; then
        pio_bin="pio"
    elif [[ -x "${HOME}/.platformio/penv/bin/pio" ]]; then
        pio_bin="${HOME}/.platformio/penv/bin/pio"
    else
        echo "PlatformIO not found. Set PLATFORMIO=/path/to/pio." >&2
        exit 1
    fi
fi

restore_prefs() {
    if [[ "${prefs_backed_up}" == true ]]; then
        cp "${backup}" "${active_prefs}"
    fi
    rm -f "${backup}"
}

trap restore_prefs EXIT

cd "${repo_root}"

if [[ ! -f "${profile}" ]]; then
    echo "Missing Ekoparty userPrefs profile: ${profile}" >&2
    exit 1
fi

if [[ -n "$(git status --porcelain --untracked-files=no)" ]]; then
    echo "Warning: tracked working tree has local changes; proceeding and restoring userPrefs.jsonc after build." >&2
fi

build_revision="$(git rev-parse HEAD)"
if [[ -n "$(git status --porcelain)" ]]; then
    build_state="dirty"
else
    build_state="clean"
fi

cp "${active_prefs}" "${backup}"
prefs_backed_up=true
cp "${profile}" "${active_prefs}"

"${pio_bin}" run -e "${env_name}" "${pio_args[@]}"

dist_dir="${repo_root}/dist/${env_name}"
build_dir="${repo_root}/.pio/build/${env_name}"
build_stamp="$(date -u +%Y%m%dT%H%M%SZ)"
mkdir -p "${dist_dir}"

copy_latest_artifact() {
    local pattern="$1"
    local suffix="$2"
    local latest=""
    local candidate
    local source_name
    local stem

    while IFS= read -r candidate; do
        if [[ -z "${latest}" || "${candidate}" -nt "${latest}" ]]; then
            latest="${candidate}"
        fi
    done < <(find "${build_dir}" -maxdepth 1 -type f -name "${pattern}" -print)

    if [[ -z "${latest}" ]]; then
        return
    fi

    source_name="$(basename "${latest}")"
    if [[ "${suffix}" == "mt.json" ]]; then
        stem="${source_name%.mt.json}"
    else
        stem="${source_name%.${suffix}}"
    fi

    cp "${latest}" "${dist_dir}/${stem}-${build_stamp}.${suffix}"
    cp "${latest}" "${dist_dir}/latest-${env_name}.${suffix}"

    if [[ "${suffix}" == "uf2" ]]; then
        latest_uf2="${source_name}"
    fi
}

latest_uf2=""
copy_latest_artifact '*.uf2' 'uf2'
copy_latest_artifact '*.hex' 'hex'
copy_latest_artifact '*.zip' 'zip'
copy_latest_artifact '*.mt.json' 'mt.json'

if [[ -z "${latest_uf2}" ]]; then
    echo "No UF2 artifact found in ${build_dir}" >&2
    exit 1
fi

if command -v sha256sum >/dev/null 2>&1; then
    latest_uf2_sha256="$(sha256sum "${dist_dir}/latest-${env_name}.uf2" | awk '{print $1}')"
else
    latest_uf2_sha256="$(shasum -a 256 "${dist_dir}/latest-${env_name}.uf2" | awk '{print $1}')"
fi

{
    printf 'environment=%s\n' "${env_name}"
    printf 'build_utc=%s\n' "${build_stamp}"
    printf 'git_commit=%s\n' "${build_revision}"
    printf 'git_state=%s\n' "${build_state}"
    printf 'source_uf2=%s\n' "${latest_uf2}"
    printf 'canonical_uf2=latest-%s.uf2\n' "${env_name}"
    printf 'canonical_uf2_sha256=%s\n' "${latest_uf2_sha256}"
} > "${dist_dir}/BUILD_INFO.txt"

echo "Latest UF2: ${dist_dir}/latest-${env_name}.uf2"
echo "Build metadata: ${dist_dir}/BUILD_INFO.txt"
