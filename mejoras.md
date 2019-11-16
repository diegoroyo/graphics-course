# Added extensions

### Geometry: Use of `constexpr`
Matrix transform constructors have been optimized with `constexpr` so values can be known at compile time.

### Tone mapper: Use of Reinhard02 operator
Implemented Reinhard02 operator as seen in:
> _Reinhard, E., Stark, M., Shirley, P., & Ferwerda, J. (2002, July).  Photographic tone reproduction for digital images. In ACM transactions on graphics (TOG) (Vol. 21, No. 3, pp. 267-276). ACM._

### Tone mapper: Save images as PNG
Used own PNG image loader/saver to save images as PNG instead of PPM (passing `-p` parameter to the tonemapper)


### Geometrical primitives: Triangles and PLY model loading
Added triangle geometrical primitive for its use with PLY models.

### Ray tracing: acceleration structures
* **Bounding Volume Hierarchies:** `BVNode` figures consist of a bounding box and a children list. To intersect with one, the ray is first checked with the bounding box and only if it hits it's later checked with all the children.
* **Kd-Trees:** `KdTreeNode` figures consist of a bounding box and two children (who must also have their own bounding boxes). PLY models are divided into various regions forming a Kd-tree. The tree's children nodes are `BVNode` figures that contain all the corresponding triangles of that region.

PLY models are optimized and ordered into a Kd-tree with a number of subdivisions specified by the user. For each subdivision, the algorithm finds the bounding box's largest axis and divides it in two parts given that axis.

_TODO: document various examples of the improvement on rendering time_

### PLY models: added textures
PLY models also read texture data (for now only color data is read, which corresponds to the object's emission). Texture must be UV mapped on the PLY model data for this to work.