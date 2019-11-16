#include "plymodel.h"

PLYModel::PLYModel(const char *filename) {
    std::string plyFilename = std::string(filename) + ".ply";
    std::ifstream is(plyFilename);
    if (!is.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        is.close();
        return;
    }

    // Read PLY file
    // ply header + format info.
    std::string ply, format;
    std::getline(is, ply);
    std::getline(is, format);
    if (ply != "ply" || format != "format ascii 1.0") {
        std::cerr << "Unsupported PLY file format (only ascii 1.0 format)"
                  << std::endl;
        return;
    }
    // read line-by-line until header end
    bool hasUV = false;  // model has UV mapping included
    std::string line;
    std::getline(is, line);
    while (!is.eof() && line != "end_header") {
        if (line.substr(0, 7) == "comment") {
            // Comment line, ignore
            // Read next line
            std::getline(is, line);
        } else if (line.substr(0, 7) == "element") {
            if (line.substr(8, 6) == "vertex") {
                // Vertex info
                int nverts = std::stoi(line.substr(15));
                this->verts.resize(nverts);
                // Check for x, y, z vertex info (mandatory)
                std::string propx, propy, propz;
                std::getline(is, propx);
                std::getline(is, propy);
                std::getline(is, propz);
                if (propx != "property float x" ||
                    propy != "property float y" ||
                    propz != "property float z") {
                    std::cerr << "Unsupported vertex properties" << std::endl;
                    return;
                }
                // Check for optional UV mapping data (s, t)
                std::getline(is, line);
                if (line == "property float s") {
                    std::getline(is, line);
                    if (line == "property float t") {
                        // PLY has model UV data
                        this->uvs.resize(nverts);
                        hasUV = true;
                        // Read next line
                        std::getline(is, line);
                    } else {
                        // Can't have s without t
                        std::cerr << "Invalid UV information" << std::endl;
                        return;
                    }
                }
                // Already has read the next line
            } else if (line.substr(8, 4) == "face") {
                // Face info
                int nfaces = std::stoi(line.substr(13));
                this->faces.resize(nfaces);
                std::string propf;
                std::getline(is, propf);
                if (propf != "property list uchar uint vertex_indices") {
                    std::cerr << "Unsupported face properties" << std::endl;
                    return;
                }
                // Read next line
                std::getline(is, line);
            } else {
                std::cerr << "Unsupported element info in PLY file"
                          << std::endl;
                return;
            }
        }
    }

    // Check valid header info
    if (is.eof()) {
        std::cerr << "Reading PLY file ended unexpectedly" << std::endl;
        return;
    }
    if (verts.size() == 0 || faces.size() == 0) {
        std::cerr << "Invalid header (vert or face info)" << std::endl;
        return;
    }

    // Read vert and face info
    for (int v = 0; v < verts.size(); v++) {
        is >> verts[v].x >> verts[v].y >> verts[v].z;
        if (hasUV) {
            is >> uvs[v][0] >> uvs[v][1];
        }
        verts[v].w = 1.0f;  // vertex are points in space
    }
    int fsize;
    for (int f = 0; f < faces.size(); f++) {
        is >> fsize;
        if (fsize != 3) {
            std::cerr << "Only triangles are supported for PLY files"
                      << std::endl;
            return;
        }
        is >> faces[f][0] >> faces[f][1] >> faces[f][2];
    }

    // Check if file was read correctly
    if (is.eof()) {
        std::cerr << "Reading PLY file ended unexpectedly" << std::endl;
        return;
    }
    is.close();

    // Read texture image if image has UV data
    std::string textureFilename = std::string(filename) + "_diffuse.ppm";
    emissionTexture.readFile(textureFilename.c_str());
    emissionTexture.flipVertically();  // correct for UV mapping
}

void PLYModel::transform(const Mat4 &modelMatrix) {
    for (int v = 0; v < this->nverts(); v++) {
        this->verts[v] = modelMatrix * this->verts[v];
    }
}

void PLYModel::getBoundingBox(const std::vector<int> &findex, Vec4 &bb0,
                              Vec4 &bb1) const {
    // Initialise with first vertex of first face
    bb0 = Vec4(this->face(findex[0])[0]);
    bb1 = Vec4(this->face(findex[0])[0]);
    // Check against all vertices of all faces
    for (int fi : findex) {
        std::array<int, 3> vi = this->face(fi);
        for (int i = 0; i < 3; i++) {
            Vec4 v = this->vert(vi[i]);
            // save min components in bb0
            bb0.x = v.x < bb0.x ? v.x : bb0.x;
            bb0.y = v.y < bb0.y ? v.y : bb0.y;
            bb0.z = v.z < bb0.z ? v.z : bb0.z;
            // save max components in bb1
            bb1.x = v.x > bb1.x ? v.x : bb1.x;
            bb1.y = v.y > bb1.y ? v.y : bb1.y;
            bb1.z = v.z > bb1.z ? v.z : bb1.z;
        }
    }
}

