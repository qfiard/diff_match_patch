/*
 * Copyright 2008 Google Inc. All Rights Reserved.
 * Author: fraser@google.com (Neil Fraser)
 * Author: mikeslemmer@gmail.com (Mike Slemmer)
 * Author: quentinfiard@gmail.com (Quentin Fiard)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Diff Match and Patch -- Test Harness
 * http://code.google.com/p/google-diff-match-patch/
 */
#include "diff_match_patch.h"

#include "gtest/gtest.h"

class TestableDiffMatchPatch : public diff_match_patch {
 public:
  using diff_match_patch::Diff_EditCost;
  using diff_match_patch::diff_bisect;
  using diff_match_patch::diff_charsToLines;
  using diff_match_patch::diff_cleanupEfficiency;
  using diff_match_patch::diff_cleanupSemantic;
  using diff_match_patch::diff_cleanupSemanticLossless;
  using diff_match_patch::diff_commonOverlap;
  using diff_match_patch::diff_halfMatch;
  using diff_match_patch::diff_linesToChars;
  using diff_match_patch::diff_match_patch;
  using diff_match_patch::match_alphabet;
  using diff_match_patch::match_bitap;
  using diff_match_patch::patch_addContext;
};

namespace {

template <typename T>
std::wstring AsString(T value) {
  std::wstringstream ss;
  ss << value;
  return ss.str();
}

std::vector<std::wstring> diff_rebuildtexts(const std::list<Diff> &diffs) {
  std::vector<std::wstring> text = {L"", L""};
  for (const auto &myDiff : diffs) {
    if (myDiff.operation != INSERT) {
      text[0] += myDiff.text;
    }
    if (myDiff.operation != DELETE) {
      text[1] += myDiff.text;
    }
  }
  return text;
}

class DiffMatchPatchTest : public testing::Test {
  void SetUp() { dmp_.reset(new TestableDiffMatchPatch); }

 protected:
  std::unique_ptr<TestableDiffMatchPatch> dmp_;
};

//  DIFF TEST FUNCTIONS
TEST_F(DiffMatchPatchTest, DiffCommonPrefix) {
  // Detect any common prefix.
  EXPECT_EQ(0, dmp_->diff_commonPrefix(L"abc", L"xyz"))
      << "diff_commonPrefix: Null case.";

  EXPECT_EQ(4, dmp_->diff_commonPrefix(L"1234abcdef", L"1234xyz"))
      << "diff_commonPrefix: Non-null case.";

  EXPECT_EQ(4, dmp_->diff_commonPrefix(L"1234", L"1234xyz"))
      << "diff_commonPrefix: Whole case.";
}

TEST_F(DiffMatchPatchTest, DiffCommonSuffix) {
  // Detect any common suffix.
  EXPECT_EQ(0, dmp_->diff_commonSuffix(L"abc", L"xyz"))
      << "diff_commonSuffix: Null case.";

  EXPECT_EQ(4, dmp_->diff_commonSuffix(L"abcdef1234", L"xyz1234"))
      << "diff_commonSuffix: Non-null case.";

  EXPECT_EQ(4, dmp_->diff_commonSuffix(L"1234", L"xyz1234"))
      << "diff_commonSuffix: Whole case.";
}

TEST_F(DiffMatchPatchTest, DiffCommonOverlap) {
  // Detect any suffix/prefix overlap.
  EXPECT_EQ(0, dmp_->diff_commonOverlap(L"", L"abcd"))
      << "diff_commonOverlap: Null case.";

  EXPECT_EQ(3, dmp_->diff_commonOverlap(L"abc", L"abcd"))
      << "diff_commonOverlap: Whole case.";

  EXPECT_EQ(0, dmp_->diff_commonOverlap(L"123456", L"abcd"))
      << "diff_commonOverlap: No overlap.";

  EXPECT_EQ(3, dmp_->diff_commonOverlap(L"123456xxx", L"xxxabcd"))
      << "diff_commonOverlap: Overlap.";

  // Some overly clever languages (C#) may treat ligatures as equal to their
  // component letters.  E.g. U+FB01 == 'fi'
  EXPECT_EQ(0, dmp_->diff_commonOverlap(L"fi", std::wstring(2, L'\ufb01')))
      << "diff_commonOverlap : Unicode.";
}

TEST_F(DiffMatchPatchTest, DiffHalfmatch) {
  // Detect a halfmatch.
  dmp_->Diff_Timeout = 1;
  EXPECT_EQ(0, dmp_->diff_halfMatch(L"1234567890", L"abcdef").size())
      << "diff_halfMatch: No match #1.";

  EXPECT_EQ(0, dmp_->diff_halfMatch(L"12345", L"23").size())
      << "diff_halfMatch: No match #2.";

  {
    std::vector<std::wstring> expected = {L"12", L"90", L"a", L"z", L"345678"};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"1234567890", L"a345678z"))
        << "diff_halfMatch: Single Match #1.";
  }

  {
    std::vector<std::wstring> expected = {L"a", L"z", L"12", L"90", L"345678"};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"a345678z", L"1234567890"))
        << "diff_halfMatch: Single Match #2.";
  }

  {
    std::vector<std::wstring> expected = {L"abc", L"z", L"1234", L"0",
                                          L"56789"};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"abc56789z", L"1234567890"))
        << "diff_halfMatch: Single Match #3.";
  }

  {
    std::vector<std::wstring> expected = {L"a", L"xyz", L"1", L"7890",
                                          L"23456"};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"a23456xyz", L"1234567890"))
        << "diff_halfMatch: Single Match #4.";
  }

  {
    std::vector<std::wstring> expected = {L"12123", L"123121", L"a", L"z",
                                          L"1234123451234"};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"121231234123451234123121",
                                             L"a1234123451234z"))
        << "diff_halfMatch: Multiple Matches #1.";
  }

  {
    std::vector<std::wstring> expected = {L"", L"-=-=-=-=-=", L"x", L"",
                                          L"x-=-=-=-=-=-=-="};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"x-=-=-=-=-=-=-=-=-=-=-=-=",
                                             L"xx-=-=-=-=-=-=-="))
        << "diff_halfMatch: Multiple Matches #2.";
  }

  {
    std::vector<std::wstring> expected = {L"-=-=-=-=-=", L"", L"", L"y",
                                          L"-=-=-=-=-=-=-=y"};
    EXPECT_EQ(expected, dmp_->diff_halfMatch(L"-=-=-=-=-=-=-=-=-=-=-=-=y",
                                             L"-=-=-=-=-=-=-=yy"))
        << "diff_halfMatch: Multiple Matches #3.";
  }

  {
    std::vector<std::wstring> expected = {L"qHillo", L"w", L"x", L"Hulloy",
                                          L"HelloHe"};
    // Optimal diff would be -q+x=H-i+e=lloHe+Hu=llo-Hew+y not
    // -qHillo+x=HelloHe-w+Hulloy
    EXPECT_EQ(expected,
              dmp_->diff_halfMatch(L"qHilloHelloHew", L"xHelloHeHulloy"))
        << "diff_halfMatch: Non-optimal halfmatch.";
  }

  {
    std::vector<std::wstring> expected = {L"12", L"90", L"a", L"z", L"345678"};
    dmp_->Diff_Timeout = 0;
    EXPECT_EQ(0,
              dmp_->diff_halfMatch(L"qHilloHelloHew", L"xHelloHeHulloy").size())
        << "diff_halfMatch: Optimal no halfmatch.";
  }
}

