# Platform strategy

## Decision

- Linux primary: Wayland with xdg-shell.
- Windows: native Win32.
- Linux compatibility: X11 through XCB after the core Wayland shell.
- Backend selection: once both Linux backends exist, attempt actual Wayland connection first and XCB second rather than trusting environment variables alone.

Wayland first prevents X11 assumptions about coordinates, window control, input, and presentation from becoming architecture. XCB remains useful for compatibility.

Native window objects stay inside backends. The renderer receives only the narrow information required to create `VkSurfaceKHR`. Window size, scale, focus, close, and input are normalized. No SDL or GLFW dependency is used in the default path.
