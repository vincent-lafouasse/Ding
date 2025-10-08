# Fetch gtest through git
#
# Targets : `libgtest` `libgmock`


if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY "https://github.com/google/googletest.git"
	GIT_TAG "main"
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