TEST_F(DiffMatchPatchTest, DiffLinesToChars) {
  // Convert lines down to characters.
  std::tuple<std::wstring, std::wstring, std::vector<std::wstring> > expected =
      std::make_tuple(std::wstring() + L'\u0001' + L'\u0002' + L'\u0001',
                      std::wstring() + L'\u0002' + L'\u0001' + L'\u0002',
                      std::vector<std::wstring>({L"", L"alpha\n", L"beta\n"}));
  EXPECT_EQ(expected, dmp_->diff_linesToChars(L"alpha\nbeta\nalpha\n",
                                              L"beta\nalpha\nbeta\n"))
      << "diff_linesToChars:";

  expected = std::make_tuple(
      L"", std::wstring() + L'\u0001' + L'\u0002' + L'\u0003' + L'\u0003',
      std::vector<std::wstring>({L"", L"alpha\r\n", L"beta\r\n", L"\r\n"}));
  EXPECT_EQ(expected,
            dmp_->diff_linesToChars(L"", L"alpha\r\nbeta\r\n\r\n\r\n"))
      << "diff_linesToChars:";

  expected =
      std::make_tuple(std::wstring() + L'\u0001', std::wstring() + L'\u0002',
                      std::vector<std::wstring>({L"", L"a", L"b"}));
  EXPECT_EQ(expected, dmp_->diff_linesToChars(L"a", L"b"))
      << "diff_linesToChars:";

  // More than 256 to reveal any 8-bit limitations.
  int n = 300;
  std::wstring lines, chars;
  std::vector<std::wstring> vect;
  vect.push_back(L"");
  for (int x = 1; x < n + 1; x++) {
    vect.push_back(AsString(x) + L"\n");
    lines += AsString(x) + L"\n";
    chars += (wchar_t)x;
  }
  expected = std::make_tuple(chars, L"", vect);
  EXPECT_EQ(expected, dmp_->diff_linesToChars(lines, L""))
      << "diff_linesToChars: More than 256.";
}

TEST_F(DiffMatchPatchTest, DiffCharsToLines) {
  // First check that Diff equality works.
  EXPECT_EQ(Diff(EQUAL, L"a"), Diff(EQUAL, L"a")) << "diff_charsToLines:";

  // Convert chars up to lines.
  std::list<Diff> diffs;
  diffs.push_back(Diff(EQUAL, std::wstring() + L'\u0001' + L'\u0002' +
                                  L'\u0001'));  // ("\u0001\u0002\u0001");
  diffs.push_back(Diff(INSERT, std::wstring() + L'\u0002' + L'\u0001' +
                                   L'\u0002'));  // ("\u0002\u0001\u0002");
  dmp_->diff_charsToLines(diffs, {L"", L"alpha\n", L"beta\n"});
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"alpha\nbeta\nalpha\n"),
                             Diff(INSERT, L"beta\nalpha\nbeta\n")}),
            diffs)
      << "diff_charsToLines:";

  // More than 256 to reveal any 8-bit limitations.
  int n = 300;
  std::wstring lines, chars;
  std::vector<std::wstring> vect;
  vect.push_back(L"");
  for (int x = 1; x < n + 1; x++) {
    vect.push_back(AsString(x) + L"\n");
    lines += AsString(x) + L"\n";
    chars += (wchar_t)x;
  }
  diffs = {Diff(DELETE, chars)};
  dmp_->diff_charsToLines(diffs, vect);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, lines)}), diffs)
      << "diff_charsToLines: More than 256.";
}

TEST_F(DiffMatchPatchTest, DiffCleanupMerge) {
  // Cleanup a messy diff.
  std::list<Diff> diffs;
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>(), diffs) << "diff_cleanupMerge: Null case.";

  diffs = {Diff(EQUAL, L"a"), Diff(DELETE, L"b"), Diff(INSERT, L"c")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>(
                {Diff(EQUAL, L"a"), Diff(DELETE, L"b"), Diff(INSERT, L"c")}),
            diffs)
      << "diff_cleanupMerge: No change case.";

  diffs = {Diff(EQUAL, L"a"), Diff(EQUAL, L"b"), Diff(EQUAL, L"c")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"abc")}), diffs)
      << "diff_cleanupMerge: Merge equalities.";

  diffs = {Diff(DELETE, L"a"), Diff(DELETE, L"b"), Diff(DELETE, L"c")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abc")}), diffs)
      << "diff_cleanupMerge: Merge deletions.";

  diffs = {Diff(INSERT, L"a"), Diff(INSERT, L"b"), Diff(INSERT, L"c")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(INSERT, L"abc")}), diffs)
      << "diff_cleanupMerge: Merge insertions.";

  diffs = {Diff(DELETE, L"a"), Diff(INSERT, L"b"), Diff(DELETE, L"c"),
           Diff(INSERT, L"d"), Diff(EQUAL, L"e"),  Diff(EQUAL, L"f")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>(
                {Diff(DELETE, L"ac"), Diff(INSERT, L"bd"), Diff(EQUAL, L"ef")}),
            diffs)
      << "diff_cleanupMerge: Merge interweave.";

  diffs = {Diff(DELETE, L"a"), Diff(INSERT, L"abc"), Diff(DELETE, L"dc")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"a"), Diff(DELETE, L"d"),
                             Diff(INSERT, L"b"), Diff(EQUAL, L"c")}),
            diffs)
      << "diff_cleanupMerge: Prefix and suffix detection.";

  diffs = {Diff(EQUAL, L"x"), Diff(DELETE, L"a"), Diff(INSERT, L"abc"),
           Diff(DELETE, L"dc"), Diff(EQUAL, L"y")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"xa"), Diff(DELETE, L"d"),
                             Diff(INSERT, L"b"), Diff(EQUAL, L"cy")}),
            diffs)
      << "diff_cleanupMerge: Prefix and suffix detection with equalities.";

  diffs = {Diff(EQUAL, L"a"), Diff(INSERT, L"ba"), Diff(EQUAL, L"c")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(INSERT, L"ab"), Diff(EQUAL, L"ac")}), diffs)
      << "diff_cleanupMerge: Slide edit left.";

  diffs = {Diff(EQUAL, L"c"), Diff(INSERT, L"ab"), Diff(EQUAL, L"a")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"ca"), Diff(INSERT, L"ba")}), diffs)
      << "diff_cleanupMerge: Slide edit right.";

  diffs = {Diff(EQUAL, L"a"), Diff(DELETE, L"b"), Diff(EQUAL, L"c"),
           Diff(DELETE, L"ac"), Diff(EQUAL, L"x")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abc"), Diff(EQUAL, L"acx")}), diffs)
      << "diff_cleanupMerge: Slide edit left recursive.";

  diffs = {Diff(EQUAL, L"x"), Diff(DELETE, L"ca"), Diff(EQUAL, L"c"),
           Diff(DELETE, L"b"), Diff(EQUAL, L"a")};
  dmp_->diff_cleanupMerge(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"xca"), Diff(DELETE, L"cba")}), diffs)
      << "diff_cleanupMerge: Slide edit right recursive.";
}

