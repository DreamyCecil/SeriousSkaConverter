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

// Write SMD mesh in SE1 ASCII format
extern void WriteMesh(const SmdOptions &opts, const SmdStructure &smd) {
#if !_DEV_STL_IO
  CFileDevice fileD((_strFilePath + ".am").c_str());
  CStringStream file(&fileD, IReadWriteDevice::OM_WRITEONLY);
#else
  FileOut_t file((_strFilePath + ".am").c_str());
#endif

  file << "SE_MESH 0.1;\n\n";
    
  // Vertex positions
  file << "VERTICES " << smd.aVertices.size() << "\n{\n";

  for (size_t iVtxPos = 0; iVtxPos < smd.aVertices.size(); ++iVtxPos) {
    const CVertex &vertex = smd.aVertices[iVtxPos];
    Vec3D vPos = vertex.vPos;
    
    // Scale the position
    vPos *= opts.fScale;

    // Proper placement
    if (opts.bFixFaceDir) {
      SwapAxes(vPos, AXIS_mX, AXIS__Z, AXIS__Y);
    }
      
    file << "  " << vPos[0] << ", " << vPos[1] << ", " << vPos[2] << ";\n";
  }
    
  // Vertex normals
  file << "}\n\nNORMALS " << smd.aVertices.size() << "\n{\n";

  for (size_t iVtxNormal = 0; iVtxNormal < smd.aVertices.size(); ++iVtxNormal) {
    const CVertex &vertex = smd.aVertices[iVtxNormal];
    Vec3D vNormal = vertex.vNormal;
      
    // Proper placement
    if (opts.bFixFaceDir) {
      SwapAxes(vNormal, AXIS_mX, AXIS__Z, AXIS__Y);
    }

    file << "  " << vNormal[0] << ", " << vNormal[1] << ", " << vNormal[2] << ";\n";
  }

  file << "}\n\n";

  // UV maps
  file << "UVMAPS 1\n{\n";
  file << "  {\n";

  // UV map name
  file << "    NAME \"" << _strFileName << "\";\n";

  // Texture coordinates
  file << "    TEXCOORDS " << smd.aVertices.size() << "\n    {\n";

  for (size_t iTexCoord = 0; iTexCoord < smd.aVertices.size(); ++iTexCoord) {
    const CVertex &vertex = smd.aVertices[iTexCoord];
    // Mirror vertically (e.g 0.35 becomes 0.65)
    file << "      " << vertex.vUV[0] << ", " << 1.0 - vertex.vUV[1] << ";\n";
  }
  file << "    }\n";

  file << "  }\n";
  file << "}\n\n";

  // Surfaces
  file << "SURFACES " << smd.aSurfaces.size() << "\n{\n";

  std::map<Str_t, CPolygons>::const_iterator it;

  for (it = smd.aSurfaces.begin(); it != smd.aSurfaces.end(); ++it) {
    file << "  {\n";

    // Surface name
    file << "    NAME \"" << it->first << "\";\n";

    const CPolygons &aPolygons = it->second;

    // Texture coordinates
    file << "    TRIANGLE_SET " << aPolygons.size() << "\n    {\n";

    for (size_t iPol = 0; iPol < aPolygons.size(); ++iPol) {
      const CPolygon &pol = aPolygons[iPol];
      file << "      " << pol.aiVertices[0] << ", " << pol.aiVertices[1] << ", " << pol.aiVertices[2] << ";\n";
    }

    file << "    }\n";
    file << "  }\n";
  }

  file << "}\n\n";

  // Count bones with weights
  s32 iWeights = 0;
  s32 iBone;
    
  for (iBone = 0; iBone < smd.iBones; ++iBone) {
    const CBoneInfo &bone = smd.aSkeleton[iBone];
    iWeights += (bone.aWeights.size() > 0);
  }
    
  // Vertex weights
  #if 1
  file << "WEIGHTS " << iWeights << "\n{\n";

  for (iBone = 0; iBone < smd.iBones; ++iBone) {
    const CBoneInfo &bone = smd.aSkeleton[iBone];

    // No weights
    if (bone.aWeights.size() == 0) {
      continue;
    }

    file << "  {\n";

    file << "    NAME \"" << bone.strName << "\";\n";

    file << "    WEIGHT_SET " << bone.aWeights.size() << '\n';
    file << "    {\n";

    for (size_t iWeight = 0; iWeight < bone.aWeights.size(); ++iWeight) {
      const CWeight &weight = bone.aWeights[iWeight];
      file << "      { " << weight.iVertex << "; " << weight.fWeight << "; }\n";
    }
      
    file << "    }\n";
    file << "  }\n";
  }

  file << "}\n\n";

  #else
    file << "WEIGHTS 0\n{\n}\n\n";
  #endif

  // Morphs
  file << "MORPHS 0\n{\n}\n\n";

  file << "SE_MESH_END;";

#if !_DEV_STL_IO
  fileD.Close();
#else
  file.close();
#endif

  std::cout << "Converted mesh...\n";
};
