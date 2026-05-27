/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GL/glus.h"

#include <math.h>

/*
 * Column-major element access macros.
 * Element at row r, column c in an N-wide flat column-major array m:
 *   S1  3x3  (N=3)
 *   S2  5x5  (N=5)
 *   S3  7x7  (N=7)
 */
#define S1(m, r, c) ((m)[(c) * 3 + (r)])
#define S2(m, r, c) ((m)[(c) * 5 + (r)])
#define S3(m, r, c) ((m)[(c) * 7 + (r)])

/* Precomputed square-root constants used by the Wigner-D recurrence. */
static const GLUSfloat k01_03 = 0.57735026918962576450f; /* sqrt(1/3)  */
static const GLUSfloat k02_03 = 0.81649658092772603272f; /* sqrt(2/3)  */
static const GLUSfloat k04_03 = 1.15470053837925152902f; /* sqrt(4/3)  */
static const GLUSfloat k01_04 = 0.5f;                    /* sqrt(1/4)  */
static const GLUSfloat k03_04 = 0.86602540378443864676f; /* sqrt(3/4)  */
static const GLUSfloat k01_05 = 0.44721359549995793928f; /* sqrt(1/5)  */
static const GLUSfloat k03_05 = 0.77459666924148337704f; /* sqrt(3/5)  */
static const GLUSfloat k06_05 = 1.09544511501033222691f; /* sqrt(6/5)  */
static const GLUSfloat k08_05 = 1.26491106406735172882f; /* sqrt(8/5)  */
static const GLUSfloat k09_05 = 1.34164078649987381866f; /* sqrt(9/5)  */
static const GLUSfloat k01_06 = 0.40824829046386301636f; /* sqrt(1/6)  */
static const GLUSfloat k05_06 = 0.91287092917527685576f; /* sqrt(5/6)  */
static const GLUSfloat k03_08 = 0.61237243569579452349f; /* sqrt(3/8)  */
static const GLUSfloat k05_08 = 0.79056941504209483300f; /* sqrt(5/8)  */
static const GLUSfloat k09_08 = 1.06066017177982128660f; /* sqrt(9/8)  */
static const GLUSfloat k05_09 = 0.74535599249992990146f; /* sqrt(5/9)  */
static const GLUSfloat k08_09 = 0.94280904158206336545f; /* sqrt(8/9)  */
static const GLUSfloat k01_10 = 0.31622776601683793320f; /* sqrt(1/10) */
static const GLUSfloat k03_10 = 0.54772255750516611348f; /* sqrt(3/10) */
static const GLUSfloat k01_12 = 0.28867513459481288225f; /* sqrt(1/12) */
static const GLUSfloat k04_15 = 0.51639777949432218558f; /* sqrt(4/15) */
static const GLUSfloat k01_16 = 0.25f;                   /* sqrt(1/16) */
static const GLUSfloat k15_16 = 0.96824583655185422129f; /* sqrt(15/16)*/
static const GLUSfloat k01_18 = 0.23570226039551587243f; /* sqrt(1/18) */
static const GLUSfloat k01_60 = 0.12909944487358056036f; /* sqrt(1/60) */
static const GLUSfloat k03_02 = 1.22474487139158904909f; /* sqrt(3/2)  */

/* --------------------------------------------------------------------------
 * Band 1 - 3x3 Wigner-D matrix
 *
 * The input rotation matrix follows the GLM column-major convention:
 * rotation_matrix[col][row].  In our flat column-major storage that is
 * S1(rotation, row, col).  The output sh1[row][col] maps to S1(result, row, col).
 * -------------------------------------------------------------------------- */
GLUSvoid GLUSAPIENTRY glusSHBuildRotation1f(GLUSfloat result[9], const GLUSfloat rotation[9])
{
    S1(result, 0, 0) = S1(rotation, 1, 1);
    S1(result, 0, 1) = S1(rotation, 2, 1);
    S1(result, 0, 2) = S1(rotation, 0, 1);

    S1(result, 1, 0) = S1(rotation, 1, 2);
    S1(result, 1, 1) = S1(rotation, 2, 2);
    S1(result, 1, 2) = S1(rotation, 0, 2);

    S1(result, 2, 0) = S1(rotation, 1, 0);
    S1(result, 2, 1) = S1(rotation, 2, 0);
    S1(result, 2, 2) = S1(rotation, 0, 0);
}

