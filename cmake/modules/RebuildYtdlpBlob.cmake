# RebuildYtdlpBlob.cmake (script mode: cmake -DYTDLP_VERSION=... -DOUT_BLOB=... -P ...)
#
# Downloads the pinned yt-dlp wheel (+ its matching yt-dlp-ejs dep) from
# PyPI and repacks them as the shebang+zlib zipimport blob that
# youtubedl-android expects at res/raw/ytdlp. __main__.py is carried over
# from the pinned wheel (the entry is version-agnostic: `import yt_dlp;
# yt_dlp.main()`).

if(NOT DEFINED YTDLP_VERSION OR NOT DEFINED OUT_BLOB)
  message(FATAL_ERROR "RebuildYtdlpBlob.cmake needs -DYTDLP_VERSION and -DOUT_BLOB")
endif()

find_program(_py NAMES python3 REQUIRED)

set(_work "${CMAKE_BINARY_DIR}/_ytdlp_blob")
file(MAKE_DIRECTORY "${_work}")

# Which ejs version the pinned wheel needs (default dep pin).
execute_process(
  COMMAND ${_py} -c
    "import json,urllib.request; d=json.load(urllib.request.urlopen('https://pypi.org/pypi/yt-dlp/${YTDLP_VERSION}/json')); print([u['url'] for u in d['urls'] if u['packagetype']=='bdist_wheel'][0]); print([r for r in d['info']['requires_dist'] or [] if 'ejs' in r][0] if any('ejs' in r for r in d['info']['requires_dist'] or []) else '')"
  OUTPUT_VARIABLE _meta
  RESULT_VARIABLE _meta_rc
)
if(NOT _meta_rc EQUAL 0)
  message(FATAL_ERROR "Could not fetch yt-dlp ${YTDLP_VERSION} metadata from PyPI")
endif()
string(REPLACE "\n" ";" _meta_lines "${_meta}")
list(GET _meta_lines 0 _wheel_url)

execute_process(
  COMMAND ${_py} -c
    "import urllib.request; urllib.request.urlretrieve('${_wheel_url}', '${_work}/ytdlp.whl')"
  RESULT_VARIABLE _dl_rc
)
if(NOT _dl_rc EQUAL 0)
  message(FATAL_ERROR "Could not download ${_wheel_url}")
endif()

execute_process(
  COMMAND ${_py} -m pip download yt-dlp-ejs --no-deps -d "${_work}"
  RESULT_VARIABLE _ejs_rc
)
if(NOT _ejs_rc EQUAL 0)
  message(FATAL_ERROR "Could not download yt-dlp-ejs from PyPI")
endif()

file(GLOB _ejs_whl "${_work}/yt_dlp_ejs-*.whl")
if(NOT _ejs_whl)
  message(FATAL_ERROR "yt-dlp-ejs wheel missing in ${_work}")
endif()

execute_process(
  COMMAND ${_py} - "${OUT_BLOB}" "${_work}/ytdlp.whl" "${_ejs_whl}"
  INPUT_FILE ${CMAKE_CURRENT_LIST_DIR}/rebuild_ytdlp_blob.py
  RESULT_VARIABLE _pack_rc
)
if(NOT _pack_rc EQUAL 0)
  message(FATAL_ERROR "Packing yt-dlp blob failed")
endif()
message(STATUS "Fresh yt-dlp ${YTDLP_VERSION} blob at ${OUT_BLOB}")
