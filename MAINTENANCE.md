# Maintenance

## Updating the WDL

If an update of the WDL must be made, to access the newer EEL and LICE features,
make sure to follow these steps.

- Check the revision of ysfx's reference WDL. This information should be present
  at all times the commit log of the WDL subdirectory.
  Run `git log thirdparty/WDL` and the commit number must be indicated in the
  comment of the latest commit which updated the WDL.
  For example: `Update to WDL 862717c from upstream`

- Go into the fresh working copy of WDL and review the changes from upstream.
  Extract a diff to view the list of changed files.
  For example: `git log 862717c..HEAD ; git diff 862717c..HEAD`

- Apply the diff from WDL over into ysfx's `thirdparty/WDL`.
  Patching is preferred over copying, because the latter may undo ysfx's
  custom edits.
  Do not add new files, unless these are newly introduced dependencies.

- If the specific file `WDL/eel2/asm-nseel-x64-sse.asm` has changed, you must
  review changed assembly code and translate it to GNU-compatible assembly
  in `sources/eel2-gas/sources/asm-nseel-x64-sse.S`.
  Update `sources/eel2-gas/sources/ref-hash-sha512.txt` with the hash of the
  `.asm` file that these changes originate from.
  Recompile these assemblies with NASM and GAS and check that their
  disassemblies match.

- If the specific file `thirdparty/WDL/source/WDL/eel2/eel_lice.h` has changed,
  copy relevant portions of code to `sources/ysfx_api_gfx_lice.hpp`.
  Only part of the original file is reused, so there might be nothing to copy.

- Commit these changes, and mention the new reference commit of the WDL.
  The headline can be formatted like this:
  `Update to WDL XXXXXXX from upstream`
