set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(MINGW_TRIPLE x86_64-w64-mingw32)

set(CMAKE_C_COMPILER   ${MINGW_TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${MINGW_TRIPLE}-g++)
set(CMAKE_RC_COMPILER  ${MINGW_TRIPLE}-windres)

# The published cross image installs its Windows sysroot under /usr/local.
set(CMAKE_FIND_ROOT_PATH
    /usr/local/${MINGW_TRIPLE}
    /usr/${MINGW_TRIPLE}
)

# Build tools such as git and cmake run on the Linux host inside the
# container. Libraries, headers and CMake packages must come from the
# Windows target sysroot.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