FigurePtr PLYModel::divideNode(const FigurePtrVector &triangles,
                               const std::vector<int> &findex,
                               std::vector<Vec4 *>::iterator &vbegin,
                               std::vector<Vec4 *>::iterator &vend,
                               int numIterations) {
    // Find bounding box of all vertices
    Vec4 bb0, bb1;
    this->getBoundingBox(findex, bb0, bb1);
    FigurePtr nodeBbox = FigurePtr(new Figures::Box(bb0, bb1));
    if (numIterations == 0) {
        // Add all triangles that correspond to the faces inside bbox
        FigurePtrVector nodeTriangles;
        for (int fi : findex) {
            nodeTriangles.push_back(triangles[fi]);
        }
        // Generate child node (children are triangles, not kdnodes)
        return FigurePtr(new Figures::BVNode(nodeTriangles, nodeBbox));
    } else {
        // Find biggest axis in bbox, store in axis
        // dot(axis, vector) only has vector's desired axis
        Vec4 bbox = bb1 - bb0;
        Vec4 axis(0.0f, 0.0f, 0.0f, 0.0f);
        if (bbox.x > bbox.y && bbox.x > bbox.z) {
            axis.x = 1.0f;
        } else if (bbox.y > bbox.x && bbox.y > bbox.z) {
            axis.y = 1.0f;
        } else {
            axis.z = 1.0f;
        }
        // Find median of all vertices' coordinates in that axis
        std::vector<Vec4 *>::iterator vmedian = vbegin + (vend - vbegin) / 2;
        std::nth_element(vbegin, vmedian, vend,
                         [&axis](const Vec4 *lhs, const Vec4 *rhs) {
                             return dot(*lhs, axis) < dot(*rhs, axis);
                         });
        // Find all triangles that correspond to first half and second half
        std::vector<int> findexFirst, findexLast;
        for (int fi : findex) {
            std::array<int, 3> vi = this->face(fi);
            // If it has any of its vertices inside the bbox, add it to first
            if (std::find(vbegin, vmedian, &verts[vi[0]]) != vmedian ||
                std::find(vbegin, vmedian, &verts[vi[1]]) != vmedian ||
                std::find(vbegin, vmedian, &verts[vi[2]]) != vmedian) {
                findexFirst.push_back(fi);
            }
            // Same for last
            if (std::find(vmedian, vend, &verts[vi[0]]) != vend ||
                std::find(vmedian, vend, &verts[vi[1]]) != vend ||
                std::find(vmedian, vend, &verts[vi[2]]) != vend) {
                findexLast.push_back(fi);
            }
        }
        // Further subdivide children nodes
        FigurePtr leftChild = this->divideNode(triangles, findexFirst, vbegin,
                                               vmedian, numIterations - 1);
        FigurePtr rightChild = this->divideNode(triangles, findexLast, vmedian,
                                                vend, numIterations - 1);
        return FigurePtr(
            new Figures::KdTreeNode(leftChild, rightChild, nodeBbox));
    }
}

FigurePtr PLYModel::getFigure(int subdivisions) {
    // Create vector of all triangles & faces to be used
    FigurePtrVector triangles;
    std::vector<int> faceIndexes;
    for (int f = 0; f < this->nfaces(); f++) {
        faceIndexes.push_back(f);
        std::array<int, 3> vi = this->face(f);  // face = vertex indices
        triangles.push_back(
            FigurePtr(new Figures::Triangle(this, vi[0], vi[1], vi[2])));
    }
    // Same with pointers to vertices
    std::vector<Vec4 *> vertexPtrs;
    for (int v = 0; v < this->nverts(); v++) {
        vertexPtrs.push_back(&verts[v]);
    }
    // Call divideNode, which optimizes the model with given subdivisions
    std::vector<Vec4 *>::iterator vbegin = std::begin(vertexPtrs);
    std::vector<Vec4 *>::iterator vend = std::end(vertexPtrs);
    return this->divideNode(triangles, faceIndexes, vbegin, vend, subdivisions);
}