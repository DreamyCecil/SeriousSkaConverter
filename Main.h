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

// File manipulation
#include <DreamyUtilities/Parser/Tokenizer.hpp>
#include <DreamyUtilities/Files/Filenames.hpp>
#include <DreamyUtilities/Files/TextFiles.hpp>

#include <DreamyUtilities/JSON/JSON.hpp>

// [Cecil] NOTE: '_DEV_STL_IO' is a development macro that switches between Dreamy Utilities' data streams and STL's.
// The current version is missing appropriate typedefs for STL streams, such as 'std::ofstream' as 'FileOut_t'.
#define _DEV_STL_IO 0

#if !_DEV_STL_IO
  #include <DreamyUtilities/IO/FileDevice.hpp>
#endif

// Math
#include <DreamyUtilities/Angles/ScalarAngles.hpp>
#include <DreamyUtilities/Matrix/ScalarMatrices.hpp>
#include <DreamyUtilities/Vector/ScalarVectors.hpp>

#include <DreamyUtilities/Matrix/Matrix12.hpp>
#include <DreamyUtilities/Quaternion/Quaternion.hpp>
#include <DreamyUtilities/Quaternion/QuatVec.hpp>

// Output
#include <iostream>

using namespace dreamy;
