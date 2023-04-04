/* Copyright (c) 2023 Dreamy Cecil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Main.h"

// Entry point
int main(int iArgs, c8 *astrArgs[]) {
  // Display opened file
  if (iArgs > 1) {
    std::cout << astrArgs[1] << "\n\n";
  }

  // Get program arguments
  Strings_t aArguments;

  for (s32 iArg = 2; iArg < iArgs; ++iArg) {
    aArguments.push_back(astrArgs[iArg]);
  }

  if (iArgs < 2) {
    return 0;
  }

  // Read animation file
  CPath strFile = astrArgs[1];
  Str_t strASCII = ReadTextFile(strFile);

  // Declare converters
  extern void ConvertAnimationSE2(CTokenList &aTokens, const CPath &strFile);
  extern void ConvertSkeletonSE2(CTokenList &aTokens, const CPath &strFile, Str_t strSkeleton);
  extern void ConvertSkeletonSE1(CTokenList &aTokens, const CPath &strFile, Str_t strSkeleton);
  extern void ConvertSourceMesh(const CPath &strFile, bool bVtxAnimation, Strings_t &aArguments);

  try {
    Str_t strExt = strFile.GetFileExt();
    CTokenList aTokens;

    // Invalid format
    if (strExt == "") {
      throw CMessageException("Unknown file extension");
    }

    // SE2+ ASCII animation
    if (strExt == ".aaf") {
      // Tokenize ASCII file
      TokenizeString(aTokens, strASCII);

      // Convert animation
      ConvertAnimationSE2(aTokens, strFile);

    // SE2+ ASCII skeleton
    } else if (strExt == ".asf") {
      // Tokenize ASCII file
      TokenizeString(aTokens, strASCII);

      // Convert skeleton
      ConvertSkeletonSE2(aTokens, strFile, strASCII);

    // SE1 ASCII skeleton
    } else if (strExt == ".as") {
      // Tokenize ASCII file
      TokenizeString(aTokens, strASCII);

      // Convert skeleton
      ConvertSkeletonSE1(aTokens, strFile, strASCII);

    // Valve SMD model
    } else if (strExt == ".smd") {
      ConvertSourceMesh(strFile, false, aArguments);

    // Valve SMD vertex animation
    } else if (strExt == ".vta") {
      ConvertSourceMesh(strFile, true, aArguments);
    
    // Invalid format
    } else {
      CMessageException::Throw("Unrecognized file format (%s)", strExt.c_str());
    }

  } catch (CException &ex) {
    std::cout << "Error: " << ex.What() << '\n';
  }

  getchar();
  return 0;
};
