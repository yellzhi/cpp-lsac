file(GLOB sources "*.cpp" "*.h")

add_library(brccore ${sources})

target_include_directories(brccore PRIVATE "${UTILS_INCLUDE_DIR}")
target_link_libraries(brccore PUBLIC devcrypto devcore)
