#include <iostream>
#include <string>
#include "diff_match_patch.h"
int main(int argc, char **argv) {
  diff_match_patch dmp;
  std::string str1 = "First string in diff";
  std::string str2 = "Second string in diff";

  auto patch = dmp.patch_toText(dmp.patch_make(str1, str2));
  auto out = dmp.patch_apply(dmp.patch_fromText(patch), str1);
  std::string strResult = out.first;

  std::cout << "First string: " << str1 << std::endl;
  std::cout << "Second string: " << str2 << std::endl;
  std::cout << "First string after patch: " << strResult << std::endl;

  // here, strResult will equal str2 above.
  return strResult != str2;
}