TEST_F(DiffMatchPatchTest, DiffCleanupSemanticLossless) {
  // Slide diffs to match logical boundaries.
  std::list<Diff> diffs = {};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(std::list<Diff>(), diffs) << "diff_cleanupSemantic: Null case.";

  diffs = {Diff(EQUAL, L"AAA\r\n\r\nBBB"), Diff(INSERT, L"\r\nDDD\r\n\r\nBBB"),
           Diff(EQUAL, L"\r\nEEE")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"AAA\r\n\r\n"),
                             Diff(INSERT, L"BBB\r\nDDD\r\n\r\n"),
                             Diff(EQUAL, L"BBB\r\nEEE")}),
            diffs)
      << "diff_cleanupSemanticLossless: Blank lines.";

  diffs = {Diff(EQUAL, L"AAA\r\nBBB"), Diff(INSERT, L" DDD\r\nBBB"),
           Diff(EQUAL, L" EEE")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(EQUAL, L"AAA\r\n"), Diff(INSERT, L"BBB DDD\r\n"),
                       Diff(EQUAL, L"BBB EEE")}),
      diffs)
      << "diff_cleanupSemanticLossless: Line boundaries.";

  diffs = {Diff(EQUAL, L"The c"), Diff(INSERT, L"ow and the c"),
           Diff(EQUAL, L"at.")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(EQUAL, L"The "), Diff(INSERT, L"cow and the "),
                       Diff(EQUAL, L"cat.")}),
      diffs)
      << "diff_cleanupSemantic: Word boundaries.";

  diffs = {Diff(EQUAL, L"The-c"), Diff(INSERT, L"ow-and-the-c"),
           Diff(EQUAL, L"at.")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(EQUAL, L"The-"), Diff(INSERT, L"cow-and-the-"),
                       Diff(EQUAL, L"cat.")}),
      diffs)
      << "diff_cleanupSemantic: Alphanumeric boundaries.";

  diffs = {Diff(EQUAL, L"a"), Diff(DELETE, L"a"), Diff(EQUAL, L"ax")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"a"), Diff(EQUAL, L"aax")}), diffs)
      << "diff_cleanupSemantic: Hitting the start.";

  diffs = {Diff(EQUAL, L"xa"), Diff(DELETE, L"a"), Diff(EQUAL, L"a")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(EQUAL, L"xaa"), Diff(DELETE, L"a")}), diffs)
      << "diff_cleanupSemantic: Hitting the end.";

  diffs = {Diff(EQUAL, L"The xxx. The "), Diff(INSERT, L"zzz. The "),
           Diff(EQUAL, L"yyy.")};
  dmp_->diff_cleanupSemanticLossless(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(EQUAL, L"The xxx."), Diff(INSERT, L" The zzz."),
                       Diff(EQUAL, L" The yyy.")}),
      diffs)
      << "diff_cleanupSemantic: Sentence boundaries.";
}

TEST_F(DiffMatchPatchTest, DiffCleanupSemantic) {
  // Cleanup semantically trivial equalities.
  std::list<Diff> diffs = {};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>(), diffs) << "diff_cleanupSemantic: Null case.";

  diffs = {Diff(DELETE, L"ab"), Diff(INSERT, L"cd"), Diff(EQUAL, L"12"),
           Diff(DELETE, L"e")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"ab"), Diff(INSERT, L"cd"),
                             Diff(EQUAL, L"12"), Diff(DELETE, L"e")}),
            diffs)
      << "diff_cleanupSemantic: No elimination #1.";

  diffs = {Diff(DELETE, L"abc"), Diff(INSERT, L"ABC"), Diff(EQUAL, L"1234"),
           Diff(DELETE, L"wxyz")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abc"), Diff(INSERT, L"ABC"),
                             Diff(EQUAL, L"1234"), Diff(DELETE, L"wxyz")}),
            diffs)
      << "diff_cleanupSemantic: No elimination #2.";

  diffs = {Diff(DELETE, L"a"), Diff(EQUAL, L"b"), Diff(DELETE, L"c")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abc"), Diff(INSERT, L"b")}), diffs)
      << "diff_cleanupSemantic: Simple elimination.";

  diffs = {Diff(DELETE, L"ab"), Diff(EQUAL, L"cd"), Diff(DELETE, L"e"),
           Diff(EQUAL, L"f"), Diff(INSERT, L"g")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abcdef"), Diff(INSERT, L"cdfg")}),
            diffs)
      << "diff_cleanupSemantic: Backpass elimination.";

  diffs = {Diff(INSERT, L"1"), Diff(EQUAL, L"A"),  Diff(DELETE, L"B"),
           Diff(INSERT, L"2"), Diff(EQUAL, L"_"),  Diff(INSERT, L"1"),
           Diff(EQUAL, L"A"),  Diff(DELETE, L"B"), Diff(INSERT, L"2")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"AB_AB"), Diff(INSERT, L"1A2_1A2")}),
            diffs)
      << "diff_cleanupSemantic: Multiple elimination.";

  diffs = {Diff(EQUAL, L"The c"), Diff(DELETE, L"ow and the c"),
           Diff(EQUAL, L"at.")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(EQUAL, L"The "), Diff(DELETE, L"cow and the "),
                       Diff(EQUAL, L"cat.")}),
      diffs)
      << "diff_cleanupSemantic: Word boundaries.";

  diffs = {Diff(DELETE, L"abcxx"), Diff(INSERT, L"xxdef")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abcxx"), Diff(INSERT, L"xxdef")}),
            diffs)
      << "diff_cleanupSemantic: No overlap elimination.";

  diffs = {Diff(DELETE, L"abcxxx"), Diff(INSERT, L"xxxdef")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abc"), Diff(EQUAL, L"xxx"),
                             Diff(INSERT, L"def")}),
            diffs)
      << "diff_cleanupSemantic: Overlap elimination.";

  diffs = {Diff(DELETE, L"xxxabc"), Diff(INSERT, L"defxxx")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(INSERT, L"def"), Diff(EQUAL, L"xxx"),
                             Diff(DELETE, L"abc")}),
            diffs)
      << "diff_cleanupSemantic: Reverse overlap elimination.";

  diffs = {Diff(DELETE, L"abcd1212"), Diff(INSERT, L"1212efghi"),
           Diff(EQUAL, L"----"), Diff(DELETE, L"A3"), Diff(INSERT, L"3BC")};
  dmp_->diff_cleanupSemantic(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"abcd"), Diff(EQUAL, L"1212"),
                             Diff(INSERT, L"efghi"), Diff(EQUAL, L"----"),
                             Diff(DELETE, L"A"), Diff(EQUAL, L"3"),
                             Diff(INSERT, L"BC")}),
            diffs)
      << "diff_cleanupSemantic: Two overlap eliminations.";
}

TEST_F(DiffMatchPatchTest, DiffCleanupEfficiency) {
  // Cleanup operationally trivial equalities.
  dmp_->Diff_EditCost = 4;
  std::list<Diff> diffs = {};
  dmp_->diff_cleanupEfficiency(diffs);
  EXPECT_EQ(std::list<Diff>(), diffs) << "diff_cleanupEfficiency: Null case.";

  diffs = {Diff(DELETE, L"ab"), Diff(INSERT, L"12"), Diff(EQUAL, L"wxyz"),
           Diff(DELETE, L"cd"), Diff(INSERT, L"34")};
  dmp_->diff_cleanupEfficiency(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"ab"), Diff(INSERT, L"12"),
                             Diff(EQUAL, L"wxyz"), Diff(DELETE, L"cd"),
                             Diff(INSERT, L"34")}),
            diffs)
      << "diff_cleanupEfficiency: No elimination.";

  diffs = {Diff(DELETE, L"ab"), Diff(INSERT, L"12"), Diff(EQUAL, L"xyz"),
           Diff(DELETE, L"cd"), Diff(INSERT, L"34")};
  dmp_->diff_cleanupEfficiency(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(DELETE, L"abxyzcd"), Diff(INSERT, L"12xyz34")}),
      diffs)
      << "diff_cleanupEfficiency: Four-edit elimination.";

  diffs = {Diff(INSERT, L"12"), Diff(EQUAL, L"x"), Diff(DELETE, L"cd"),
           Diff(INSERT, L"34")};
  dmp_->diff_cleanupEfficiency(diffs);
  EXPECT_EQ(std::list<Diff>({Diff(DELETE, L"xcd"), Diff(INSERT, L"12x34")}),
            diffs)
      << "diff_cleanupEfficiency: Three-edit elimination.";

  diffs = {Diff(DELETE, L"ab"), Diff(INSERT, L"12"), Diff(EQUAL, L"xy"),
           Diff(INSERT, L"34"), Diff(EQUAL, L"z"),   Diff(DELETE, L"cd"),
           Diff(INSERT, L"56")};
  dmp_->diff_cleanupEfficiency(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(DELETE, L"abxyzcd"), Diff(INSERT, L"12xy34z56")}),
      diffs)
      << "diff_cleanupEfficiency: Backpass elimination.";

  dmp_->Diff_EditCost = 5;
  diffs = {Diff(DELETE, L"ab"), Diff(INSERT, L"12"), Diff(EQUAL, L"wxyz"),
           Diff(DELETE, L"cd"), Diff(INSERT, L"34")};
  dmp_->diff_cleanupEfficiency(diffs);
  EXPECT_EQ(
      std::list<Diff>({Diff(DELETE, L"abwxyzcd"), Diff(INSERT, L"12wxyz34")}),
      diffs)
      << "diff_cleanupEfficiency: High cost elimination.";
  dmp_->Diff_EditCost = 4;
}

