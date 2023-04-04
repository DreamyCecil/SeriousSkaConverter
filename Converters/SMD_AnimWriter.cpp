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

// Write SMD animation in SE1 ASCII format
extern void WriteAnimation(const SmdOptions &opts, const SmdStructure &smd) {
  // Get animation file name if needed
  Str_t strAnimation = (smd.bAnimFile ? _strFileName : "Default");
  f64 fFPS = 24.0; // Consistent 24 FPS

  // Retrieve animation info if possible
  if (opts.valAnimInfo.GetType() != CVariant::VAL_INVALID) {
    const CValObject &oInfo = opts.valAnimInfo.ToObject();

    // Animation name
    CValObject::const_iterator it = oInfo.find("name");

    if (it != oInfo.end()) {
      strAnimation = it->second.ToString();
    }

    // Animation FPS
    it = oInfo.find("fps");

    if (it != oInfo.end()) {
      fFPS = GetNumber<f64>(it->second);
    }
  }

#if !_DEV_STL_IO
  CFileDevice fileD((_strFilePath + ".aa").c_str());
  CStringStream file(&fileD, IReadWriteDevice::OM_WRITEONLY);
#else
  FileOut_t file((_strFilePath + ".aa").c_str());
#endif

  file << "SE_ANIM 0.1;\n\n";

  const s32 iWriteFrames = smd.iFrames - (smd.iFrames > 1);

  file << "SEC_PER_FRAME " << (1.0/fFPS) << ";\n"; // Seconds per one frame
  file << "FRAMES " << iWriteFrames << ";\n";
  file << "ANIM_ID \"" << strAnimation << "\";\n\n";

  // Get affected bones in the entire animation
  std::map<s32, CBoneInfo *> mapUsed;
  
  // Go through each frame (without the first one if more than 1)
  for (s32 iFrameCheck = (smd.iFrames > 1); iFrameCheck < smd.iFrames; ++iFrameCheck)
  {
    const CEnvelopes &aFrameEnvelopes = smd.aFrames[iFrameCheck].aBones;

    // Go through each bone envelope in the frame
    for (size_t iEnv = 0; iEnv < aFrameEnvelopes.size(); ++iEnv) {
      // Not used
      if (!smd.aFrames[iFrameCheck].aUsed[iEnv]) {
        continue;
      }
      
      // Add this bone to the used envelopes list
      const CBoneEnvelope &env = aFrameEnvelopes[iEnv];
      mapUsed[env.pInfo->iID] = env.pInfo;
    }
  }

  file << "BONEENVELOPES " << mapUsed.size() << "\n{\n";

  // Go through each affected envelope
  std::map<s32, CBoneInfo *>::const_iterator it;

  for (it = mapUsed.begin(); it != mapUsed.end(); ++it) {
    const s32 iBoneIndex = it->first;

    const CBoneInfo &info = *it->second;
    const CBoneEnvelope &boneDef = smd.aFrames[0].aBones[iBoneIndex];
    Mat12D mPlacement = boneDef.mConverted;

    file << "  NAME \"" << info.strName << "\"\n";

    // Default bone position
    file << "  DEFAULT_POSE { ";
    PrintPlacement(mPlacement, file);
    file << " }\n";

    // Bone envelope frames
    file << "  {";
    
    // Go through each frame (without the first one if more than 1)
    for (s32 iFrame = (smd.iFrames > 1); iFrame < smd.iFrames; ++iFrame) {
      file << "\n    ";

      // Set this frame's bone placement if it's used
      if (smd.aFrames[iFrame].aUsed[iBoneIndex]) {
        const CBoneEnvelope &boneEnv = smd.aFrames[iFrame].aBones[iBoneIndex];
        mPlacement = boneEnv.mConverted;
      }

      // Copy last placement if unused
      PrintPlacement(mPlacement, file);
    }

    file << "\n  }\n";
  }

  file << "}\n\n";

  file << "MORPHENVELOPES 0\n{\n}\n\n";
    
  file << "SE_ANIM_END;\n";

#if !_DEV_STL_IO
  fileD.Close();
#else
  file.close();
#endif
    
  std::cout << "Converted animation...\n";
};
