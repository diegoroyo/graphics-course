#include "photonkdtree.h"

/// Builder ///

PhotonKdTree::NodePtr PhotonKdTreeBuilder::dividePhotons(
    std::vector<Photon>::iterator &vbegin,
    std::vector<Photon>::iterator &vend) {
    if (vend - vbegin <= 1) {
        if (vend - vbegin == 1) {
            return PhotonKdTree::NodePtr(new PhotonKdTree::Node(*vbegin));
        } else {
            return nullptr;
        }
    }
    // Get bounding box of photons
    Vec4 bb0(vbegin->point), bb1(vbegin->point);  // bounding box
    for (auto it = vbegin + 1; it < vend; it++) {
        Vec4 pos = it->point;
        // save min components in bb0
        bb0.x = std::fminf(bb0.x, pos.x);
        bb0.y = std::fminf(bb0.y, pos.y);
        bb0.z = std::fminf(bb0.z, pos.z);
        // save max components in bb1
        bb1.x = std::fmaxf(bb1.x, pos.x);
        bb1.y = std::fmaxf(bb1.y, pos.y);
        bb1.z = std::fmaxf(bb1.z, pos.z);
    }

    // Find bigger axis
    Vec4 axis(0.0f, 0.0f, 0.0f, 0.0f);
    Vec4 bbox = bb1 - bb0;
    if (bbox.x > bbox.y && bbox.x > bbox.z) {
        axis.x = 1.0f;
    } else if (bbox.y > bbox.x && bbox.y > bbox.z) {
        axis.y = 1.0f;
    } else {
        axis.z = 1.0f;
    }

    // Find median in that axis
    std::vector<Photon>::iterator vmedian = vbegin + (vend - vbegin - 1) / 2;
    std::nth_element(vbegin, vmedian, vend,
                     [&axis](const Photon &lhs, const Photon &rhs) {
                         return dot(lhs.point, axis) < dot(rhs.point, axis);
                     });

    // Sort left and right sides and return node
    PhotonKdTree::NodePtr left = dividePhotons(vbegin, vmedian);
    std::vector<Photon>::iterator vnext = vmedian + 1;
    PhotonKdTree::NodePtr right = dividePhotons(vnext, vend);
    return PhotonKdTree::NodePtr(
        new PhotonKdTree::Node(*vmedian, axis, left, right));
}

PhotonKdTree PhotonKdTreeBuilder::build() {
    std::vector<Photon>::iterator begin = photons.begin();
    std::vector<Photon>::iterator end = photons.end();
    PhotonKdTree::NodePtr root = dividePhotons(begin, end);
    photons.clear();
    return PhotonKdTree(root);
}

/// KdTree: Searches, etc ///

void PhotonKdTree::searchNode(std::vector<const Photon *> &best,
                              const Vec4 &point, int k, const NodePtr &node,
                              float &worstDistance) const {
    const auto compare = [&point](const Photon *lhs, const Photon *rhs) {
        return (lhs->point - point).module() < (rhs->point - point).module();
    };

    if (best.size() < k) {
        // Still haven't found k elements, add it to the list
        best.push_back(&node->photon);
        if (best.size() == k) {
            std::make_heap(best.begin(), best.end(), compare);
            worstDistance = (best.front()->point - point).module();
        }
    } else {
        // Found k elements already, check if its better than any of them
        float distance = (node->photon.point - point).module();
        if (distance < worstDistance) {
            // Switch it and update worst distance
            std::pop_heap(best.begin(), best.end(), compare);
            best.pop_back();
            best.push_back(&node->photon);
            std::push_heap(best.begin(), best.end(), compare);
            worstDistance = (best.front()->point - point).module();
        }
    }

    if (!node->leaf) {
        float axisDistance =
            dot(point, node->axis) - dot(node->photon.point, node->axis);
        if (axisDistance > 1e-5f) {
            // Best options are in right half
            if (node->right != nullptr) {
                searchNode(best, point, k, node->right, worstDistance);
            }
            // Check if we can skip the left half
            if (node->left != nullptr && worstDistance > axisDistance) {
                searchNode(best, point, k, node->left, worstDistance);
            }
        } else {
            // Best options are in left half
            if (node->left != nullptr) {
                searchNode(best, point, k, node->left, worstDistance);
            }
            // Check if we can skip the right half
            if (node->right != nullptr && worstDistance > axisDistance) {
                searchNode(best, point, k, node->right, worstDistance);
            }
        }
    }
}

float PhotonKdTree::searchNN(std::vector<const Photon *> &photons,
                             const Vec4 &point, int k) const {
    photons.clear();
    float worstDistance = std::numeric_limits<float>::max();
    searchNode(photons, point, k, this->root, worstDistance);
    return worstDistance;
}

std::ostream &PhotonKdTree::printNode(std::ostream &os,
                                      const NodePtr &node) const {
    os << "Node: " << node->photon.point << " con axis " << node->axis
       << std::endl;
    os << "Left child:" << std::endl;
    if (node->left != nullptr) {
        printNode(os, node->left);
    } else {
        os << "(none)" << std::endl;
    }
    os << "Right child:" << std::endl;
    if (node->right != nullptr) {
        printNode(os, node->right);
    } else {
        os << "(none)" << std::endl;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const PhotonKdTree &tree) {
    tree.printNode(os, tree.root);
    return os;
}