TEST_F(DiffMatchPatchTest, DiffPrettyHtml) {
  // Pretty print.
  std::list<Diff> diffs = {Diff(EQUAL, L"a\n"), Diff(DELETE, L"<B>b</B>"),
                           Diff(INSERT, L"c&d")};
  EXPECT_EQ(
      L"<span>a&para;<br></span><del "
      L"style=\"background:#ffe6e6;\">&lt;B&gt;b&lt;/B&gt;</del><ins "
      L"style=\"background:#e6ffe6;\">c&amp;d</ins>",
      dmp_->diff_widePrettyHtml(diffs))
      << "diff_widePrettyHtml:";
}

TEST_F(DiffMatchPatchTest, DiffText) {
  // Compute the source and destination texts.
  std::list<Diff> diffs = {Diff(EQUAL, L"jump"), Diff(DELETE, L"s"),
                           Diff(INSERT, L"ed"),  Diff(EQUAL, L" over "),
                           Diff(DELETE, L"the"), Diff(INSERT, L"a"),
                           Diff(EQUAL, L" lazy")};
  EXPECT_EQ(L"jumps over the lazy", dmp_->diff_wideText1(diffs))
      << "diff_wideText1:";
  EXPECT_EQ(L"jumped over a lazy", dmp_->diff_wideText2(diffs))
      << "diff_wideText2:";
}

TEST_F(DiffMatchPatchTest, DiffDelta) {
  // Convert a diff into delta string.
  std::list<Diff> diffs = {Diff(EQUAL, L"jump"),  Diff(DELETE, L"s"),
                           Diff(INSERT, L"ed"),   Diff(EQUAL, L" over "),
                           Diff(DELETE, L"the"),  Diff(INSERT, L"a"),
                           Diff(EQUAL, L" lazy"), Diff(INSERT, L"old dog")};
  auto text1 = dmp_->diff_wideText1(diffs);
  EXPECT_EQ(L"jumps over the lazy", text1) << "diff_wideText1: Base text.";

  auto delta = dmp_->diff_toWideDelta(diffs);
  EXPECT_EQ(L"=4\t-1\t+ed\t=6\t-3\t+a\t=5\t+old dog", delta)
      << "diff_toWideDelta:";

  // Convert delta string into a diff.
  EXPECT_EQ(diffs, dmp_->diff_fromDelta(text1, delta))
      << "diff_fromDelta: Normal.";

  // Test deltas with special characters.
  diffs = {Diff(EQUAL, L'\u0680' + std::wstring(L" \000 \t %", 6)),
           Diff(DELETE, L'\u0681' + std::wstring(L" \001 \n ^", 6)),
           Diff(INSERT, L'\u0682' + std::wstring(L" \002 \\ |", 6))};
  text1 = dmp_->diff_wideText1(diffs);
  EXPECT_EQ(L'\u0680' + std::wstring(L" \000 \t %", 6) + L'\u0681' +
                std::wstring(L" \001 \n ^", 6),
            text1)
      << "diff_wideText1: Unicode text.";

  delta = dmp_->diff_toWideDelta(diffs);
  EXPECT_EQ(L"=7\t-7\t+%DA%82 %02 %5C %7C", delta)
      << "diff_toWideDelta: Unicode.";

  EXPECT_EQ(diffs, dmp_->diff_fromDelta(text1, delta))
      << "diff_fromDelta: Unicode.";

  // Verify pool of unchanged characters.
  diffs = {
      Diff(INSERT, L"A-Z a-z 0-9 - _ . ! ~ * ' ( ) ; / ? : @ & = + $ , #")};
  auto text2 = dmp_->diff_wideText2(diffs);
  EXPECT_EQ(L"A-Z a-z 0-9 - _ . ! ~ * \' ( ) ; / ? : @ & = + $ , #", text2)
      << "diff_wideText2: Unchanged characters.";

  delta = dmp_->diff_toWideDelta(diffs);
  EXPECT_EQ(L"+A-Z a-z 0-9 - _ . ! ~ * \' ( ) ; / ? : @ & = + $ , #", delta)
      << "diff_toWideDelta: Unchanged characters.";

  // Convert delta string into a diff.
  EXPECT_EQ(diffs, dmp_->diff_fromDelta(L"", delta))
      << "diff_fromDelta: Unchanged characters.";

  // Generates error (19 < 20).
  try {
    dmp_->diff_fromDelta(text1 + L"x", delta);
    EXPECT_TRUE(false) << "diff_fromDelta: Too long.";
  }
  catch (const std::wstring &e) {
    EXPECT_EQ(L"Delta size (0) smaller than source text size (15)", e)
        << "diff_fromDelta: Too long.";
  }
  catch (...) {
    EXPECT_TRUE(false) << "diff_fromDelta: Too long.";
  }

  // Generates error (19 > 18).
  try {
    dmp_->diff_fromDelta(text1.substr(1), delta);
    EXPECT_TRUE(false) << "diff_fromDelta: Too short.";
  }
  catch (const std::wstring &e) {
    EXPECT_EQ(L"Delta size (0) smaller than source text size (13)", e)
        << "diff_fromDelta: Too short.";
  }
  catch (...) {
    EXPECT_TRUE(false) << "diff_fromDelta: Too long.";
  }
}

TEST_F(DiffMatchPatchTest, DiffXIndex) {
  // Translate a location in text1 to text2.
  std::list<Diff> diffs = {Diff(DELETE, L"a"), Diff(INSERT, L"1234"),
                           Diff(EQUAL, L"xyz")};
  EXPECT_EQ(5, dmp_->diff_xIndex(diffs, 2))
      << "diff_xIndex: Translation on equality.";
  diffs = {Diff(EQUAL, L"a"), Diff(DELETE, L"1234"), Diff(EQUAL, L"xyz")};
  EXPECT_EQ(1, dmp_->diff_xIndex(diffs, 3))
      << "diff_xIndex: Translation on deletion.";
}

TEST_F(DiffMatchPatchTest, DiffLevenshtein) {
  std::list<Diff> diffs = {Diff(DELETE, L"abc"), Diff(INSERT, L"1234"),
                           Diff(EQUAL, L"xyz")};
  EXPECT_EQ(4, dmp_->diff_levenshtein(diffs))
      << "diff_levenshtein: Trailing equality.";
  diffs = {Diff(EQUAL, L"xyz"), Diff(DELETE, L"abc"), Diff(INSERT, L"1234")};
  EXPECT_EQ(4, dmp_->diff_levenshtein(diffs))
      << "diff_levenshtein: Leading equality.";
  diffs = {Diff(DELETE, L"abc"), Diff(EQUAL, L"xyz"), Diff(INSERT, L"1234")};
  EXPECT_EQ(7, dmp_->diff_levenshtein(diffs))
      << "diff_levenshtein: Middle equality.";
}

