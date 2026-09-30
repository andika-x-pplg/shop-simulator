cmake_minimum_required(VERSION 3.20...3.30)
project(ShopSimulator LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setup FetchContent for raylib
include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(USE_WAYLAND OFF CACHE BOOL "" FORCE)

message(STATUS "Fetching raylib 5.0...")
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# Source files
set(SOURCES
    src/main.cpp
    src/Player.cpp
    src/Shop.cpp
    src/Rack.cpp
    src/Customer.cpp
    src/Cashier.cpp
    src/Storage.cpp
    src/Supplier.cpp
    src/Finance.cpp
    src/PriceManager.cpp
    src/Reputation.cpp
    src/ShopUpgrade.cpp
    src/Furniture.cpp
    src/GameTime.cpp
    src/DailyStats.cpp
    src/SaveSystem.cpp
)

# Header files for IDE navigation
set(HEADERS
    include/Player.hpp
    include/Shop.hpp
    include/Rack.hpp
    include/Product.hpp
    include/Customer.hpp
    include/Cashier.hpp
    include/Storage.hpp
    include/Supplier.hpp
    include/Common.hpp
    include/Finance.hpp
    include/PriceManager.hpp
    include/Reputation.hpp
    include/ShopUpgrade.hpp
    include/Furniture.hpp
    include/GameTime.hpp
    include/DailyStats.hpp
    include/SaveSystem.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Copy assets folder to build directory on post-build if assets exist
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
