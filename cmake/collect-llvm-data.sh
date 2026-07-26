#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 AlgorIT Software Consultancy
# SPDX-License-Identifier: BSL-1.0

script_dir=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
root_dir=$(realpath "$script_dir/..")

format=${1:-summary}
build_dir=${2:-$root_dir/build/llvm-coverage}

mapfile -t PROFRAW_FILES < <(find "${build_dir}" -name '*-*.profraw')
if [ ${#PROFRAW_FILES[@]} -eq 0 ]; then
    printf 'Error: no coverage data found.\n' >&2
    exit 1
fi

declare -A buildids=()
for file in "${PROFRAW_FILES[@]}"; do
    base=${file##*/}
    buildid=${base%%-*}
    buildids["$buildid"]=1
done

object_args=()
while IFS= read -r bin; do
    if out=$(readelf -n "$bin" 2>/dev/null | grep "Build ID:"); then
        buildid=${out##* }
        if [[ -n "$buildid" && "${buildids[$buildid]}" == "1" ]]; then
            object_args+=("-object" "$bin")
        fi
    fi
done < <(find "$build_dir" -type f -perm /111)
object_args=("${object_args[@]:1}")

profdata_file="$build_dir/cxx_coverage.profdata"
filename_regex="$root_dir/(build/|tests/)"

llvm-profdata merge -sparse "${PROFRAW_FILES[@]}" -o "${profdata_file}"

if [[ "$format" == "summary" ]]; then

    llvm-cov report \
        -instr-profile="${profdata_file}" \
        -ignore-filename-regex="$filename_regex" \
        -show-branch-summary=false \
        -show-region-summary=false \
        "${object_args[@]}" 2>/dev/null

    exit 0
fi

if [[ "$format" == "report" ]]; then

    coverage_dir=$build_dir/coverage-report
    if [[ -d "${coverage_dir}" ]]; then
        rm -rf "${coverage_dir}"
    fi
    mkdir -p "${coverage_dir}"

    llvm-cov show \
        -instr-profile="${profdata_file}" \
        -ignore-filename-regex="$filename_regex" \
        -format=html \
        -output-dir="${coverage_dir}" \
        "${object_args[@]}"

    exit 0
fi

if [[ "$format" == "data" ]]; then

    coverage_file=$build_dir/coverage.info
    if [[ -f "${coverage_file}" ]]; then
        rm "${coverage_file}"
    fi

    llvm-cov export \
        -instr-profile="${profdata_file}" \
        -ignore-filename-regex="$filename_regex" \
        -format=lcov \
        "${object_args[@]}" >"$coverage_file"

    exit 0
fi

printf 'Error: unknown format %s.\n' "$format" >&2
exit 1
