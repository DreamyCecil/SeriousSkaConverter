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

// Tokenize SMD file contents
static void TokenizeSMD(CTokenList &aTokens, const Str_t &str, bool bVtxAnimation) {
  CParserData data(str);
    
  while (data.CanParse()) {
    switch (*data.pchCur) {
      // Skip spaces
      case ' ': case '\t': case '\r': break;
      
      // Line break
      case '\n': data.CountLine(); break;

      // End-line comments
      case '#': case ';': {
        // Skip until the line break
        while (!data.AtEnd()) {
          data.SetToCurrent();

          if (*data.pchCur == '\r'
           || *data.pchCur == '\n') {
            break;
          }

          data.Advance(1);
        }
      } break;

      default: {
        // Special tokenizers
        bool bTokenized = data.ParseComments(aTokens, false)
          || data.ParseIdentifiers(aTokens)
          || data.ParseOperators(aTokens)
          || data.ParseNumbers(aTokens)
          || data.ParseCharSequences(aTokens, '"', '\'');

        if (!bTokenized) {
          throw CTokenException(data.pos, "Invalid character for tokenization");
        }
      }
    }
  }
};

// Current SMD file
Str_t _strFilePath = "";
Str_t _strFileName = "";

#define ANIM_BASE_SMD Str_t("!Base.smd")
#define BASE_SMD_ARGS Str_t("!Converter.txt")
#define ANIM_INFOS Str_t("!AnimInfo.json")

