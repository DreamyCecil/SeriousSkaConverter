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
#include "SMD_Structures.h"

// Write SMD skeleton in SE1 ASCII format
extern void WriteSkeleton(const SmdOptions &opts, const SmdStructure &smd) {
  // Don't make skeletons out of animations
  if (smd.bAnimFile) {
    return;
  }

  // Take the first frame for the entire skeleton
  const CEnvelopes &aEnvelopes = smd.aFrames[0].aBones;

  // Mismatching bone count
  if (aEnvelopes.size() != (size_t)smd.iBones) {
    CMessageException::Throw("Expected to have %d bones in the first frame but got %d", smd.iBones, aEnvelopes.size());
  }
  
#if !_DEV_STL_IO
  CFileDevice fileD((_strFilePath + ".as").c_str());
  CStringStream file(&fileD, IReadWriteDevice::OM_WRITEONLY);
#else
  FileOut_t file((_strFilePath + ".as").c_str());
#endif

  file << "SE_SKELETON 0.1;\n\n";

  file << "BONES " << smd.iBones << "\n{\n";

  // Write each bone
  for (s32 iBone = 0; iBone < smd.iBones; ++iBone) {
    const CBoneEnvelope &bone = aEnvelopes[iBone];
    CBoneInfo &info = *bone.pInfo;

    // Bone name
    file << "  NAME \"" << info.strName << "\";\n";

    // Parent bone name
    file << "  PARENT ";
    
    // Also calculate bone length relative to the parent
    f64 fLength = 8.0 * opts.fScale;

    if (info.iParent != -1) {
      const CBoneEnvelope &boneParent = aEnvelopes[info.iParent];

      file << "\"" << boneParent.pInfo->strName << "\";\n";

      //fLength = (boneParent.vPos - bone.vPos).Length();

    } else {
      file << "\"\";\n";
    }

    // Bone length
    file << "  LENGTH " << fLength << ";\n";

    file << "  {\n    ";
    PrintPlacement(bone.mConverted, file);
    file << "\n  }\n";
  }

  file << "}\n\n";

  file << "SE_SKELETON_END;\n";

#if !_DEV_STL_IO
  fileD.Close();
#else
  file.close();
#endif
    
  std::cout << "Converted skeleton...\n";
};
