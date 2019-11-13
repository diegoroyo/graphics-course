#include "plymodel.h"

PLYModel::PLYModel(const char* filename) {
    std::ifstream is(filename);
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
    std::string line;
    std::getline(is, line);
    while (!is.eof() && line != "end_header") {
        if (line.substr(0, 7) == "comment") {
            // Comment line, ignore
        } else if (line.substr(0, 7) == "element") {
            if (line.substr(8, 6) == "vertex") {
                // Vertex info
                int nverts = std::stoi(line.substr(15));
                this->verts.resize(nverts);
                std::string propx, propy, propz;
                std::getline(is, propx);
                std::getline(is, propy);
                std::getline(is, propz);
                if (propx != "property float32 x" ||
                    propy != "property float32 y" ||
                    propz != "property float32 z") {
                    std::cerr << "Unsupported vertex properties" << std::endl;
                    return;
                }
            } else if (line.substr(8, 4) == "face") {
                // Face info
                int nfaces = std::stoi(line.substr(13));
                this->faces.resize(nfaces);
                std::string propf;
                std::getline(is, propf);
                if (propf != "property list uint8 int32 vertex_indices") {
                    std::cerr << "Unsupported face properties" << std::endl;
                    return;
                }
            }
        }
        // Read next line
        std::getline(is, line);
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
}