TEST_F(DiffMatchPatchTest, DiffBisect) {
  // Normal.
  auto a = L"cat";
  auto b = L"map";
  // Since the resulting diff hasn't been normalized, it would be ok if
  // the insertion and deletion pairs are swapped.
  // If the order changes, tweak this test as required.
  std::list<Diff> diffs = {Diff(DELETE, L"c"), Diff(INSERT, L"m"),
                           Diff(EQUAL, L"a"), Diff(DELETE, L"t"),
                           Diff(INSERT, L"p")};
  EXPECT_EQ(diffs, dmp_->diff_bisect(a, b, std::numeric_limits<clock_t>::max()))
      << "diff_bisect: Normal.";

  // Timeout.
  diffs = {Diff(DELETE, L"cat"), Diff(INSERT, L"map")};
  EXPECT_EQ(diffs, dmp_->diff_bisect(a, b, 0)) << "diff_bisect: Timeout.";
}

TEST_F(DiffMatchPatchTest, DiffMain) {
  // Perform a trivial diff.
  std::list<Diff> diffs = {};
  EXPECT_EQ(diffs, dmp_->diff_main(L"", L"", false)) << "diff_main: Null case.";
  diffs = {Diff(EQUAL, L"abc")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"abc", L"abc", false))
      << "diff_main: Equality.";
  diffs = {Diff(EQUAL, L"ab"), Diff(INSERT, L"123"), Diff(EQUAL, L"c")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"abc", L"ab123c", false))
      << "diff_main: Simple insertion.";
  diffs = {Diff(EQUAL, L"a"), Diff(DELETE, L"123"), Diff(EQUAL, L"bc")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"a123bc", L"abc", false))
      << "diff_main: Simple deletion.";
  diffs = {Diff(EQUAL, L"a"), Diff(INSERT, L"123"), Diff(EQUAL, L"b"),
           Diff(INSERT, L"456"), Diff(EQUAL, L"c")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"abc", L"a123b456c", false))
      << "diff_main: Two insertions.";
  diffs = {Diff(EQUAL, L"a"), Diff(DELETE, L"123"), Diff(EQUAL, L"b"),
           Diff(DELETE, L"456"), Diff(EQUAL, L"c")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"a123b456c", L"abc", false))
      << "diff_main: Two deletions.";

  // Perform a real diff.
  // Switch off the timeout.
  dmp_->Diff_Timeout = 0;
  diffs = {Diff(DELETE, L"a"), Diff(INSERT, L"b")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"a", L"b", false))
      << "diff_main: Simple case # 1. ";
  diffs = {Diff(DELETE, L"Apple"), Diff(INSERT, L"Banana"),
           Diff(EQUAL, L"s are a"), Diff(INSERT, L"lso"),
           Diff(EQUAL, L" fruit.")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"Apples are a fruit.",
                                   L"Bananas are also fruit.", false))
      << "diff_main: Simple case #2.";

  diffs = {Diff(DELETE, L"a"), Diff(INSERT, std::wstring(1, L'\u0680')),
           Diff(EQUAL, L"x"), Diff(DELETE, L"\t"),
           Diff(INSERT, std::wstring(L"\0", 1))};
  EXPECT_EQ(diffs, dmp_->diff_main(
                       L"ax\t", L'\u0680' + std::wstring(L"x\000", 2), false))
      << "diff_main: Simple case #3.";

  diffs = {Diff(DELETE, L"1"), Diff(EQUAL, L"a"),  Diff(DELETE, L"y"),
           Diff(EQUAL, L"b"),  Diff(DELETE, L"2"), Diff(INSERT, L"xab")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"1ayb2", L"abxab", false))
      << "diff_main: Overlap #1. ";
  diffs = {Diff(INSERT, L"xaxcx"), Diff(EQUAL, L"abc"), Diff(DELETE, L"y")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"abcy", L"xaxcxabc", false))
      << "diff_main: Overlap #2. ";
  diffs = {Diff(DELETE, L"ABCD"),          Diff(EQUAL, L"a"),
           Diff(DELETE, L"="),             Diff(INSERT, L"-"),
           Diff(EQUAL, L"bcd"),            Diff(DELETE, L"="),
           Diff(INSERT, L"-"),             Diff(EQUAL, L"efghijklmnopqrs"),
           Diff(DELETE, L"EFGHIJKLMNOefg")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"ABCDa=bcd=efghijklmnopqrsEFGHIJKLMNOefg",
                                   L"a-bcd-efghijklmnopqrs", false))
      << "diff_main: Overlap #3.";

  diffs = {Diff(INSERT, L" "), Diff(EQUAL, L"a"), Diff(INSERT, L"nd"),
           Diff(EQUAL, L" [[Pennsylvania]]"), Diff(DELETE, L" and [[New")};
  EXPECT_EQ(diffs, dmp_->diff_main(L"a [[Pennsylvania]] and [[New",
                                   L" and [[Pennsylvania]]", false))
      << "diff_main: Large equality.";

  dmp_->Diff_Timeout = 0.1f;  // 100ms
  // This test may 'fail' on extremely fast computers.  If so, just increase
  // the
  // text sizes.
  std::wstring a =
      L"`Twas brillig, and the slithy toves\nDid gyre and gimble in the "
      L"wabe:\nAll mimsy were the borogoves,\nAnd the mome raths "
      L"outgrabe.\n ";
  std::wstring b =
      L"I am the very model of a modern major general,\nI've "
      L"information "
      L"vegetable, animal, and mineral,\nI know the kings of England, and I "
      L"quote the fights historical,\nFrom Marathon to Waterloo, in "
      L"order "
      L"categorical.\n";
  // Increase the text sizes by 1024 times to ensure a timeout.
  for (int x = 0; x < 10; x++) {
    a = a + a;
    b = b + b;
  }
  clock_t startTime = clock();
  dmp_->diff_main(a, b);
  clock_t endTime = clock();
  // Test that we took at least the timeout period.
  EXPECT_TRUE(dmp_->Diff_Timeout * CLOCKS_PER_SEC <= endTime - startTime)
      << "diff_main: Timeout min.";
  // Test that we didn't take forever (be forgiving).
  // Theoretically this test could fail very occasionally if the
  // OS task swaps or locks up for a second at the wrong moment.
  // Java seems to overrun by ~80% (compared with 10% for other languages).
  // Therefore use an upper limit of 0.5s instead of 0.2s.
  EXPECT_TRUE(dmp_->Diff_Timeout * CLOCKS_PER_SEC * 2 > endTime - startTime)
      << "diff_main: Timeout max.";
  dmp_->Diff_Timeout = 0;

  // Test the linemode speedup.
  // Must be long to pass the 100 char cutoff.
  a =
      L"1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n"
      L"1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n"
      L"1234567890\n1234567890\n1234567890\n";
  b =
      L"abcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\n"
      L"abcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\nabcdefghij\n"
      L"abcdefghij\nabcdefghij\nabcdefghij\n";
  EXPECT_EQ(dmp_->diff_main(a, b, true), dmp_->diff_main(a, b, false))
      << "diff_main: Simple line - mode.";
  a =
      L"123456789012345678901234567890123456789012345678901234567890123456789"
      L"01"
      L"23456789012345678901234567890123456789012345678901234567890";
  b =
      L"abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghi"
      L"ja"
      L"bcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcdefghij";
  EXPECT_EQ(dmp_->diff_main(a, b, true), dmp_->diff_main(a, b, false))
      << "diff_main: Single line - mode.";
  a =
      L"1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n"
      L"1234567890\n1234567890\n1234567890\n1234567890\n1234567890\n"
      L"1234567890\n1234567890\n1234567890\n";
  b =
      L"abcdefghij\n1234567890\n1234567890\n1234567890\nabcdefghij\n"
      L"1234567890\n1234567890\n1234567890\nabcdefghij\n1234567890\n"
      L"1234567890\n1234567890\nabcdefghij\n";
  auto texts_linemode = diff_rebuildtexts(dmp_->diff_main(a, b, true));
  auto texts_textmode = diff_rebuildtexts(dmp_->diff_main(a, b, false));
  EXPECT_EQ(texts_textmode, texts_linemode)
      << "diff_main: Overlap line - mode.";
}