/* --------------------------------------------------------------------------
 * Band 2 - 5x5 Wigner-D matrix, derived recursively from the band-1 matrix.
 * -------------------------------------------------------------------------- */
GLUSvoid GLUSAPIENTRY glusSHBuildRotation2f(GLUSfloat result[25], const GLUSfloat sh1[9])
{
    S2(result, 0, 0) = k01_04 * ((S1(sh1, 2, 2) * S1(sh1, 0, 0) + S1(sh1, 2, 0) * S1(sh1, 0, 2)) + (S1(sh1, 0, 2) * S1(sh1, 2, 0) + S1(sh1, 0, 0) * S1(sh1, 2, 2)));
    S2(result, 0, 1) = (S1(sh1, 2, 1) * S1(sh1, 0, 0) + S1(sh1, 0, 1) * S1(sh1, 2, 0));
    S2(result, 0, 2) = k03_04 * (S1(sh1, 2, 1) * S1(sh1, 0, 1) + S1(sh1, 0, 1) * S1(sh1, 2, 1));
    S2(result, 0, 3) = (S1(sh1, 2, 1) * S1(sh1, 0, 2) + S1(sh1, 0, 1) * S1(sh1, 2, 2));
    S2(result, 0, 4) = k01_04 * ((S1(sh1, 2, 2) * S1(sh1, 0, 2) - S1(sh1, 2, 0) * S1(sh1, 0, 0)) + (S1(sh1, 0, 2) * S1(sh1, 2, 2) - S1(sh1, 0, 0) * S1(sh1, 2, 0)));

    S2(result, 1, 0) = k01_04 * ((S1(sh1, 1, 2) * S1(sh1, 0, 0) + S1(sh1, 1, 0) * S1(sh1, 0, 2)) + (S1(sh1, 0, 2) * S1(sh1, 1, 0) + S1(sh1, 0, 0) * S1(sh1, 1, 2)));
    S2(result, 1, 1) = S1(sh1, 1, 1) * S1(sh1, 0, 0) + S1(sh1, 0, 1) * S1(sh1, 1, 0);
    S2(result, 1, 2) = k03_04 * (S1(sh1, 1, 1) * S1(sh1, 0, 1) + S1(sh1, 0, 1) * S1(sh1, 1, 1));
    S2(result, 1, 3) = S1(sh1, 1, 1) * S1(sh1, 0, 2) + S1(sh1, 0, 1) * S1(sh1, 1, 2);
    S2(result, 1, 4) = k01_04 * ((S1(sh1, 1, 2) * S1(sh1, 0, 2) - S1(sh1, 1, 0) * S1(sh1, 0, 0)) + (S1(sh1, 0, 2) * S1(sh1, 1, 2) - S1(sh1, 0, 0) * S1(sh1, 1, 0)));

    S2(result, 2, 0) = k01_03 * (S1(sh1, 1, 2) * S1(sh1, 1, 0) + S1(sh1, 1, 0) * S1(sh1, 1, 2)) - k01_12 * ((S1(sh1, 2, 2) * S1(sh1, 2, 0) + S1(sh1, 2, 0) * S1(sh1, 2, 2)) + (S1(sh1, 0, 2) * S1(sh1, 0, 0) + S1(sh1, 0, 0) * S1(sh1, 0, 2)));
    S2(result, 2, 1) = k04_03 * S1(sh1, 1, 1) * S1(sh1, 1, 0) - k01_03 * (S1(sh1, 2, 1) * S1(sh1, 2, 0) + S1(sh1, 0, 1) * S1(sh1, 0, 0));
    S2(result, 2, 2) = S1(sh1, 1, 1) * S1(sh1, 1, 1) - k01_04 * (S1(sh1, 2, 1) * S1(sh1, 2, 1) + S1(sh1, 0, 1) * S1(sh1, 0, 1));
    S2(result, 2, 3) = k04_03 * S1(sh1, 1, 1) * S1(sh1, 1, 2) - k01_03 * (S1(sh1, 2, 1) * S1(sh1, 2, 2) + S1(sh1, 0, 1) * S1(sh1, 0, 2));
    S2(result, 2, 4) = k01_03 * (S1(sh1, 1, 2) * S1(sh1, 1, 2) - S1(sh1, 1, 0) * S1(sh1, 1, 0)) - k01_12 * ((S1(sh1, 2, 2) * S1(sh1, 2, 2) - S1(sh1, 2, 0) * S1(sh1, 2, 0)) + (S1(sh1, 0, 2) * S1(sh1, 0, 2) - S1(sh1, 0, 0) * S1(sh1, 0, 0)));

    S2(result, 3, 0) = k01_04 * ((S1(sh1, 1, 2) * S1(sh1, 2, 0) + S1(sh1, 1, 0) * S1(sh1, 2, 2)) + (S1(sh1, 2, 2) * S1(sh1, 1, 0) + S1(sh1, 2, 0) * S1(sh1, 1, 2)));
    S2(result, 3, 1) = S1(sh1, 1, 1) * S1(sh1, 2, 0) + S1(sh1, 2, 1) * S1(sh1, 1, 0);
    S2(result, 3, 2) = k03_04 * (S1(sh1, 1, 1) * S1(sh1, 2, 1) + S1(sh1, 2, 1) * S1(sh1, 1, 1));
    S2(result, 3, 3) = S1(sh1, 1, 1) * S1(sh1, 2, 2) + S1(sh1, 2, 1) * S1(sh1, 1, 2);
    S2(result, 3, 4) = k01_04 * ((S1(sh1, 1, 2) * S1(sh1, 2, 2) - S1(sh1, 1, 0) * S1(sh1, 2, 0)) + (S1(sh1, 2, 2) * S1(sh1, 1, 2) - S1(sh1, 2, 0) * S1(sh1, 1, 0)));

    S2(result, 4, 0) = k01_04 * ((S1(sh1, 2, 2) * S1(sh1, 2, 0) + S1(sh1, 2, 0) * S1(sh1, 2, 2)) - (S1(sh1, 0, 2) * S1(sh1, 0, 0) + S1(sh1, 0, 0) * S1(sh1, 0, 2)));
    S2(result, 4, 1) = (S1(sh1, 2, 1) * S1(sh1, 2, 0) - S1(sh1, 0, 1) * S1(sh1, 0, 0));
    S2(result, 4, 2) = k03_04 * (S1(sh1, 2, 1) * S1(sh1, 2, 1) - S1(sh1, 0, 1) * S1(sh1, 0, 1));
    S2(result, 4, 3) = (S1(sh1, 2, 1) * S1(sh1, 2, 2) - S1(sh1, 0, 1) * S1(sh1, 0, 2));
    S2(result, 4, 4) = k01_04 * ((S1(sh1, 2, 2) * S1(sh1, 2, 2) - S1(sh1, 2, 0) * S1(sh1, 2, 0)) - (S1(sh1, 0, 2) * S1(sh1, 0, 2) - S1(sh1, 0, 0) * S1(sh1, 0, 0)));
}

