#!/usr/bin/env bash
set -euo pipefail

IMGUI_TAG="v1.91.9-docking"
BASE_GH="https://github.com/ocornut/imgui/raw/${IMGUI_TAG}"
BASE_RAW="https://raw.githubusercontent.com/ocornut/imgui/${IMGUI_TAG}"
OUT_DIR="third_party/imgui"

mkdir -p "${OUT_DIR}/backends"

download() {
  local rel_path="$1"
  local out_path="$2"
  local attempts=0

  while [ "$attempts" -lt 10 ]; do
    attempts=$((attempts + 1))

    if curl -fL "${BASE_GH}/${rel_path}" -o "${out_path}"; then
      return 0
    fi

    if curl -fL "${BASE_RAW}/${rel_path}" -o "${out_path}"; then
      return 0
    fi

    sleep 1
  done

  echo "Failed to download ${rel_path} after ${attempts} attempts" >&2
  return 1
}

core_files=(
  imgui.h
  imgui.cpp
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
  imgui_internal.h
  imconfig.h
  imstb_rectpack.h
  imstb_textedit.h
  imstb_truetype.h
  LICENSE.txt
)

backend_files=(
  imgui_impl_sdl3.h
  imgui_impl_sdl3.cpp
  imgui_impl_opengl3.h
  imgui_impl_opengl3.cpp
  imgui_impl_opengl3_loader.h
)

for file in "${core_files[@]}"; do
  if [ -s "${OUT_DIR}/${file}" ]; then
    continue
  fi
  download "${file}" "${OUT_DIR}/${file}"
done

for file in "${backend_files[@]}"; do
  if [ -s "${OUT_DIR}/backends/${file}" ]; then
    continue
  fi
  download "backends/${file}" "${OUT_DIR}/backends/${file}"
done

echo "Dear ImGui (${IMGUI_TAG}) files downloaded to ${OUT_DIR}"
