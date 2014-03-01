#include <iostream>
#include <string>
#include "diff_match_patch.h"
int main(int argc, char **argv) {
  diff_match_patch dmp;
  std::wstring str1 = L"First string in diff";
  std::wstring str2 = L"Second string in diff";

  auto patch = dmp.patch_toText(dmp.patch_make(str1, str2));
  auto out = dmp.patch_apply(dmp.patch_fromText(patch), str1);
  std::wstring strResult = out.first;

  std::wcout << "First string: " << str1 << std::endl;
  std::wcout << "Second string: " << str2 << std::endl;
  std::wcout << "First string after patch: " << strResult << std::endl;

  // here, strResult will equal str2 above.
  return strResult != str2;
}
