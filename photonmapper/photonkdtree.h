#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include "math/geometry.h"
#include "scene/light.h"

class PhotonKdTree {
   private:
    class Node;
    typedef std::shared_ptr<PhotonKdTree::Node> NodePtr;

    class Node {
       public:
        PointLight photon;
        Vec4 axis;  // has 1.0f on axis of division
        bool leaf;
        NodePtr left, right;
        Node(const PointLight &_photon)
            : photon(_photon),
              axis(0.0f),
              leaf(true),
              left(nullptr),
              right(nullptr) {}
        Node(const PointLight &_photon, const Vec4 &_axis, const NodePtr &_left,
             const NodePtr &_right)
            : photon(_photon),
              axis(_axis),
              leaf(false),
              left(_left),
              right(_right) {}
    };

    class Builder {
       private:
        std::vector<PointLight> photons;

        PhotonKdTree::NodePtr dividePhotons(
            std::vector<PointLight>::iterator &vbegin,
            std::vector<PointLight>::iterator &vend);

       public:
        Builder() : photons() {}

        Builder add(const PointLight &photon) {
            photons.push_back(photon);
            return *this;
        }
        PhotonKdTree build();
    };

    NodePtr root;
    PhotonKdTree(const NodePtr &_root) : root(_root) {}

    // Helper for operator<<
    std::ostream &printNode(std::ostream &os, const NodePtr &node) const;

    // Helper for searchNN
    void searchNode(std::vector<const PointLight *> &best, const Vec4 &point,
                    int k, const NodePtr &node, float &worstDistance) const;

   public:
    static PhotonKdTree::Builder builder() { return PhotonKdTree::Builder(); }

    // k Nearest Neighbours search
    void searchNN(std::vector<const PointLight *> &photons, const Vec4 &point,
                  int k = 1) const;

    // Prints tree structure
    friend std::ostream &operator<<(std::ostream &os, const PhotonKdTree &tree);
};