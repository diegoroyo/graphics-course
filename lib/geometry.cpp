#include "geometry.h"

/// Vectors ///

std::ostream &operator<<(std::ostream &s, const Vec4 &vector) {
    s << "(" << vector.x << ", " << vector.y << ", "
             << vector.z << ", " << vector.w << ")";
    return s;
}

/// Matrices ///

// fill constructor
Mat4::Mat4(float elem) {
    raw.fill(elem);
}

Mat4 Mat4::transpose() const {
    Mat4 res;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[i][j] = m[j][i];
        }
    }
    return res;
}

// (row, col) cofactor of the matrix
// https://en.wikipedia.org/wiki/Minor_(linear_algebra)
float Mat4::cofactor(int row, int col) const {
    // a, b and c are indices for accessing the 3x3 minor
    // using the 4x4 original matrix
    // for example, if row = 1 (second row) then the 3x3
    // matrix is made from rows ax = 0, bx = 2 and cx = 3
    int ax = row > 0 ? 0 : 1;
    int bx = row > 1 ? 1 : 2;
    int cx = row > 2 ? 2 : 3;
    int ay = col > 0 ? 0 : 1;
    int by = col > 1 ? 1 : 2;
    int cy = col > 2 ? 2 : 3;
    // determinant of 3x3 minor
    float det = m[ax][ay] * m[bx][by] * m[cx][cy]
              + m[ax][by] * m[bx][cy] * m[cx][ay]
              + m[ax][cy] * m[bx][ay] * m[cx][by]
              - m[ax][cy] * m[bx][by] * m[cx][ay]
              - m[ax][ay] * m[bx][cy] * m[cx][by]
              - m[ax][by] * m[bx][ay] * m[cx][cy];
    // cofactor calculation (see wikipedia link above)
    float sign = ((row + col) % 2 == 0 ? 1.0f : -1.0f);
    return sign * det;
}

// calculation using Minv = Madj / det(A)
Mat4 Mat4::inverse() const {
    Mat4 adjugate; // transposed adjugate, re-transposed at the end
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            adjugate[j][i] = cofactor(i, j);
        }
    }

    // determinant of A using first row/col of A/adjugate
    float det = adjugate[0][0] * m[0][0] + adjugate[1][0] * m[0][1] +
                adjugate[2][0] * m[0][2] + adjugate[3][0] * m[0][3];

    // Minv = Madj / det(A)
    return adjugate * (1.0f / det);
}

std::ostream &operator<<(std::ostream &s, const Mat4 &matrix) {
    s << std::endl;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            s << matrix[i][j] << ' ';
        }
        s << std::endl;
    }
    s << std::endl;
    return s;
}