// //  MATCH TEST FUNCTIONS

TEST_F(DiffMatchPatchTest, MatchAlphabet) {
  // Initialise the bitmasks for Bitap.
  std::unordered_map<wchar_t, std::size_t> bitmask;
  bitmask.emplace(L'a', 4);
  bitmask.emplace(L'b', 2);
  bitmask.emplace(L'c', 1);
  EXPECT_EQ(bitmask, dmp_->match_alphabet(L"abc")) << "match_alphabet:Unique.";
  bitmask = std::unordered_map<wchar_t, std::size_t>();
  bitmask.emplace(L'a', 37);
  bitmask.emplace(L'b', 18);
  bitmask.emplace(L'c', 8);
  EXPECT_EQ(bitmask, dmp_->match_alphabet(L"abcaba"))
      << "match_alphabet: Duplicates.";
}

TEST_F(DiffMatchPatchTest, MatchBitap) {
  // Bitap algorithm.
  dmp_->Match_Distance = 100;
  dmp_->Match_Threshold = 0.5f;
  EXPECT_EQ(5, dmp_->match_bitap(L"abcdefghijk", L"fgh", 5))
      << "match_bitap: Exact match #1. ";
  EXPECT_EQ(5, dmp_->match_bitap(L"abcdefghijk", L"fgh", 0))
      << "match_bitap: Exact match #2. ";
  EXPECT_EQ(4, dmp_->match_bitap(L"abcdefghijk", L"efxhi", 0))
      << "match_bitap: Fuzzy match #1. ";
  EXPECT_EQ(2, dmp_->match_bitap(L"abcdefghijk", L"cdefxyhijk", 5))
      << "match_bitap: Fuzzy match #2.";

  EXPECT_EQ(-1, dmp_->match_bitap(L"abcdefghijk", L"bxy", 1))
      << "match_bitap: Fuzzy match #3. ";
  EXPECT_EQ(2, dmp_->match_bitap(L"123456789xx0", L"3456789x0", 2))
      << "match_bitap: Overflow.";

  EXPECT_EQ(0, dmp_->match_bitap(L"abcdef", L"xxabc", 4))
      << "match_bitap: Before start match.";
  EXPECT_EQ(3, dmp_->match_bitap(L"abcdef", L"defyy", 4))
      << "match_bitap: Beyond end match.";
  EXPECT_EQ(0, dmp_->match_bitap(L"abcdef", L"xabcdefy", 0))
      << "match_bitap: Oversized pattern.";
  dmp_->Match_Threshold = 0.4f;
  EXPECT_EQ(4, dmp_->match_bitap(L"abcdefghijk", L"efxyhi", 1))
      << "match_bitap: Threshold #1.";

  dmp_->Match_Threshold = 0.3f;
  EXPECT_EQ(-1, dmp_->match_bitap(L"abcdefghijk", L"efxyhi", 1))
      << "match_bitap: Threshold #2.";

  dmp_->Match_Threshold = 0.0f;
  EXPECT_EQ(1, dmp_->match_bitap(L"abcdefghijk", L"bcdef", 1))
      << "match_bitap: Threshold #3. ";
  dmp_->Match_Threshold = 0.5f;
  EXPECT_EQ(0, dmp_->match_bitap(L"abcdexyzabcde", L"abccde", 3))
      << "match_bitap: Multiple select #1.";

  EXPECT_EQ(8, dmp_->match_bitap(L"abcdexyzabcde", L"abccde", 5))
      << "match_bitap: Multiple select #2.";

  dmp_->Match_Distance = 10;  // Strict location.
  EXPECT_EQ(-1,
            dmp_->match_bitap(L"abcdefghijklmnopqrstuvwxyz", L"abcdefg", 24))
      << "match_bitap: Distance test #1.";

  EXPECT_EQ(0,
            dmp_->match_bitap(L"abcdefghijklmnopqrstuvwxyz", L"abcdxxefg", 1))
      << "match_bitap: Distance test #2.";

  dmp_->Match_Distance = 1000;  // Loose location.
  EXPECT_EQ(0, dmp_->match_bitap(L"abcdefghijklmnopqrstuvwxyz", L"abcdefg", 24))
      << "match_bitap: Distance test #3.";
}

TEST_F(DiffMatchPatchTest, MatchMain) {
  // Full match.
  EXPECT_EQ(0, dmp_->match_main(L"abcdef", L"abcdef", 1000))
      << "match_main: Equality.";
  EXPECT_EQ(-1, dmp_->match_main(L"", L"abcdef", 1))
      << "match_main: Null text.";
  EXPECT_EQ(3, dmp_->match_main(L"abcdef", L"", 3))
      << "match_main: Null pattern.";
  EXPECT_EQ(3, dmp_->match_main(L"abcdef", L"de", 3))
      << "match_main: Exact match.";
  dmp_->Match_Threshold = 0.7f;
  EXPECT_EQ(4,
            dmp_->match_main(L"I am the very model of a modern major general.",
                             L" that berry ", 5))
      << "match_main: Complex match.";
  dmp_->Match_Threshold = 0.5f;
}

// //  PATCH TEST FUNCTIONS

TEST_F(DiffMatchPatchTest, PatchObj) {
  // Patch Object.
  Patch p;
  p.start1 = 20;
  p.start2 = 21;
  p.size1 = 18;
  p.size2 = 17;
  p.diffs = {Diff(EQUAL, L"jump"),   Diff(DELETE, L"s"),   Diff(INSERT, L"ed"),
             Diff(EQUAL, L" over "), Diff(DELETE, L"the"), Diff(INSERT, L"a"),
             Diff(EQUAL, L"\nlaz")};

  EXPECT_EQ(
      L"@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n %0Alaz\n",
      p.toString())
      << "Patch: toString.";
}

TEST_F(DiffMatchPatchTest, PatchFromText) {
  EXPECT_TRUE(dmp_->patch_fromText(L"").empty()) << "patch_fromText: #0.";

  auto strp =
      L"@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n %0Alaz\n";
  EXPECT_EQ(strp, dmp_->patch_fromText(strp).front().toString())
      << "patch_fromText: #1.";

  strp = L"@@ -1 +1 @@\n-a\n+b\n";
  EXPECT_EQ(strp, dmp_->patch_fromText(strp).front().toString())
      << "patch_fromText: #2.";

  strp = L"@@ -1,3 +0,0 @@\n-abc\n";
  EXPECT_EQ(strp, dmp_->patch_fromText(strp).front().toString())
      << "patch_fromText: #3.";

  strp = L"@@ -0,0 +1,3 @@\n+abc\n";
  EXPECT_EQ(strp, dmp_->patch_fromText(strp).front().toString())
      << "patch_fromText: #4.";

  try {
    dmp_->patch_fromText(L"Bad\nPatch\n");
    EXPECT_TRUE(false) << "patch_fromText: #5.";
  }
  catch (const std::wstring &e) {
    EXPECT_EQ(L"Invalid patch string: Bad", e) << "patch_fromText: #5.";
  }
  catch (...) {
    EXPECT_TRUE(false) << "patch_fromText: #5.";
  }
}

TEST_F(DiffMatchPatchTest, PatchToText) {
  auto strp =
      L"@@ -21,18 +22,17 @@\n jump\n-s\n+ed\n  over \n-the\n+a\n  laz\n";
  auto patches = dmp_->patch_fromText(strp);
  EXPECT_EQ(strp, dmp_->patch_toWideText(patches))
      << "patch_toWideText: Single";

  strp =
      L"@@ -1,9 +1,9 @@\n-f\n+F\n oo+fooba\n@@ -7,9 +7,9 @@\n obar\n-,\n+.\n "
      L" tes\n";
  patches = dmp_->patch_fromText(strp);
  EXPECT_EQ(strp, dmp_->patch_toWideText(patches)) << "patch_toWideText: Dual";
}

