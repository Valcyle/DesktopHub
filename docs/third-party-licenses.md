# Third-party licenses

This file records third-party components used by Desktop Hub and the obligations that apply when distributing the application.

## Qt 6.12.0

- License: **LGPLv3** (open-source edition)
- Usage: dynamically linked
- Modules used:
  - Qt Core
  - Qt Gui
  - Qt Qml
  - Qt Quick

### Obligations

- Distribute the full text of the LGPLv3 and the Qt copyright notices with the application.
- Keep Qt dynamically linked so users can replace the Qt libraries with a compatible version.
- Provide the corresponding source code for the Qt version used, or a written offer to provide it, as required by the LGPLv3.
- Do not statically link Qt or add restrictions that prevent users from relinking.

The license text will be included in the distribution package during Phase 5. This document must be updated whenever a Qt module is added or removed.

## MinGW-w64 runtime

- Component: GCC 13.1.0 runtime libraries (`libgcc`, `libstdc++`, `libwinpthread`)
- License: GPL with the GCC Runtime Library Exception
- Usage: runtime DLLs deployed next to the executable

## Future dependencies

Before adding a dependency:

1. Confirm that it is actually necessary.
2. Check its license compatibility with LGPLv3 distribution.
3. Record the name, version, license, and purpose in this file.

No other third-party libraries are used at this time.