/* --------------------------------------------------------------------------
 * Band 3 - 7x7 Wigner-D matrix, derived recursively from band-1 and band-2.
 * -------------------------------------------------------------------------- */
GLUSvoid GLUSAPIENTRY glusSHBuildRotation3f(GLUSfloat result[49], const GLUSfloat sh1[9], const GLUSfloat sh2[25])
{
    /* Row 0 */
    S3(result, 0, 0) = k01_04 * ((S1(sh1, 2, 2) * S2(sh2, 0, 0) + S1(sh1, 2, 0) * S2(sh2, 0, 4)) + (S1(sh1, 0, 2) * S2(sh2, 4, 0) + S1(sh1, 0, 0) * S2(sh2, 4, 4)));
    S3(result, 0, 1) = k03_02 * (S1(sh1, 2, 1) * S2(sh2, 0, 0) + S1(sh1, 0, 1) * S2(sh2, 4, 0));
    S3(result, 0, 2) = k15_16 * (S1(sh1, 2, 1) * S2(sh2, 0, 1) + S1(sh1, 0, 1) * S2(sh2, 4, 1));
    S3(result, 0, 3) = k05_06 * (S1(sh1, 2, 1) * S2(sh2, 0, 2) + S1(sh1, 0, 1) * S2(sh2, 4, 2));
    S3(result, 0, 4) = k15_16 * (S1(sh1, 2, 1) * S2(sh2, 0, 3) + S1(sh1, 0, 1) * S2(sh2, 4, 3));
    S3(result, 0, 5) = k03_02 * (S1(sh1, 2, 1) * S2(sh2, 0, 4) + S1(sh1, 0, 1) * S2(sh2, 4, 4));
    S3(result, 0, 6) = k01_04 * ((S1(sh1, 2, 2) * S2(sh2, 0, 4) - S1(sh1, 2, 0) * S2(sh2, 0, 0)) + (S1(sh1, 0, 2) * S2(sh2, 4, 4) - S1(sh1, 0, 0) * S2(sh2, 4, 0)));

    /* Row 1 */
    S3(result, 1, 0) = k01_06 * (S1(sh1, 1, 2) * S2(sh2, 0, 0) + S1(sh1, 1, 0) * S2(sh2, 0, 4)) + k01_06 * ((S1(sh1, 2, 2) * S2(sh2, 1, 0) + S1(sh1, 2, 0) * S2(sh2, 1, 4)) + (S1(sh1, 0, 2) * S2(sh2, 3, 0) + S1(sh1, 0, 0) * S2(sh2, 3, 4)));
    S3(result, 1, 1) = S1(sh1, 1, 1) * S2(sh2, 0, 0) + (S1(sh1, 2, 1) * S2(sh2, 1, 0) + S1(sh1, 0, 1) * S2(sh2, 3, 0));
    S3(result, 1, 2) = k05_08 * S1(sh1, 1, 1) * S2(sh2, 0, 1) + k05_08 * (S1(sh1, 2, 1) * S2(sh2, 1, 1) + S1(sh1, 0, 1) * S2(sh2, 3, 1));
    S3(result, 1, 3) = k05_09 * S1(sh1, 1, 1) * S2(sh2, 0, 2) + k05_09 * (S1(sh1, 2, 1) * S2(sh2, 1, 2) + S1(sh1, 0, 1) * S2(sh2, 3, 2));
    S3(result, 1, 4) = k05_08 * S1(sh1, 1, 1) * S2(sh2, 0, 3) + k05_08 * (S1(sh1, 2, 1) * S2(sh2, 1, 3) + S1(sh1, 0, 1) * S2(sh2, 3, 3));
    S3(result, 1, 5) = S1(sh1, 1, 1) * S2(sh2, 0, 4) + (S1(sh1, 2, 1) * S2(sh2, 1, 4) + S1(sh1, 0, 1) * S2(sh2, 3, 4));
    S3(result, 1, 6) = k01_06 * (S1(sh1, 1, 2) * S2(sh2, 0, 4) - S1(sh1, 1, 0) * S2(sh2, 0, 0)) + k01_06 * ((S1(sh1, 2, 2) * S2(sh2, 1, 4) - S1(sh1, 2, 0) * S2(sh2, 1, 0)) + (S1(sh1, 0, 2) * S2(sh2, 3, 4) - S1(sh1, 0, 0) * S2(sh2, 3, 0)));

    /* Row 2 */
    S3(result, 2, 0) = k04_15 * (S1(sh1, 1, 2) * S2(sh2, 1, 0) + S1(sh1, 1, 0) * S2(sh2, 1, 4)) + k01_05 * (S1(sh1, 0, 2) * S2(sh2, 2, 0) + S1(sh1, 0, 0) * S2(sh2, 2, 4)) - k01_60 * ((S1(sh1, 2, 2) * S2(sh2, 0, 0) + S1(sh1, 2, 0) * S2(sh2, 0, 4)) - (S1(sh1, 0, 2) * S2(sh2, 4, 0) + S1(sh1, 0, 0) * S2(sh2, 4, 4)));
    S3(result, 2, 1) = k08_05 * S1(sh1, 1, 1) * S2(sh2, 1, 0) + k06_05 * S1(sh1, 0, 1) * S2(sh2, 2, 0) - k01_10 * (S1(sh1, 2, 1) * S2(sh2, 0, 0) - S1(sh1, 0, 1) * S2(sh2, 4, 0));
    S3(result, 2, 2) = S1(sh1, 1, 1) * S2(sh2, 1, 1) + k03_04 * S1(sh1, 0, 1) * S2(sh2, 2, 1) - k01_16 * (S1(sh1, 2, 1) * S2(sh2, 0, 1) - S1(sh1, 0, 1) * S2(sh2, 4, 1));
    S3(result, 2, 3) = k08_09 * S1(sh1, 1, 1) * S2(sh2, 1, 2) + k02_03 * S1(sh1, 0, 1) * S2(sh2, 2, 2) - k01_18 * (S1(sh1, 2, 1) * S2(sh2, 0, 2) - S1(sh1, 0, 1) * S2(sh2, 4, 2));
    S3(result, 2, 4) = S1(sh1, 1, 1) * S2(sh2, 1, 3) + k03_04 * S1(sh1, 0, 1) * S2(sh2, 2, 3) - k01_16 * (S1(sh1, 2, 1) * S2(sh2, 0, 3) - S1(sh1, 0, 1) * S2(sh2, 4, 3));
    S3(result, 2, 5) = k08_05 * S1(sh1, 1, 1) * S2(sh2, 1, 4) + k06_05 * S1(sh1, 0, 1) * S2(sh2, 2, 4) - k01_10 * (S1(sh1, 2, 1) * S2(sh2, 0, 4) - S1(sh1, 0, 1) * S2(sh2, 4, 4));
    S3(result, 2, 6) = k04_15 * (S1(sh1, 1, 2) * S2(sh2, 1, 4) - S1(sh1, 1, 0) * S2(sh2, 1, 0)) + k01_05 * (S1(sh1, 0, 2) * S2(sh2, 2, 4) - S1(sh1, 0, 0) * S2(sh2, 2, 0)) - k01_60 * ((S1(sh1, 2, 2) * S2(sh2, 0, 4) - S1(sh1, 2, 0) * S2(sh2, 0, 0)) - (S1(sh1, 0, 2) * S2(sh2, 4, 4) - S1(sh1, 0, 0) * S2(sh2, 4, 0)));

    /* Row 3 */
    S3(result, 3, 0) = k03_10 * (S1(sh1, 1, 2) * S2(sh2, 2, 0) + S1(sh1, 1, 0) * S2(sh2, 2, 4)) - k01_10 * ((S1(sh1, 2, 2) * S2(sh2, 3, 0) + S1(sh1, 2, 0) * S2(sh2, 3, 4)) + (S1(sh1, 0, 2) * S2(sh2, 1, 0) + S1(sh1, 0, 0) * S2(sh2, 1, 4)));
    S3(result, 3, 1) = k09_05 * S1(sh1, 1, 1) * S2(sh2, 2, 0) - k03_05 * (S1(sh1, 2, 1) * S2(sh2, 3, 0) + S1(sh1, 0, 1) * S2(sh2, 1, 0));
    S3(result, 3, 2) = k09_08 * S1(sh1, 1, 1) * S2(sh2, 2, 1) - k03_08 * (S1(sh1, 2, 1) * S2(sh2, 3, 1) + S1(sh1, 0, 1) * S2(sh2, 1, 1));
    S3(result, 3, 3) = S1(sh1, 1, 1) * S2(sh2, 2, 2) - k01_03 * (S1(sh1, 2, 1) * S2(sh2, 3, 2) + S1(sh1, 0, 1) * S2(sh2, 1, 2));
    S3(result, 3, 4) = k09_08 * S1(sh1, 1, 1) * S2(sh2, 2, 3) - k03_08 * (S1(sh1, 2, 1) * S2(sh2, 3, 3) + S1(sh1, 0, 1) * S2(sh2, 1, 3));
    S3(result, 3, 5) = k09_05 * S1(sh1, 1, 1) * S2(sh2, 2, 4) - k03_05 * (S1(sh1, 2, 1) * S2(sh2, 3, 4) + S1(sh1, 0, 1) * S2(sh2, 1, 4));
    S3(result, 3, 6) = k03_10 * (S1(sh1, 1, 2) * S2(sh2, 2, 4) - S1(sh1, 1, 0) * S2(sh2, 2, 0)) - k01_10 * ((S1(sh1, 2, 2) * S2(sh2, 3, 4) - S1(sh1, 2, 0) * S2(sh2, 3, 0)) + (S1(sh1, 0, 2) * S2(sh2, 1, 4) - S1(sh1, 0, 0) * S2(sh2, 1, 0)));

    /* Row 4 */
    S3(result, 4, 0) = k04_15 * (S1(sh1, 1, 2) * S2(sh2, 3, 0) + S1(sh1, 1, 0) * S2(sh2, 3, 4)) + k01_05 * (S1(sh1, 2, 2) * S2(sh2, 2, 0) + S1(sh1, 2, 0) * S2(sh2, 2, 4)) - k01_60 * ((S1(sh1, 2, 2) * S2(sh2, 4, 0) + S1(sh1, 2, 0) * S2(sh2, 4, 4)) - (S1(sh1, 0, 2) * S2(sh2, 0, 0) + S1(sh1, 0, 0) * S2(sh2, 0, 4)));
    S3(result, 4, 1) = k08_05 * S1(sh1, 1, 1) * S2(sh2, 3, 0) + k06_05 * S1(sh1, 2, 1) * S2(sh2, 2, 0) - k01_10 * (S1(sh1, 2, 1) * S2(sh2, 4, 0) + S1(sh1, 0, 1) * S2(sh2, 0, 0));
    S3(result, 4, 2) = S1(sh1, 1, 1) * S2(sh2, 3, 1) + k03_04 * S1(sh1, 2, 1) * S2(sh2, 2, 1) - k01_16 * (S1(sh1, 2, 1) * S2(sh2, 4, 1) + S1(sh1, 0, 1) * S2(sh2, 0, 1));
    S3(result, 4, 3) = k08_09 * S1(sh1, 1, 1) * S2(sh2, 3, 2) + k02_03 * S1(sh1, 2, 1) * S2(sh2, 2, 2) - k01_18 * (S1(sh1, 2, 1) * S2(sh2, 4, 2) + S1(sh1, 0, 1) * S2(sh2, 0, 2));
    S3(result, 4, 4) = S1(sh1, 1, 1) * S2(sh2, 3, 3) + k03_04 * S1(sh1, 2, 1) * S2(sh2, 2, 3) - k01_16 * (S1(sh1, 2, 1) * S2(sh2, 4, 3) + S1(sh1, 0, 1) * S2(sh2, 0, 3));
    S3(result, 4, 5) = k08_05 * S1(sh1, 1, 1) * S2(sh2, 3, 4) + k06_05 * S1(sh1, 2, 1) * S2(sh2, 2, 4) - k01_10 * (S1(sh1, 2, 1) * S2(sh2, 4, 4) + S1(sh1, 0, 1) * S2(sh2, 0, 4));
    S3(result, 4, 6) = k04_15 * (S1(sh1, 1, 2) * S2(sh2, 3, 4) - S1(sh1, 1, 0) * S2(sh2, 3, 0)) + k01_05 * (S1(sh1, 2, 2) * S2(sh2, 2, 4) - S1(sh1, 2, 0) * S2(sh2, 2, 0)) - k01_60 * ((S1(sh1, 2, 2) * S2(sh2, 4, 4) - S1(sh1, 2, 0) * S2(sh2, 4, 0)) - (S1(sh1, 0, 2) * S2(sh2, 0, 4) - S1(sh1, 0, 0) * S2(sh2, 0, 0)));

    /* Row 5 */
    S3(result, 5, 0) = k01_06 * (S1(sh1, 1, 2) * S2(sh2, 4, 0) + S1(sh1, 1, 0) * S2(sh2, 4, 4)) + k01_06 * ((S1(sh1, 2, 2) * S2(sh2, 3, 0) + S1(sh1, 2, 0) * S2(sh2, 3, 4)) - (S1(sh1, 0, 2) * S2(sh2, 1, 0) + S1(sh1, 0, 0) * S2(sh2, 1, 4)));
    S3(result, 5, 1) = S1(sh1, 1, 1) * S2(sh2, 4, 0) + (S1(sh1, 2, 1) * S2(sh2, 3, 0) - S1(sh1, 0, 1) * S2(sh2, 1, 0));
    S3(result, 5, 2) = k05_08 * S1(sh1, 1, 1) * S2(sh2, 4, 1) + k05_08 * (S1(sh1, 2, 1) * S2(sh2, 3, 1) - S1(sh1, 0, 1) * S2(sh2, 1, 1));
    S3(result, 5, 3) = k05_09 * S1(sh1, 1, 1) * S2(sh2, 4, 2) + k05_09 * (S1(sh1, 2, 1) * S2(sh2, 3, 2) - S1(sh1, 0, 1) * S2(sh2, 1, 2));
    S3(result, 5, 4) = k05_08 * S1(sh1, 1, 1) * S2(sh2, 4, 3) + k05_08 * (S1(sh1, 2, 1) * S2(sh2, 3, 3) - S1(sh1, 0, 1) * S2(sh2, 1, 3));
    S3(result, 5, 5) = S1(sh1, 1, 1) * S2(sh2, 4, 4) + (S1(sh1, 2, 1) * S2(sh2, 3, 4) - S1(sh1, 0, 1) * S2(sh2, 1, 4));
    S3(result, 5, 6) = k01_06 * (S1(sh1, 1, 2) * S2(sh2, 4, 4) - S1(sh1, 1, 0) * S2(sh2, 4, 0)) + k01_06 * ((S1(sh1, 2, 2) * S2(sh2, 3, 4) - S1(sh1, 2, 0) * S2(sh2, 3, 0)) - (S1(sh1, 0, 2) * S2(sh2, 1, 4) - S1(sh1, 0, 0) * S2(sh2, 1, 0)));

    /* Row 6 */
    S3(result, 6, 0) = k01_04 * ((S1(sh1, 2, 2) * S2(sh2, 4, 0) + S1(sh1, 2, 0) * S2(sh2, 4, 4)) - (S1(sh1, 0, 2) * S2(sh2, 0, 0) + S1(sh1, 0, 0) * S2(sh2, 0, 4)));
    S3(result, 6, 1) = k03_02 * (S1(sh1, 2, 1) * S2(sh2, 4, 0) - S1(sh1, 0, 1) * S2(sh2, 0, 0));
    S3(result, 6, 2) = k15_16 * (S1(sh1, 2, 1) * S2(sh2, 4, 1) - S1(sh1, 0, 1) * S2(sh2, 0, 1));
    S3(result, 6, 3) = k05_06 * (S1(sh1, 2, 1) * S2(sh2, 4, 2) - S1(sh1, 0, 1) * S2(sh2, 0, 2));
    S3(result, 6, 4) = k15_16 * (S1(sh1, 2, 1) * S2(sh2, 4, 3) - S1(sh1, 0, 1) * S2(sh2, 0, 3));
    S3(result, 6, 5) = k03_02 * (S1(sh1, 2, 1) * S2(sh2, 4, 4) - S1(sh1, 0, 1) * S2(sh2, 0, 4));
    S3(result, 6, 6) = k01_04 * ((S1(sh1, 2, 2) * S2(sh2, 4, 4) - S1(sh1, 2, 0) * S2(sh2, 4, 0)) - (S1(sh1, 0, 2) * S2(sh2, 0, 4) - S1(sh1, 0, 0) * S2(sh2, 0, 0)));
}

#undef S1
#undef S2
#undef S3
