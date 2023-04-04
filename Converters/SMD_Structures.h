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

#ifndef _SMD_STRUCTURES_H
#define _SMD_STRUCTURES_H

#define Tkn_t CParserToken

// SMD vertex weight
class CWeight {
  public:
    s32 iVertex;
    f64 fWeight;
    
  public:
    CWeight(const s32 iSetVtx, const f64 fSetWeight) :
      iVertex(iSetVtx), fWeight(fSetWeight)
    {
    };
};

// SMD skeleton bone
class CBoneInfo {
  public:
    Str_t strName;
    s32 iID;
    s32 iParent;

    // Vertices affected by this bone
    std::vector<CWeight> aWeights;

  public:
    CBoneInfo(const s32 iSetID, const s32 iSetParent, const Str_t &strSetName) :
      strName(strSetName), iID(iSetID), iParent(iSetParent)
    {
    };
};

// Bone list
typedef std::vector<CBoneInfo> CBones;

// SMD bone envelope in the animation
class CBoneEnvelope {
  public:
    // Bone info
    CBoneInfo *pInfo;

    // Bone placement
    Vec3D vPos;
    Ang3D vRot;

    // Converted placement as a matrix with positon
    Mat12D mConverted;

  public:
    // Default constructor
    CBoneEnvelope(CBoneInfo *pSetInfo = nullptr) :
      pInfo(pSetInfo)
    {
    };

    // Copy bone placement
    void CopyPlacement(const CBoneEnvelope &envOther) {
      vPos = envOther.vPos;
      vRot = envOther.vRot;
      mConverted = envOther.mConverted;
    };
};

