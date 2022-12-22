add_executable(kotki-cli kotki.cpp)
target_link_libraries(kotki-cli PRIVATE kotki-lib)

# .srt subtitle file convertor
add_executable(kotki-srt kotki-srt.cpp)
target_link_libraries(kotki-srt PRIVATE kotki-lib)