TEST_F(DiffMatchPatchTest, PatchAddContext) {
  dmp_->Patch_Margin = 4;
  Patch p;
  p = dmp_->patch_fromText(L"@@ -21,4 +21,10 @@\n-jump\n+somersault\n").front();
  dmp_->patch_addContext(p, L"The quick brown fox jumps over the lazy dog.");
  EXPECT_EQ(L"@@ -17,12 +17,18 @@\n fox \n-jump\n+somersault\n s ov\n",
            p.toString())
      << "patch_addContext: Simple case.";

  p = dmp_->patch_fromText(L"@@ -21,4 +21,10 @@\n-jump\n+somersault\n").front();
  dmp_->patch_addContext(p, L"The quick brown fox jumps.");
  EXPECT_EQ(L"@@ -17,10 +17,16 @@\n fox \n-jump\n+somersault\n s.\n",
            p.toString())
      << "patch_addContext: Not enough trailing context.";

  p = dmp_->patch_fromText(L"@@ -3 +3,2 @@\n-e\n+at\n").front();
  dmp_->patch_addContext(p, L"The quick brown fox jumps.");
  EXPECT_EQ(L"@@ -1,7 +1,8 @@\n Th\n-e\n+at\n  qui\n", p.toString())
      << "patch_addContext: Not enough leading context.";

  p = dmp_->patch_fromText(L"@@ -3 +3,2 @@\n-e\n+at\n").front();
  dmp_->patch_addContext(
      p, L"The quick brown fox jumps.  The quick brown fox crashes.");
  EXPECT_EQ(L"@@ -1,27 +1,28 @@\n Th\n-e\n+at\n  quick brown fox jumps. \n",
            p.toString())
      << "patch_addContext: Ambiguity.";
}

TEST_F(DiffMatchPatchTest, PatchMake) {
  auto patches = dmp_->patch_make(L"", L"");
  EXPECT_EQ(L"", dmp_->patch_toWideText(patches)) << "patch_make: Null case";

  std::wstring text1 = L"The quick brown fox jumps over the lazy dog.";
  std::wstring text2 = L"That quick brown fox jumped over a lazy dog.";
  auto expectedPatch =
      L"@@ -1,8 +1,7 @@\n Th\n-at\n+e\n  qui\n@@ -21,17 +21,18 @@\n "
      L"jump\n-ed\n+s\n  over \n-a\n+the\n  laz\n";
  // The second patch must be "-21,17 +21,18", not "-22,17 +21,18" due to
  // rolling context.
  patches = dmp_->patch_make(text2, text1);
  EXPECT_EQ(expectedPatch, dmp_->patch_toWideText(patches))
      << "patch_make: Text2+Text1 inputs ";
  expectedPatch =
      L"@@ -1,11 +1,12 @@\n Th\n-e\n+at\n  quick b\n@@ -22,18 +22,17 @@\n "
      L"jump\n-s\n+ed\n  over \n-the\n+a\n  laz\n";
  patches = dmp_->patch_make(text1, text2);
  EXPECT_EQ(expectedPatch, dmp_->patch_toWideText(patches))
      << "patch_make: Text1+Text2 inputs ";
  auto diffs = dmp_->diff_main(text1, text2, false);
  patches = dmp_->patch_make(diffs);
  EXPECT_EQ(expectedPatch, dmp_->patch_toWideText(patches))
      << "patch_make: Diff input";

  patches = dmp_->patch_make(text1, diffs);
  EXPECT_EQ(expectedPatch, dmp_->patch_toWideText(patches))
      << "patch_make: Text1+Diff inputs ";
  patches = dmp_->patch_make(text1, text2, diffs);
  EXPECT_EQ(expectedPatch, dmp_->patch_toWideText(patches))
      << "patch_make: Text1 + Text2 + Diff inputs(deprecated) ";
  patches =
      dmp_->patch_make(L"`1234567890-=[]\\;',./", L"~!@#$%^&*()_+{}|:\"<>?");
  EXPECT_EQ(
      L"@@ -1,21 +1,21 "
      L"@@\n-%601234567890-=%5B%5D%5C;',./"
      L"\n+~!@#$%25%5E&*()_+%7B%7D%7C:%22%3C%3E?\n",
      dmp_->patch_toWideText(patches))
      << "patch_toWideText: Character encoding.";
  diffs = {Diff(DELETE, L"`1234567890-=[]\\;',./"),
           Diff(INSERT, L"~!@#$%^&*()_+{}|:\"<>?")};
  EXPECT_EQ(diffs, dmp_->patch_fromText(
                             L"@@ -1,21 +1,21 "
                             L"@@\n-%601234567890-=%5B%5D%5C;',./"
                             L"\n+~!@#$%25%5E&*()_+%7B%7D%7C:%22%3C%3E?\n")
                       .front()
                       .diffs)
      << "patch_fromText: Character decoding.";

  text1 = L"";
  for (int x = 0; x < 100; x++) {
    text1 += L"abcdef";
  }
  text2 = text1 + L"123";
  expectedPatch =
      L"@@ -573,28 +573,31 @@\n cdefabcdefabcdefabcdefabcdef\n+123\n";
  patches = dmp_->patch_make(text1, text2);
  EXPECT_EQ(expectedPatch, dmp_->patch_toWideText(patches))
      << "patch_make: Long string with repeats.";
}

TEST_F(DiffMatchPatchTest, PatchSplitMax) {
  // Assumes that Match_MaxBits is 32.
  auto patches = dmp_->patch_make(
      L"abcdefghijklmnopqrstuvwxyz01234567890",
      L"XabXcdXefXghXijXklXmnXopXqrXstXuvXwxXyzX01X23X45X67X89X0");
  dmp_->patch_splitMax(patches);
  EXPECT_EQ(
      L"@@ -1,32 +1,46 @@\n+X\n ab\n+X\n cd\n+X\n ef\n+X\n gh\n+X\n ij\n+X\n "
      L"kl\n+X\n mn\n+X\n op\n+X\n qr\n+X\n st\n+X\n uv\n+X\n wx\n+X\n "
      L"yz\n+X\n 012345\n@@ -25,13 +39,18 @@\n zX01\n+X\n 23\n+X\n 45\n+X\n "
      L"67\n+X\n 89\n+X\n 0\n",
      dmp_->patch_toWideText(patches))
      << "patch_splitMax: #1.";

  patches = dmp_->patch_make(
      L"abcdef123456789012345678901234567890123456789012345678901234567890123"
      L"45"
      L"67890uvwxyz",
      L"abcdefuvwxyz");
  auto oldToText = dmp_->patch_toWideText(patches);
  dmp_->patch_splitMax(patches);
  EXPECT_EQ(oldToText, dmp_->patch_toWideText(patches))
      << "patch_splitMax: # 2. ";
  patches = dmp_->patch_make(
      L"123456789012345678901234567890123456789012345678901234567890123456789"
      L"0",
      L"abc");
  dmp_->patch_splitMax(patches);
  EXPECT_EQ(
      L"@@ -1,32 +1,4 @@\n-1234567890123456789012345678\n 9012\n@@ -29,32 "
      L"+1,4 "
      L"@@\n-9012345678901234567890123456\n 7890\n@@ -57,14 +1,3 "
      L"@@\n-78901234567890\n+abc\n",
      dmp_->patch_toWideText(patches))
      << "patch_splitMax: #3.";

  patches = dmp_->patch_make(
      L"abcdefghij , h : 0 , t : 1 abcdefghij , h : 0 , t : 1 abcdefghij , h "
      L": "
      L"0 , t : 1",
      L"abcdefghij , h : 1 , t : 1 abcdefghij , h : 1 , t : 1 abcdefghij , h "
      L": "
      L"0 , t : 1");
  dmp_->patch_splitMax(patches);
  EXPECT_EQ(
      L"@@ -2,32 +2,32 @@\n bcdefghij , h : \n-0\n+1\n  , t : 1 abcdef\n@@ "
      L"-29,32 +29,32 @@\n bcdefghij , h : \n-0\n+1\n  , t : 1 abcdef\n",
      dmp_->patch_toWideText(patches))
      << "patch_splitMax: #4.";
}

