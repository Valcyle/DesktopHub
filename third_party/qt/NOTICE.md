# Qt license notices

The license texts in this directory apply to **Qt 6**, which Desktop Hub uses
under the LGPLv3. They do **not** define the license of Desktop Hub itself.

Desktop Hub's own source-code license has not been chosen yet. See the
"License" section in the repository README. The product name and logo are also
separate from the Qt license.

## Why these files are here

Desktop Hub dynamically links the Qt libraries and ships the Qt runtime DLLs in
its Windows package. The LGPLv3 requires that the license texts and copyright
notices travel with the distribution and that users can replace the Qt
libraries with compatible versions.

- `LGPL-3.0.txt` — GNU Lesser General Public License v3.0
- `GPL-3.0.txt` — GNU General Public License v3.0 (referenced by the LGPLv3)

The application does not statically link Qt and does not add restrictions that
would prevent relinking.
