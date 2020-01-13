#include "photonkdtree.h"
#include "scene/light.h"

void testKdTreeKNN() {
    // Simple KdTree KNearestNeighbours search
    Vec4 point(-5.0f, 5.0f, 0.0f, 0.0f); // search knn of point

    // Create KdTree and show distances for validation
    auto builder = PhotonKdTree::builder();
    std::vector<Photon> photonsSource = {
        Photon(Vec4(2.0f, 3.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(5.0f, 4.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(9.0f, 6.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(4.0f, 7.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(8.0f, 1.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(7.0f, 2.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White)};
    for (const Photon& photon : photonsSource) {
        std::cout << photon.point << ": " << (photon.point - point).module()
                  << std::endl;
        builder.add(photon);
    }
    PhotonKdTree tree = builder.build();

    // std::cout << tree << std::endl; // show tree

    // Perform kNN search and show results
    std::vector<const Photon*> photonsNN;
    tree.searchNN(photonsNN, point, 3);
    for (const auto* photon : photonsNN) {
        std::cout << photon->point << std::endl;
    }
}

int main(int argc, char** argv) {
    testKdTreeKNN();
}