// Print out the placement matrix
#if !_DEV_STL_IO
inline void PrintPlacement(const Mat12D &m, CStringStream &out) {
#else
inline void PrintPlacement(const Mat12D &m, Out_t &out) {
#endif
  #if 1
    for (s32 i = 0; i < 12; ++i) {
      out << m(i / 4, i % 4) << (i == 11 ? ";" : ", ");
    }

  #else
    // Print the same way as MilkShape 3D
    Mat12D m12 = m;

    // Fix negative zeros
    for (s32 i = 0; i < 12; ++i) {
      if (m12(i / 4, i % 4) == -0.0) {
        m12(i / 4, i % 4) = 0.0;
      }
    }

    out << m12(0, 0) << ',' << m12(0, 1) << ',' << m12(0, 2) << ',' << m12(0, 3) << ",\t";
    out << m12(1, 0) << ',' << m12(1, 1) << ',' << m12(1, 2) << ',' << m12(1, 3) << ",\t";
    out << m12(2, 0) << ',' << m12(2, 1) << ',' << m12(2, 2) << ',' << m12(2, 3) << ";";
  #endif
};

// Bone envelopes
typedef std::vector<CBoneEnvelope> CEnvelopes;

// One frame with envelope positions
class CAnimFrame {
  public:
    CEnvelopes aBones; // Bone envelopes
    Bits_t aUsed; // Mark used bones

  public:
    // Allocate enough space for all envelopes
    CAnimFrame(const s32 iBones) : aBones(iBones), aUsed(iBones, false)
    {
    };
};

// Animation frames
typedef std::vector<CAnimFrame> CAnimation;

// SMD mesh vertex
class CVertex {
  public:
    s32 iBone;
    Vec3D vPos;
    Vec3D vNormal;
    Vec2D vUV;
};

// Vertex list
typedef std::vector<CVertex> CVertices;

// SMD mesh polygon
class CPolygon {
  public:
    Str_t strMaterial;
    Ints_t aiVertices;

  public:
    CPolygon(const Str_t &strSetMat) : strMaterial(strSetMat)
    {
    };
};

// Polygon list
typedef std::vector<CPolygon> CPolygons;

// Surface list
typedef std::map<Str_t, CPolygons> CSurfaces;

// Swap east and north axes (e.g. SMD -> Source w/ Y upwards)
/*inline void EastToNorthAxisSE1(Vec3D &v) {
  // YZX -> XZY (in SE1)
  v = Vec3D(-v[2], v[1], v[0]);
};

// Swap east and north axes (e.g. SMD -> Source w/ Z upwards)
inline void EastToNorthAxisSource(Vec3D &v) {
  // ZXY -> XZY (in SE1)
  v = Vec3D(v[1], -v[0], v[2]);
};

// Source coordinate system into SE1
inline void SourceToSE1Axis(Vec3D &v) {
  // XZY -> XYZ (in SE1)
  v = Vec3D(-v[0], v[2], v[1]);
};

// SMD animation coordinate system into SE1
inline void SmdAnimToSE1Axis(Vec3D &v) {
  // ZXY -> ZYX or XZY -> XYZ (in SE1)
  v = Vec3D(-v[1], v[2], -v[0]);

  // Equivalent to this (in this specific order)
  //SourceToSE1Axis(v);
  //EastToNorthAxisSE1(v);
};*/

// EastToNorthAxisSE1    - SwapAxes(v, AXIS_mZ, AXIS__Y, AXIS__X);
// EastToNorthAxisSource - SwapAxes(v, AXIS__Y, AXIS_mX, AXIS__Z);
// SourceToSE1Axis       - SwapAxes(v, AXIS_mX, AXIS__Z, AXIS__Y);
// SmdAnimToSE1Axis      - SwapAxes(v, AXIS_mY, AXIS__Z, AXIS_mX);

// Axis directions
#define AXIS_center 0
#define AXIS_mX 1 // -X
#define AXIS_mY 2 // -Y
#define AXIS_mZ 3 // -Z
#define AXIS__X 4 // +X
#define AXIS__Y 5 // +Y
#define AXIS__Z 6 // +Z

// Swap facing axes (+X, +Y, +Z by default)
inline void SwapAxes(Vec3D &v, const u8 x, const u8 y, const u8 z) {
  const f64 aDirections[7] = {
    0.0, // Centered position
    -v[0], -v[1], -v[2], // -X, -Y, -Z
    +v[0], +v[1], +v[2], // +X, +Y, +Z
  };

  v = Vec3D(aDirections[x], aDirections[y], aDirections[z]);
};

// Current SMD file
extern Str_t _strFilePath;
extern Str_t _strFileName;

// SMD file structure
struct SmdStructure {
  // Skeleton bones
  CBones aSkeleton;
  s32 iBones;

  // Animation frames
  CAnimation aFrames;
  s32 iFrames;

  CVertices aVertices; // Mesh vertices
  CSurfaces aSurfaces; // Mesh surfaces with polygons

  bool bAnimFile;     // Skeletal animation file
  bool bVtxAnim;      // Vertex animation file
  bool bOnlySkeleton; // Skeleton file

  // Default constructor
  SmdStructure(void) {
    iBones = 0;
    iFrames = 0;
    bAnimFile = true;
    bVtxAnim = false;
    bOnlySkeleton = false;
  };

  // Clear the structure
  void Clear(void) {
    aSkeleton.clear();
    iBones = 0;

    aFrames.clear();
    iFrames = 0;

    aVertices.clear();
    aSurfaces.clear();

    bAnimFile = true;
    bVtxAnim = false;
    bOnlySkeleton = false;
  };
};

// Converter options
struct SmdOptions {
  f64 fScale;
  bool bFixFaceDir;
  bool bFixAnimNorth;
  Str_t strBaseSMD;

  // Pre-set options
  bool bArgSet[4];

  // Animation info
  CVariant valAnimInfo;

  SmdOptions(void) {
    fScale = 1.0;
    bFixFaceDir = false;
    bFixAnimNorth = false;
    strBaseSMD = "";
    SetAll(false);
  };

  inline void SetAll(bool bState) {
    memset(bArgSet, bState, sizeof(bool) * 4);
  };
};

#endif
