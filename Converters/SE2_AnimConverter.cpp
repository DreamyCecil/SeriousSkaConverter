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

// Place in 3D space
struct Placement {
  Vec3D pos;
  Vec3D rot;
};

// Bone envelope structure
class CEnvelope {
  public:
    Str_t strName;
    Placement plDefault;

    std::vector<Placement> avFrames;

    CEnvelope(void) : strName("") {};
};

// Create bone envelopes from the animation file
void CreateBones(CTokenList &aTokens, std::vector<CEnvelope> &aBones) {
  CTokenList::const_iterator it;

  for (it = aTokens.begin(); it != aTokens.end(); ++it)
  {
    // Skip non-keyword tokens
    if (it->GetType() != CParserToken::TKN_IDENTIFIER) {
      continue;
    }

    // Not a bone envelope
    if (it->GetValue().ToString() != "ENVELOPE") {
      continue;
    }

    CEnvelope env;

    // Get bone name
    env.strName = (++it)->GetValue().ToString();

    it += 2; // Offset

    // Go through individual positions
    for (s32 iPos = 0; iPos < 6; ++iPos) {
      it += 6;

      f64 fValue;
      bool bNegative = false;

      // Encountered default position
      if (it->GetValue().ToString() == "DEFAULT") {
        it += 2; // Value of "DEFAULT:"

        // Skip unary operators
        if (it->GetType() == CParserToken::TKN_SUB) {
          bNegative = true;
          ++it;
        }

        // Get the value
        fValue = GetNumber<f64>(it->GetValue()) * (bNegative ? -1 : 1);

        if (iPos < 3) {
          env.plDefault.pos[iPos] = fValue;
        } else {
          env.plDefault.rot[iPos - 3] = fValue;
        }

        // Skip semicolon
        it += 2;
      }

      // Amount of frames
      ++it;
      s32 iFrames = GetNumber<s32>(it->GetValue()); // Value of "FRAMES"

      // Fill bone frames
      if (iPos == 0) {
        env.avFrames.resize(iFrames);
      }

      for (s32 iFrame = 0; iFrame < iFrames; ++iFrame) {
        it += 4; // Goes to the value of "<frame>:"
        bNegative = false;

        // Skip unary operators
        if (it->GetType() == CParserToken::TKN_SUB) {
          bNegative = true;
          ++it;
        }
            
        // Fill appropriate position with this frame's position
        fValue = GetNumber<f64>(it->GetValue()) * (bNegative ? -1 : 1);
        Placement &pl = env.avFrames[iFrame];

        if (iPos < 3) {
          pl.pos[iPos] = fValue;
        } else {
          pl.rot[iPos - 3] = fValue;
        }
      }
    }

    aBones.push_back(env);
  }
};

// Convert SE2+ ASCII animation file (.aaf) into SE1 ASCII animation (.aa)
extern void ConvertAnimationSE2(CTokenList &aTokens, const CPath &strFile) {
  // Get animation info
  Str_t strAnimName = "";

  f64 fSpeed = 0.0;
  s32 iFrames = 0;

  std::vector<CEnvelope> aBones;
  CTokenList::const_iterator it;
  
  try {
    s32 iFirstFrame = 0;
    s32 iLastFrame = 0;

    for (it = aTokens.begin(); it != aTokens.end(); ++it)
    {
      // Skip non-keyword tokens
      if (it->GetType() != CParserToken::TKN_IDENTIFIER) {
        continue;
      }

      const Str_t str = it->GetValue().ToString();

      // Anim name
      if (str == "ANIMATION_NAME") {
        strAnimName = (++it)->GetValue().ToString();
      
      // Anim speed
      } else if (str == "SEC_PER_FRAME") {
        fSpeed = GetNumber<f64>((++it)->GetValue());
      
      // First frame
      } else if (str == "FIRST_FRAME") {
        iFirstFrame = (s32)(++it)->GetValue().ToS64();

      // Last frame
      } else if (str == "LAST_FRAME") {
        iLastFrame = (s32)(++it)->GetValue().ToS64();
      }
    }

    // Count frames
    iFrames = iLastFrame - iFirstFrame + 1;

    // Create bone envelopes from the animation file
    CreateBones(aTokens, aBones);

  } catch (CBadAnyCastException &ex) {
    (void)ex;
    throw CTokenException(it->GetTokenPos(), "Wrong value type");
  }

  // Write animation info
#if !_DEV_STL_IO
  CFileDevice fileD((strFile.RemoveExt() + ".aa").c_str());
  CStringStream file(&fileD, IReadWriteDevice::OM_WRITEONLY);
#else
  FileOut_t file((strFile.RemoveExt() + ".aa").c_str());
#endif

  file << "SE_ANIM 0.1;\n\n";
  file << "SEC_PER_FRAME " << fSpeed << ";\n";
  file << "FRAMES " << iFrames << ";\n";
  file << "ANIM_ID \"" << strAnimName << "\";\n\n";

  // Envelope count
  file << "BONEENVELOPES " << aBones.size() << "\n{\n";

  // Go through each bone envelope
  std::vector<CEnvelope>::const_iterator itBone;

  for (itBone = aBones.begin(); itBone != aBones.end(); ++itBone)
  {
    QuatVecD qv;
    Mat12D m12;

    // Write envelope name
    file << "  NAME \"" << itBone->strName << "\"\n";

    // Write default position
    qv._pos = itBone->plDefault.pos;
    qv._rot.FromEuler(itBone->plDefault.rot);
    qv.ToMatrix12(m12);

    file << "  DEFAULT_POSE { ";

    for (s32 iDef = 0; iDef < 12; ++iDef) {
      file << m12(iDef / 4, iDef % 4) << (iDef == 11 ? "; " : ", ");
    }

    file << "}\n";

    // Write frame positions
    file << "  {\n";

    // Go through each frame
    std::vector<Placement>::const_iterator itFrame;
    for (itFrame = itBone->avFrames.begin(); itFrame != itBone->avFrames.end(); ++itFrame)
    {
      file << "    ";

      // Convert placement from XYZHPB to 4x3 matrix
      const Placement &pl = *itFrame;

      qv._pos = pl.pos;
      qv._rot.FromEuler(pl.rot);
      qv.ToMatrix12(m12);

      // Write frame placement
      for (s32 iPos = 0; iPos < 12; ++iPos) {
        file << m12(iPos / 4, iPos % 4) << (iPos == 11 ? ";\n" : ", ");
      }
    }

    file << "  }\n";
  }

  file << "}\n\n";

  // No envelopes
  file << "MORPHENVELOPES 0\n{\n}\n\n";

  // Close the file
  file << "SE_ANIM_END;\n";

#if !_DEV_STL_IO
  fileD.Close();
#else
  file.close();
#endif

  std::cout << "Successfully converted SE2+ ASCII animation into SE1 ASCII animation!\n";
};
