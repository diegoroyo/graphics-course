#include "photonkdtree.h"

void testKdTreeKNN() {
    // Simple KdTree KNearestNeighbours search
    Vec4 point(-5.0f, 5.0f, 0.0f, 0.0f); // search knn of point

    // Create KdTree and show distances for validation
    auto builder = PhotonKdTree::builder();
    std::vector<PointLight> photonsSource = {
        PointLight(Vec4(2.0f, 3.0f, 0.0f, 0.0f), RGBColor::White),
        PointLight(Vec4(5.0f, 4.0f, 0.0f, 0.0f), RGBColor::White),
        PointLight(Vec4(9.0f, 6.0f, 0.0f, 0.0f), RGBColor::White),
        PointLight(Vec4(4.0f, 7.0f, 0.0f, 0.0f), RGBColor::White),
        PointLight(Vec4(8.0f, 1.0f, 0.0f, 0.0f), RGBColor::White),
        PointLight(Vec4(7.0f, 2.0f, 0.0f, 0.0f), RGBColor::White)};
    for (const PointLight& photon : photonsSource) {
        std::cout << photon.point << ": " << (photon.point - point).module()
                  << std::endl;
        builder.add(photon);
    }
    PhotonKdTree tree = builder.build();

    // std::cout << tree << std::endl; // show tree

    // Perform kNN search and show results
    std::vector<const PointLight*> photonsNN;
    tree.searchNN(photonsNN, point, 3);
    for (const auto* photon : photonsNN) {
        std::cout << photon->point << std::endl;
    }
}

int main(int argc, char** argv) {
    testKdTreeKNN();
}