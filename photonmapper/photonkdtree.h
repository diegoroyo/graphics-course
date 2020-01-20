#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include "math/geometry.h"
#include "scene/light.h"

class PhotonKdTree {
   private:
    class Node;
    typedef std::shared_ptr<const PhotonKdTree::Node> NodePtr;
    friend class PhotonKdTreeBuilder;

    class Node {
       public:
        const Photon photon;
        const Vec4 axis;  // has 1.0f on axis of division
        const bool leaf;
        const PhotonKdTree::NodePtr left, right;
        Node(const Photon &_photon)
            : photon(_photon),
              axis(0.0f),
              leaf(true),
              left(nullptr),
              right(nullptr) {}
        Node(const Photon &_photon, const Vec4 &_axis,
             const PhotonKdTree::NodePtr &_left,
             const PhotonKdTree::NodePtr &_right)
            : photon(_photon),
              axis(_axis),
              leaf(false),
              left(_left),
              right(_right) {}
        friend class PhotonKdTreeBuilder;
    };

    /// Attributes ///

    const PhotonKdTree::NodePtr root;
    PhotonKdTree(const PhotonKdTree::NodePtr &_root) : root(_root) {}
    friend class PhotonKdTreeBuilder;

    // Helper for operator<<
    std::ostream &printNode(std::ostream &os,
                            const PhotonKdTree::NodePtr &node) const;

    // Helper for searchNN
    void searchNode(std::vector<const Photon *> &best, const Vec4 &point, int k,
                    const PhotonKdTree::NodePtr &node,
                    float &worstDistance) const;

   public:
    // k Nearest Neighbours search
    float searchNN(std::vector<const Photon *> &photons, const Vec4 &point,
                   int k = 1) const;

    // Prints tree structure
    friend std::ostream &operator<<(std::ostream &os, const PhotonKdTree &tree);
};

class PhotonKdTreeBuilder {
   private:
    PhotonKdTree::NodePtr dividePhotons(std::vector<Photon>::iterator &vbegin,
                                        std::vector<Photon>::iterator &vend);

   public:
    std::vector<Photon> photons;

    PhotonKdTreeBuilder() : photons() {}

    PhotonKdTreeBuilder add(const Photon &photon) {
        photons.push_back(photon);
        return *this;
    }

    // clears photons vector and returns new vector
    PhotonKdTree build();
};