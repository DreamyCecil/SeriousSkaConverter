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

// Convert SE1 ASCII skeleton file (.as) into SE2+ ASCII skeleton (.asf)
extern void ConvertSkeletonSE1(CTokenList &aTokens, const CPath &strFile, Str_t strSkeleton) {
  // Go through the skeleton file
  CTokenList::reverse_iterator it;

  for (it = aTokens.rbegin(); it != aTokens.rend(); ++it)
  {
    // Skip non-keyword tokens
    if (it->GetType() != CParserToken::TKN_IDENTIFIER) {
      continue;
    }

    const Str_t str = it->GetValue().ToString();

    // Encountered variables
    if (str == "NAME" || str == "PARENT" || str == "LENGTH") {
      // Erase semicolon after the value
      CTokenList::reverse_iterator itValue = it - 1;
      strSkeleton.erase(itValue->GetTokenPos().iLast, 1);

      // If it's the last variable
      if (str == "LENGTH")
      {
        // Add "DEFAULT_POSE" keyword before the next block
        CTokenList::reverse_iterator itDefPose = it - 3; // Opening curly bracket
        
        strSkeleton.insert(itDefPose->GetTokenPos().iFirst - 1, "DEFAULT_POSE ");

        // Add generic "LIMITS" block after the length
        static const c8 *strLimits = "\n"
          "  LIMITS {\n"
          "   { 0: -3.14159, 3.14159; }\n"
          "   { 0: -3.14159, 3.14159; }\n"
          "   { 0: -3.14159, 3.14159; }\n"
          "  }";

        strSkeleton.insert(itValue->GetTokenPos().iLast, strLimits);
      }

    // Encountered version
    } else if (str == "SE_SKELETON") {
      // Get version number
      CTokenList::reverse_iterator itValue = it - 1;

      // Erase version with the semicolon
      strSkeleton.erase(itValue->GetTokenPos().iFirst, itValue->GetTokenPos().Length() + 1);

      // Write new version
      strSkeleton.insert(itValue->GetTokenPos().iFirst, "1.01");

    // Encountered ending keyword
    } else if (str == "SE_SKELETON_END") {
      // Erase keyword with the semicolon (+1) and surrounding linebreaks (+2)
      strSkeleton.erase(it->GetTokenPos().iFirst - 1, it->GetTokenPos().Length() + 3);
    }
  }

#if !_DEV_STL_IO
  CFileDevice fileD((strFile.RemoveExt() + ".asf").c_str());
  CStringStream file(&fileD, IReadWriteDevice::OM_WRITEONLY);

  file << strSkeleton;
  fileD.Close();
#else
  FileOut_t file((strFile.RemoveExt() + ".asf").c_str());
  file << strSkeleton;
  file.close();
#endif
  
  std::cout << "Successfully converted SE1 ASCII skeleton into SE2+ ASCII skeleton!\n";
};
