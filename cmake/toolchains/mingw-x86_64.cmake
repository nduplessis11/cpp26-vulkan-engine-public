set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(MINGW_TRIPLE x86_64-w64-mingw32)

set(CMAKE_C_COMPILER   ${MINGW_TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${MINGW_TRIPLE}-g++)
set(CMAKE_RC_COMPILER  ${MINGW_TRIPLE}-windres)

# The cross toolchain installs Windows headers and libraries under /usr/local.
set(CMAKE_FIND_ROOT_PATH
    /usr/local/${MINGW_TRIPLE}
    /usr/${MINGW_TRIPLE}
)

# Find build tools on Linux, and libraries, headers, and packages in the Windows sysroot.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
