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

### Ray intersections: acceleration structures
* **Bounding Volume Hierarchies:** `BVNode` figures consist of a bounding box and a children list. To intersect with one, the ray is first checked with the bounding box and only if it hits it's later checked with all the children.
* **Kd-Trees:** `KdTreeNode` figures consist of a bounding box and two children (who must also have their own bounding boxes). PLY models are divided into various regions forming a Kd-tree. The tree's children nodes are `BVNode` figures that contain all the corresponding triangles of that region.

PLY models are optimized and ordered into a Kd-tree with a number of subdivisions specified by the user. For each subdivision, the algorithm finds the bounding box's largest axis and divides it in two parts given that axis.

_TODO: document various examples of the improvement on rendering time_

### Multithreading support
Added multithreading support using a consumer scheme where each of the threads
consumes "work batches" (one image pixel at a time). The number of threads is
the same as the number of cores in the running host machine.

_TODO: document various examples of the imporvement on rendering time_

### Added texture mapping for different primitives
Planes and triangles (and thus, PLY models) can read texture data using
the operations defined in `UVMaterial`. An `UVMaterial` is a material matrix
which can be UV mapped to any object. Materials can be read from image data
so each part has different properties (diffuse, specular, refractive, etc.)

### Custom B(S/R/T?)DF: Portal
On intersections, a new kind of event can happen. It's called a "Portal"
event. A "Portal" event is defined by two bounded planes (they can be of
any size and in any position/rotation). Inspired by the videogame "Portal",
whenever a ray enters through one of the planes, it exits through the other
one.

It uses the portal's local coordinates to define the ray's outgoing
position and position. To define a portal, you must define an origin and two
vectors which define the bounded plane.

_TODO: add various images to explain portal effect_

### Separated Phong Diffuse event from Phong Specular
Because Phong Diffuse and Phong Specular prefer different sampling strategies,
the events have been separated (diffuse uses uniform cosine sampling,
specular uses phong's specular lobe sampling)

### Added Fresnel's equations to Delta BTDF (perfect refraction)
Whenever a perfect refraction event happens, the ray might end up reflecting
from the surface instead. The Fresnel equations provide the ratio of
refracted rays to reflected rays, depending on the angle of incidence and
the mediums' refractive index.