TEST_F(DiffMatchPatchTest, PatchAddPadding) {
  auto patches = dmp_->patch_make(L"", L"test");
  EXPECT_EQ(L"@@ -0,0 +1,4 @@\n+test\n", dmp_->patch_toWideText(patches))
      << "patch_addPadding: Both edges full.";
  dmp_->patch_addPadding(patches);
  EXPECT_EQ(L"@@ -1,8 +1,12 @@\n %01%02%03%04\n+test\n %01%02%03%04\n",
            dmp_->patch_toWideText(patches))
      << "patch_addPadding: Both edges full.";
  patches = dmp_->patch_make(L"XY", L"XtestY");
  EXPECT_EQ(L"@@ -1,2 +1,6 @@\n X\n+test\n Y\n",
            dmp_->patch_toWideText(patches))
      << "patch_addPadding: Both edges partial.";
  dmp_->patch_addPadding(patches);
  EXPECT_EQ(L"@@ -2,8 +2,12 @@\n %02%03%04X\n+test\n Y%01%02%03\n",
            dmp_->patch_toWideText(patches))
      << "patch_addPadding: Both edges partial.";
  patches = dmp_->patch_make(L"XXXXYYYY", L"XXXXtestYYYY");
  EXPECT_EQ(L"@@ -1,8 +1,12 @@\n XXXX\n+test\n YYYY\n",
            dmp_->patch_toWideText(patches))
      << "patch_addPadding: Both edges none.";
  dmp_->patch_addPadding(patches);
  EXPECT_EQ(L"@@ -5,8 +5,12 @@\n XXXX\n+test\n YYYY\n",
            dmp_->patch_toWideText(patches))
      << "patch_addPadding: Both edges none.";
}

TEST_F(DiffMatchPatchTest, PatchApply) {
  dmp_->Match_Distance = 1000;
  dmp_->Match_Threshold = 0.5f;
  dmp_->Patch_DeleteThreshold = 0.5f;
  auto patches = dmp_->patch_make(L"", L"");
  auto results = dmp_->patch_apply(patches, L"Hello world.");
  auto boolArray = results.second;

  auto resultStr = results.first + L"\t" + AsString(boolArray.size());
  EXPECT_EQ(L"Hello world.\t0", resultStr) << "patch_apply: Null case.";

  patches = dmp_->patch_make(L"The quick brown fox jumps over the lazy dog.",
                             L"That quick brown fox jumped over a lazy dog.");
  results = dmp_->patch_apply(patches,
                              L"The quick brown fox jumps over the lazy dog.");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(L"That quick brown fox jumped over a lazy dog.\ttrue\ttrue",
            resultStr)
      << "patch_apply: Exact match.";

  results = dmp_->patch_apply(
      patches, L"The quick red rabbit jumps over the tired tiger.");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(L"That quick red rabbit jumped over a tired tiger.\ttrue\ttrue",
            resultStr)
      << "patch_apply: Partial match.";

  results = dmp_->patch_apply(
      patches, L"I am the very model of a modern major general.");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(L"I am the very model of a modern major general.\tfalse\tfalse",
            resultStr)
      << "patch_apply: Failed match.";

  patches = dmp_->patch_make(
      L"x12345678901234567890123456789012345678901234567890123456789012345678"
      L"90"
      L"y",
      L"xabcy");
  results =
      dmp_->patch_apply(patches,
                        L"x123456789012345678901234567890-----++++++++++----"
                        L"-123456789012345678901234567890y");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(L"xabcy\ttrue\ttrue", resultStr)
      << "patch_apply: Big delete, small change.";

  patches = dmp_->patch_make(
      L"x12345678901234567890123456789012345678901234567890123456789012345678"
      L"90"
      L"y",
      L"xabcy");
  results =
      dmp_->patch_apply(patches,
                        L"x12345678901234567890---------------++++++++++----"
                        L"-----------12345678901234567890y");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(
      L"xabc12345678901234567890---------------++++++++++--------------"
      L"-12345678901234567890y\tfalse\ttrue",
      resultStr)
      << "patch_apply: Big delete, large change 1.";
  dmp_->Patch_DeleteThreshold = 0.6f;
  patches = dmp_->patch_make(
      L"x12345678901234567890123456789012345678901234567890123456789012345678"
      L"90"
      L"y",
      L"xabcy");
  results =
      dmp_->patch_apply(patches,
                        L"x12345678901234567890---------------++++++++++----"
                        L"-----------12345678901234567890y");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(L"xabcy\ttrue\ttrue", resultStr)
      << "patch_apply: Big delete, large change 2.";
  dmp_->Patch_DeleteThreshold = 0.5f;

  dmp_->Match_Threshold = 0.0f;
  dmp_->Match_Distance = 0;
  patches = dmp_->patch_make(
      L"abcdefghijklmnopqrstuvwxyz--------------------1234567890",
      L"abcXXXXXXXXXXdefghijklmnopqrstuvwxyz--------------------"
      L"1234567YYYYYYYYYY890");
  results = dmp_->patch_apply(
      patches, L"ABCDEFGHIJKLMNOPQRSTUVWXYZ--------------------1234567890");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false") +
              L"\t" + (boolArray[1] ? L"true" : L"false");
  EXPECT_EQ(
      L"ABCDEFGHIJKLMNOPQRSTUVWXYZ--------------------"
      L"1234567YYYYYYYYYY890\tfalse\ttrue",
      resultStr)
      << "patch_apply: Compensate for failed patch.";
  dmp_->Match_Threshold = 0.5f;
  dmp_->Match_Distance = 1000;

  patches = dmp_->patch_make(L"", L"test");
  auto patchStr = dmp_->patch_toWideText(patches);
  dmp_->patch_apply(patches, L"");
  EXPECT_EQ(patchStr, dmp_->patch_toWideText(patches))
      << "patch_apply: No side effects.";

  patches = dmp_->patch_make(L"The quick brown fox jumps over the lazy dog.",
                             L"Woof");
  patchStr = dmp_->patch_toWideText(patches);
  dmp_->patch_apply(patches, L"The quick brown fox jumps over the lazy dog.");
  EXPECT_EQ(patchStr, dmp_->patch_toWideText(patches))
      << "patch_apply: No side effects with major delete.";

  patches = dmp_->patch_make(L"", L"test");
  results = dmp_->patch_apply(patches, L"");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false");
  EXPECT_EQ(L"test\ttrue", resultStr) << "patch_apply: Edge exact match.";

  patches = dmp_->patch_make(L"XY", L"XtestY");
  results = dmp_->patch_apply(patches, L"XY");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false");
  EXPECT_EQ(L"XtestY\ttrue", resultStr)
      << "patch_apply: Near edge exact match.";

  patches = dmp_->patch_make(L"y", L"y123");
  results = dmp_->patch_apply(patches, L"x");
  boolArray = results.second;
  resultStr = results.first + L"\t" + (boolArray[0] ? L"true" : L"false");
  EXPECT_EQ(L"x123\ttrue", resultStr) << "patch_apply: Edge partial match.";
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
