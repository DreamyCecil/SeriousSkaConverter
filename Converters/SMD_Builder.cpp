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

// SMD building
static CTokenList *paTokens = nullptr;
static CTokenList::const_iterator it;

// Expect a certain identifier
bool ExpectKeyword(const Str_t &strID, bool bThrowException) {
  // At the end
  if (it == paTokens->end()) {
    return false;
  }

  // Invalid keyword
  if (it->GetType() != Tkn_t::TKN_IDENTIFIER || it->GetValue().ToString() != strID)
  {
    if (bThrowException) {
      CTokenException::Throw(it->GetTokenPos(), "Expected '%s' keyword", strID.c_str());
    }

    return false;
  }

  return true;
};

// Build from the tokenized SMD file
extern void BuildSMD(CTokenList &aTokens, SmdStructure &smd) {
  paTokens = &aTokens;
  it = aTokens.begin();
  
  try {
    // Expect version and skip it
    ExpectKeyword("version", true);
    it += 2;

    // Past the nodes
    ExpectKeyword("nodes", true);
    ++it;

    // Parse until the block end
    bool bEnd = false;
    s32 iNegative = 1;

    while (!bEnd) {
      // Get ID and go to the name
      s32 iID = (s32)(*it)(Tkn_t::TKN_VALUE).GetValue().ToS64();
      ++it;

      // Get name and go to parent ID
      Str_t strName = (*it)(Tkn_t::TKN_VALUE).GetValue().ToString();
      ++it;

      iNegative = 1;

      // Skip unary operators
      if (it->GetType() == Tkn_t::TKN_SUB) {
        iNegative = -1;
        ++it;
      }

      // Get parent ID and go further
      s32 iParent = (s32)(*it)(Tkn_t::TKN_VALUE).GetValue().ToS64() * iNegative;
      ++it;

      smd.aSkeleton.push_back(CBoneInfo(iID, iParent, strName));

      // Check for next bone
      bEnd = ExpectKeyword("end", false);
    }

    // Count bones in the skeleton
    smd.iBones = (s32)smd.aSkeleton.size();

    // Skip block end
    ++it;

    // Expect skeleton
    ExpectKeyword("skeleton", true);
    ++it;

    // Expect animation frame
    ExpectKeyword("time", true);

    // Parse bone positions for each frame
    bool bNextBlock = false;
    bEnd = false;
  
    // Go until the block end
    do {
      // Skip 'time'
      ++it;

      // Get time frame
      s32 iFrame = (s32)(*it)(Tkn_t::TKN_VALUE).GetValue().ToS64();
      ++it;

      // Create new frame
      CAnimFrame frame(smd.iBones);

      // It only sets to true if there are no bones in the current frame (safety check)
      bNextBlock = ExpectKeyword("time", false);

      // Parsed bones
      s32 iBonePositions = 0;

      // Go until the next frame or block end
      while (!bNextBlock && !bEnd) {
        // NOTE: If for some reason the frame doesn't contain any bone positions, this will fail

        // Get bone index
        s32 iBone = (s32)(*it)(Tkn_t::TKN_VALUE).GetValue().ToS64();

        // Invalid bone
        if (iBone < 0 || iBone >= smd.iBones) {
          CTokenException::Throw(it->GetTokenPos(), "Bone index %d is out of bounds [0, %d]", iBone, smd.iBones - 1);
        }

        // Bone envelope in this frame
        CBoneEnvelope env(&smd.aSkeleton[iBone]);
        
        // Go to the bone positions
        ++it;

        // Parse XYZHPB bone positions
        for (s32 iPos = 0; iPos < 6; ++iPos) {
          iNegative = 1;

          // Skip unary operators
          if (it->GetType() == Tkn_t::TKN_SUB) {
            iNegative = -1;
            ++it;
          }

          if (iPos < 3) {
            env.vPos[iPos] = GetNumber<f64>(it->GetValue()) * (f64)iNegative;
          } else {
            env.vRot[iPos - 3] = GetNumber<f64>(it->GetValue()) * (f64)iNegative;
          }

          ++it;
        }

        // Assign bone envelope to the frame
        frame.aBones[iBone] = env;
        frame.aUsed[iBone] = true;

        // Next frame or block end
        bEnd = ExpectKeyword("end", false);
        bNextBlock = ExpectKeyword("time", false);

        ++iBonePositions;
      }

      // Should go through all bones in the first frame
      if (iFrame == 0 && iBonePositions < smd.iBones) {
        CMessageException::Throw("Expected positions for all bones on the first frame but got %d/%d", iBonePositions, smd.iBones);
      }

      // Add animation frame
      smd.aFrames.push_back(frame);

      std::cout << "Added animation frame " << smd.aFrames.size() << "...\n";

    // Go again if there's another frame
    } while (bNextBlock && !bEnd);

    // Skip block end
    ++it;

    // Count animation frames
    smd.iFrames = (s32)smd.aFrames.size();

    if (smd.iFrames > 0) {
      std::cout << '\n';
    }

    // Only build the skeleton
    if (smd.bOnlySkeleton) {
      return;
    }
    
    // Expect triangles block, if it's present
    if (ExpectKeyword("triangles", false) && !smd.bVtxAnim) {
      smd.bAnimFile = false;

      // Go to the material of the first triangle
      ++it;

      // Parse vertices of each triangle
      bNextBlock = false;
      bEnd = false;

      // Go until the block end
      do {
        bNextBlock = false;

        // Get material name
        Str_t strMaterial = (*it)(Tkn_t::TKN_IDENTIFIER).GetValue().ToString();
        ++it;

        // New polygon
        CPolygon pol(strMaterial);

        // Go through three vertices
        for (s32 iVtx = 0; iVtx < 3; ++iVtx) {
          // Get parent bone index
          s32 iParentBone = (s32)(*it)(Tkn_t::TKN_VALUE).GetValue().ToS64();

          // Invalid bone
          if (iParentBone < 0 || iParentBone >= smd.iBones) {
            CTokenException::Throw(it->GetTokenPos(), "Bone index %d is out of bounds [0, %d]", iParentBone, smd.iBones - 1);
          }

          // New vertex
          CVertex vertex;
          vertex.iBone = iParentBone;

          s32 iVertexIndex = (s32)smd.aVertices.size();
        
          // Go to the positions
          ++it;

          // Parse XYZ vertex positions and normals
          for (s32 iPos = 0; iPos < 6; ++iPos) {
            iNegative = 1;

            // Skip unary operators
            if (it->GetType() == Tkn_t::TKN_SUB) {
              iNegative = -1;
              ++it;
            }

            if (iPos < 3) {
              vertex.vPos[iPos] = GetNumber<f64>(it->GetValue()) * (f64)iNegative;
            } else {
              vertex.vNormal[iPos - 3] = GetNumber<f64>(it->GetValue()) * (f64)iNegative;
            }

            ++it;
          }

          // Parse UV coordinates
          for (s32 iUV = 0; iUV < 2; ++iUV) {
            iNegative = 1;

            // Skip unary operators
            if (it->GetType() == Tkn_t::TKN_SUB) {
              iNegative = -1;
              ++it;
            }

            vertex.vUV[iUV] = GetNumber<f64>(it->GetValue()) * (f64)iNegative;
            ++it;
          }

          // Get amount of weights for this vertex
          s32 iWeights = (s32)(it++)->GetValue().ToS64();

          for (s32 iWeight = 0; iWeight < iWeights; ++iWeight) {
            // Get this weight's bone
            s32 iWeightBone = (s32)(*it)(Tkn_t::TKN_VALUE).GetValue().ToS64();
            ++it;

            // Get weight
            f64 fWeight = GetNumber<f64>(it->GetValue());
            ++it;

            // Add weight to the vertex
            smd.aSkeleton[iWeightBone].aWeights.push_back(CWeight(iVertexIndex, fWeight));
          }

          // Add vertex
          smd.aVertices.push_back(vertex);
          pol.aiVertices.push_back(iVertexIndex);
        }
          
        // Add new surface if it doesn't exist yet
        if (smd.aSurfaces.find(strMaterial) == smd.aSurfaces.end()) {
          smd.aSurfaces[strMaterial] = CPolygons();
        }

        // Add polygon to the surface
        smd.aSurfaces[strMaterial].push_back(pol);

        // Next triangle or block end
        if (ExpectKeyword("end", false)) {
          bEnd = true;

        } else if (it->GetType() == Tkn_t::TKN_IDENTIFIER) {
          bNextBlock = true;

        } else {
          throw CTokenException(it->GetTokenPos(), "Expected a block end or material name of another polygon after the mesh triangle");
        }

      // Go again if there's another frame
      } while (bNextBlock && !bEnd);

    // Always expect vertex animation block if it's required
    } else if (smd.bVtxAnim) {
      if (ExpectKeyword("vertexanimation", false)) {
        // TODO: Implement vertex animations
        
      } else {
        throw CTokenException(it->GetTokenPos(), "Expected 'vertexanimation' block");
      }
    }

  // Errors
  } catch (CBadAnyCastException &ex) {
    (void)ex;
    throw CTokenException(it->GetTokenPos(), "Wrong value type");

  } catch (CException &ex) {
    throw ex;
  }
};