extern void ConvertSourceMesh(const CPath &strFile, bool bVtxAnimation, Strings_t &aArguments) {
  // [Cecil] NOTE: Not yet implemented
  if (bVtxAnimation) {
    CMessageException::Throw("VTA support is not yet implemented, try SMD with skeletal animation");
  }

  _strFilePath = strFile.RemoveExt();
  _strFileName = strFile.GetFileName();

  {
    // Override converter arguments
    Str_t strArgs;

    if (ReadTextFileIfPossible(BASE_SMD_ARGS, strArgs)) {
      std::cout << "Read converted arguments from " << BASE_SMD_ARGS << "...\n";
      CharSplit<Str_t>(strArgs, ' ', aArguments);
    }
  }

  CTokenList aTokens;
  {
    // Read and tokenize SMD data
    Str_t strData = ReadTextFile(strFile);
    TokenizeSMD(aTokens, strData, bVtxAnimation);
  }
  
  // Current SMD file structure
  SmdStructure smd;
  smd.bVtxAnim = bVtxAnimation;

  // Conversion options
  SmdOptions opts;

  // Set from arguments
  Strings_t::const_iterator itOption;
  const Strings_t::const_iterator itArgEnd = aArguments.end();

  for (itOption = aArguments.begin(); itOption != itArgEnd; ++itOption) {
    const Str_t &strOption = *itOption;

    // Custom model scale
    if (strOption == "-scale") {
      opts.bArgSet[0] = true;
      ++itOption;

      // No scale specified
      if (itOption == itArgEnd) {
        CMessageException::Throw("Please specify model scale after the 'scale' argument");
      }

      Str_t strScale = *itOption;
      opts.fScale = atof(strScale.c_str());

    // Source to SE1 scale
    } else if (strOption == "-fixscale") {
      opts.bArgSet[0] = true;
      opts.fScale = 1.0/64.0;

    // Fix model facing direction
    } else if (strOption == "-fixdir") {
      opts.bArgSet[1] = true;
      opts.bFixFaceDir = true;

    // Keep model facing direction
    } else if (strOption == "-keepdir") {
      opts.bArgSet[1] = true;
      opts.bFixFaceDir = false;

    // Fix animation facing direction
    } else if (strOption == "-fixanim") {
      opts.bArgSet[2] = true;
      opts.bFixAnimNorth = true;

    // Keep animation facing direction
    } else if (strOption == "-keepanim") {
      opts.bArgSet[2] = true;
      opts.bFixAnimNorth = false;

    // Base SMD model for the animation
    } else if (strOption == "-base") {
      opts.bArgSet[3] = true;
      ++itOption;

      // No model specified
      if (itOption == itArgEnd) {
        CMessageException::Throw("Please specify path to the SMD model after the 'base' argument");
      }

      opts.strBaseSMD = *itOption;
    }
  }
  
  // Apply default options for the SMD converter
  #if 0
    opts.fScale = 1.0/64.0;
    opts.bFixFaceDir = true;
    opts.bFixAnimNorth = true;
    opts.strBaseSMD = ANIM_BASE_SMD;
    opts.SetAll(true);
  #endif

  // Get scale multiplier
  if (!opts.bArgSet[0]) {
    if (ConsoleYN("Convert Source units to Serious Engine meters?", true)) {
      opts.fScale = 1.0/64.0;
    }
  }

  // Fix facing direction
  if (!opts.bArgSet[1]) {
    opts.bFixFaceDir = ConsoleYN("Fix facing direction?", true);
  }

  std::cout << '\n';

  // Build SMD file
  extern void BuildSMD(CTokenList &aTokens, SmdStructure &smd);
  BuildSMD(aTokens, smd);
  
  std::cout << "Built skeletal " << (smd.bAnimFile ? "animation" : "mesh") << " file...\n";

  // Animation SMD options
  if (smd.bAnimFile) {
    // Fix forward direction for animations
    if (!opts.bArgSet[2]) {
      opts.bFixAnimNorth = ConsoleYN("Fix forward direction for animations from east to north?", true);
    }

    // Retrieve default skeleton
    if (!opts.bArgSet[3]) {
      std::cout << "Specify SMD model file that this animation is for: ";
      std::getline(std::cin, opts.strBaseSMD);

      // Set to default
      if (opts.strBaseSMD.empty()) {
        opts.strBaseSMD = ANIM_BASE_SMD;
      }
    }

    std::cout << '\n';

    // Open config with info about animations
    Str_t strJSON;
    
    if (ReadTextFileIfPossible(ANIM_INFOS, strJSON)) {
      std::cout << "Reading infos about animations...\n";

      // Build JSON object
      CVariant valInfo;
      ParseJSON(valInfo, nullptr, strJSON);

      // Find entry about the current file
      CValObject &oInfo = valInfo.ToObject();

      const Str_t strAnimFile = strFile.RemoveDir();
      CValObject::const_iterator it = oInfo.find(strAnimFile);

      // Save info in the options
      if (it != oInfo.end()) {
        opts.valAnimInfo = it->second;
        std::cout << "Retrieved information about " << strAnimFile << "...\n\n";

      } else {
        std::cout << "No information found about " << strAnimFile << "...\n\n";
      }
    }
  }

  // Take default positions for bones from the external skeleton
  if (smd.bAnimFile && !opts.strBaseSMD.empty()) {
    // Read SMD skeleton
    Str_t strSkelData;
    
    // Couldn't open the base model file
    if (!ReadTextFileIfPossible(opts.strBaseSMD, strSkelData)) {
      // Throw exception if couldn't open the specified file
      if (opts.strBaseSMD != ANIM_BASE_SMD) {
        CMessageException::Throw("Cannot open the base SMD file (most likely doesn't exist)");
      }

    // Opened the base model file
    } else {
      // Tokenize the skeleton
      CTokenList aSkelTokens;
      TokenizeSMD(aSkelTokens, strSkelData, false);
    
      // Build the skeleton
      SmdStructure smdSkeleton;
      smdSkeleton.bOnlySkeleton = true;
      BuildSMD(aSkelTokens, smdSkeleton);

      const CEnvelopes &skelDefault = smdSkeleton.aFrames[0].aBones;
      CEnvelopes &skelAnim = smd.aFrames[0].aBones;

      // Mismatching bone amount
      if (skelAnim.size() != skelDefault.size()) {
        CMessageException::Throw("Base bone count of the animation differs from the bone count of the external skeleton");
      }
    
      // Copy bone envelopes
      for (size_t iBone = 0; iBone < skelAnim.size(); ++iBone) {
        skelAnim[iBone].CopyPlacement(skelDefault[iBone]);
      }
    }
  }

  // Calculate proper positions for every bone
  {
    // First frame contains default positions of every skeleton bone
    for (s32 iFrame = 0; iFrame < smd.iFrames; ++iFrame) {
      CEnvelopes &aEnvelopes = smd.aFrames[iFrame].aBones;

      for (size_t iEnv = 0; iEnv < aEnvelopes.size(); ++iEnv) {
        if (!smd.aFrames[iFrame].aUsed[iEnv]) {
          continue;
        }

        CBoneEnvelope &env = aEnvelopes[iEnv];
        const CBoneInfo &info = *env.pInfo;

        // Scale the bone
        env.vPos *= opts.fScale;

        // Resulting placement
        Vec3D vBonePos = env.vPos;
        Ang3D vBoneRot = env.vRot;

        // Convert to matrix
        Mat3D m3D;
        Mat3DFromAngles(m3D, vBoneRot);

        // Convert rotation angles to SE1
        QuatD q;
        q.FromMatrix(m3D);
        q = QuatD(-q._w, -q._y, -q._x, -q._z); // Swap X and Y
        q.ToMatrix(m3D);
        
        // Quaternion swap and negation is equal to this
        /*Mat3D mInvert = m3D;
        mInvert(0, 0) = m3D(1, 1);
        mInvert(0, 1) = m3D(0, 1);
        mInvert(0, 2) = m3D(2, 1);
        mInvert(1, 0) = m3D(1, 0);
        mInvert(1, 1) = m3D(0, 0);
        mInvert(1, 2) = m3D(2, 0);
        mInvert(2, 0) = m3D(1, 2);
        mInvert(2, 1) = m3D(0, 2);
        mInvert(2, 2) = m3D(2, 2);*/

        // Fixed angles
        Mat3DToAngles(m3D, vBoneRot);
        
        // Fix facing and forward direction for root bones
        if (info.iParent == -1) {
          // Axis changes correlate with RotateTrackBall angle application in the opposite order
          if (smd.bAnimFile) {
            // Fix facing from SMD to SE1
            if (opts.bFixFaceDir && opts.bFixAnimNorth) {
              // +X+Y+Z -> -Y+X+Z -> -Y+Z-X
              SwapAxes(vBonePos, AXIS_mY, AXIS__Z, AXIS_mX);
              vBoneRot.RotateTrackball(Ang3D(0.0, -90.0, 90.0).DegToRad());
              
            // Fix facing from Source to SE1
            } else if (opts.bFixFaceDir) {
              // +X+Y+Z -> +X+Z-Y -> -X+Z+Y
              SwapAxes(vBonePos, AXIS_mX, AXIS__Z, AXIS__Y);
              vBoneRot.RotateTrackball(Ang3D(180.0, -90.0, 0.0).DegToRad());

            // Fix facing from SMD to Source
            } else if (opts.bFixAnimNorth) {
              // +X+Y -> +Y-X
              SwapAxes(vBonePos, AXIS__Y, AXIS_mX, AXIS__Z);
              vBoneRot.RotateTrackball(Ang3D(0.0, 0.0, -90.0).DegToRad());
            }

          } else {
            // Fix facing from Source to SE1
            if (opts.bFixFaceDir) {
              // +X+Y+Z -> +X+Z-Y -> -X+Z+Y
              SwapAxes(vBonePos, AXIS_mX, AXIS__Z, AXIS__Y);
              vBoneRot.RotateTrackball(Ang3D(180.0, -90.0, 0.0).DegToRad());
            }
          }
        }

        // Make bone placement matrix
        Mat3DFromAngles(m3D, vBoneRot);
        Mat3DtoMat12(env.mConverted, m3D, vBonePos);
      }
    }
  }
  
  extern void WriteMesh(const SmdOptions &opts, const SmdStructure &smd);
  extern void WriteSkeleton(const SmdOptions &opts, const SmdStructure &smd);
  extern void WriteAnimation(const SmdOptions &opts, const SmdStructure &smd);

  // Write mesh
  if (!smd.bAnimFile) {
    WriteMesh(opts, smd);
  }

  // Write skeleton
  WriteSkeleton(opts, smd);

  // Write animation
  WriteAnimation(opts, smd);
  
  std::cout << "\nSuccessfully converted Valve SMD model into SE1 ASCII model!\n";
};
