include(ExternalProject)
include(FetchContent)

set(THREADS "12")

message(STATUS "Vendoring libs for kotki-lib")

FetchContent_Declare(marian-lite
    GIT_REPOSITORY https://github.com/kroketio/marian-lite.git
    GIT_TAG "0.1.4"
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(marian-lite)
FetchContent_GetProperties(marian-lite)
