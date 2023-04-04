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

// Convert SE2+ ASCII skeleton file (.asf) into SE1 ASCII skeleton (.as)
extern void ConvertSkeletonSE2(CTokenList &aTokens, const CPath &strFile, Str_t strSkeleton) {
  // Go through the skeleton file
  CTokenList::reverse_iterator it;

  for (it = aTokens.rbegin(); it != aTokens.rend(); ++it)
  {
    // Skip non-keyword tokens
    if (it->GetType() != CParserToken::TKN_IDENTIFIER) {
      continue;
    }

    const u32 iStart = it->GetTokenPos().iFirst;
    const Str_t str = it->GetValue().ToString();

    // Encountered position keyword
    if (str == "DEFAULT_POSE") {
      // Erase it
      strSkeleton.erase(iStart, 13);

    // Encountered limits keyword
    } else if (str == "LIMITS") {
      // Erase the whole block
      CTokenList::reverse_iterator itBlockEnd = it - 29;
      strSkeleton.erase(iStart, itBlockEnd->GetTokenPos().iLast - iStart + 4);

    // Encountered variables
    } else if (str == "NAME" || str == "PARENT" || str == "LENGTH") {
      // Add semicolon after the value
      CTokenList::reverse_iterator itValue = it - 1;
      strSkeleton.insert(itValue->GetTokenPos().iLast, ";");

    // Encountered version
    } else if (str == "SE_SKELETON") {
      // Get version number
      CTokenList::reverse_iterator itValue = it - 1;

      // Erase version
      strSkeleton.erase(itValue->GetTokenPos().iFirst, itValue->GetTokenPos().Length());

      // Write new version
      strSkeleton.insert(itValue->GetTokenPos().iFirst, "0.1;");
    }
  }

  strSkeleton += "\nSE_SKELETON_END;\n";

#if !_DEV_STL_IO
  CFileDevice fileD((strFile.RemoveExt() + ".as").c_str());
  CStringStream file(&fileD, IReadWriteDevice::OM_WRITEONLY);

  file << strSkeleton;
  fileD.Close();
#else  
  FileOut_t file((strFile.RemoveExt() + ".as").c_str());
  file << strSkeleton;
  file.close();
#endif
  
  std::cout << "Successfully converted SE2+ ASCII skeleton into SE1 ASCII skeleton!\n";
};
