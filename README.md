diff_match_patch
================

The Diff Match and Patch libraries offer robust algorithms to perform the operations required for synchronizing plain text.

### Demos

- Diff:
Compare two blocks of plain text and efficiently return a list of differences.  
[Diff Demo](http://neil.fraser.name/software/diff_match_patch/svn/trunk/demos/demo_diff.html)
- Match:
Given a search string, find its best fuzzy match in a block of plain text. Weighted for both accuracy and location.
[Match Demo](http://neil.fraser.name/software/diff_match_patch/svn/trunk/demos/demo_match.html)
- Patch:
Apply a list of patches onto plain text. Use best-effort to apply patch even when the underlying text doesn't match.
[Patch Demo](http://neil.fraser.name/software/diff_match_patch/svn/trunk/demos/demo_patch.html)


---

**Note: This project is a port of [google-diff-match-patch](http://code.google.com/p/google-diff-match-patch) to standard C++11, based on the [Qt/C++ port](http://code.google.com/p/google-diff-match-patch/source/browse/trunk/cpp/) by Mike Slemmer.**

---

### Dependencies
- None for general use :-) (besides C++11).
- (optional) [googletest](https://code.google.com/p/googletest) to run the unit tests.


---

### Overview

Diff, Match and Patch Library  
http://code.google.com/p/google-diff-match-patch/  
Neil Fraser

This library is currently available in seven different ports, all using the same API.
Every version includes a full set of unit tests.

C++:

- Mike Slemmer's port (requires the Qt library).
- This project.

C#:

- Ported by Matthaeus G. Chajdas.

Dart:

- The Dart language is still growing and evolving, so this port is only as
  stable as the underlying language.

Java:

- Included is both the source and a Maven package.

JavaScript:

- diff_match_patch_uncompressed.js is the human-readable version.
  Users of node.js should 'require' this uncompressed version since the
  compressed version is not guaranteed to work outside of a web browser.
- diff_match_patch.js has been compressed using Google's internal JavaScript compressor.
  Non-Google hackers who wish to recompress the source can use:
  http://dean.edwards.name/packer/

Lua:

- Ported by Duncan Cross.
- Does not support line-mode speedup.

Objective C:

- Ported by Jan Weiss.
- Includes speed test (this is a separate bundle for other languages).

Python:

- Two versions, one for Python 2.x, the other for Python 3.x.
- Runs 10x faster under PyPy than CPython.
