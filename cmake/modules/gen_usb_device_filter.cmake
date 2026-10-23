# gen_usb_device_filter.cmake
# Called at build time by copy-resource-android.

cmake_minimum_required(VERSION 3.16)

file(GLOB HID_XML_FILES "${CMAKE_SOURCE_DIR}/res/controllers/*.hid.xml")

if(NOT (TEMPLATE_FILE))
  message(FATAL_ERROR "TEMPLATE_FILE must be set")
endif()

if(NOT (TARGET_FILE))
  message(FATAL_ERROR "TARGET_FILE must be set")
endif()

set(USB_DEVICE_ENTRIES)
foreach(HID_XML_FILE ${HID_XML_FILES})
  get_filename_component(FILENAME "${HID_XML_FILE}" NAME)
  if(FILENAME STREQUAL "Dummy Device Screen.hid.xml")
    continue()
  endif()
  file(STRINGS "${HID_XML_FILE}" HID_LINES REGEX "protocol=\"hid\"")
  foreach(LINE ${HID_LINES})
    string(REGEX MATCH "vendor_id=\"([^\"]+)\"" _ "${LINE}")
    set(VID "${CMAKE_MATCH_1}")
    string(REGEX MATCH "product_id=\"([^\"]+)\"" _ "${LINE}")
    set(PID "${CMAKE_MATCH_1}")
    if(VID AND PID)
      set(ENTRY "    <usb-device vendor-id=\"${VID}\" product-id=\"${PID}\" />")
      list(FIND USB_DEVICE_ENTRIES "${ENTRY}" IDX)
      if(IDX EQUAL -1)
        list(APPEND USB_DEVICE_ENTRIES "${ENTRY}")
      endif()
    endif()
  endforeach()
endforeach()

list(SORT USB_DEVICE_ENTRIES)
string(REPLACE ";" "\n" USB_DEVICE_ENTRIES_STR "${USB_DEVICE_ENTRIES}")

file(READ "${TEMPLATE_FILE}" TEMPLATE_CONTENT)
string(
  REPLACE
  "    @USB_DEVICE_ENTRIES@"
  "${USB_DEVICE_ENTRIES_STR}"
  OUTPUT_CONTENT
  "${TEMPLATE_CONTENT}"
)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/res/xml/")
file(WRITE "${TARGET_FILE}" "${OUTPUT_CONTENT}")
list(LENGTH USB_DEVICE_ENTRIES SUPPORTED_DEVICES_COUNT)
message(
  STATUS
  "Generated supported USB device list for Android launch intent: ${SUPPORTED_DEVICES_COUNT} devices added."
)
