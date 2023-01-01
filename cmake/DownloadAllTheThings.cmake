include(ExternalProject)
include(FetchContent)

set(THREADS "12")

message(STATUS "Vendoring libs for kotki-lib")

FetchContent_Declare(marian-lite
    GIT_REPOSITORY https://github.com/kroketio/marian-lite.git
    GIT_TAG "6059379dc7ba9f296922fc66e550317f6443858a"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(marian-lite)
FetchContent_GetProperties(marian-lite)
