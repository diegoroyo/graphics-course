#include "camera/camera.h"
#include "camera/film.h"
#include "camera/homambmedium.h"
#include "filter.h"
#include "math/geometry.h"
#include "photonemitter.h"
#include "photonmapper.h"
#include "scene/light.h"

void testKdTreeKNN() {
    // Simple KdTree KNearestNeighbours search
    Vec4 point(-5.0f, 5.0f, 0.0f, 0.0f);  // search knn of point

    // Create KdTree and show distances for validation
    PhotonKdTreeBuilder builder;
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
    for (const Photon* photon : photonsNN) {
        std::cout << photon->point << std::endl;
    }
}

int main(int argc, char** argv) {
    int width = 600;
    int height = 600;

    int photonsGlobal = 10000;
    int photonsCaustic = 10000;
    int photonsVolume = -1;
    bool storeDirectLight = false;
    int numRays = 10000;

    Vec4 origin(-4.5f, 0.0f, 0.0, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);

    Film film(width, height, origin, forward, up);
    // film.setDepthOfField(0.015f);
    PhotonEmitter emitter(photonsGlobal, storeDirectLight, numRays);
    emitter.setCaustic(photonsCaustic);
    // emitter.setVolume(photonsVolume);
    Medium::air = HomAmbMedium::create(1.0f, 0.6f, 0.3f, RGBColor::Cyan);

// shortcuts for getting figure pointers
#define plane(normal, dist, material) \
    FigurePtr(new Figures::FlatPlane(normal, dist, material))
#define sphere(material, pos, radius) \
    FigurePtr(new Figures::Sphere(material, pos, radius))

// shortcuts for materials
#define phongDiffuse(kd) EventPtr(new PhongDiffuse(kd))
#define phongSpecular(ks, alpha) EventPtr(new PhongSpecular(ks, alpha))
#define perfectSpecular(ksp) EventPtr(new PerfectSpecular(ksp))
#define perfectRefraction(krp, medium) \
    EventPtr(new PerfectRefraction(krp, medium))
    PerfectRefraction::disableFresnel();

    // Add elements to scene

    MediumPtr glass = Medium::create(1.5f);
    // MediumPtr glass = HomAmbMedium::create(1.5f, 0.4f, 0.2f, RGBColor::Cyan *
    // 100.0f);
    UVMaterialPtr transparentTexture =
        UVMaterial::builder(1, 1).addPerfectRefraction(0.99f, glass).build();
    PLYModel teapotModel("ply/teapot.ply", transparentTexture);
    teapotModel.transform(
        Mat4::translation(0.0f, -0.5f, 0.0f) * Mat4::rotationX(M_PI_2 * -1.0f) *
        Mat4::rotationZ(M_PI_2) * Mat4::scale(0.4f, 0.4f, 0.4f));
    FigurePtr teapot = teapotModel.getFigure(6);

    float maxLight = 5000.0f;

    UVMaterialPtr whiteLight =
        // add w/e event
        UVMaterial::builder(1, 1).addPerfectSpecular(0.5f).build();
    whiteLight->overrideLights("ply/guay.ppm", maxLight, 0.0f);
    MaterialPtr whiteDiffuse =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.8f)).build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.1f, 0.8f, 0.1f)))
            .build();
    MaterialPtr redDiffuse = Material::builder()
                                 .add(phongDiffuse(RGBColor(0.8f, 0.1f, 0.1f)))
                                 .build();
    MaterialPtr mirror = Material::builder().add(perfectSpecular(0.8f)).build();
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.8f, glass)).build();

    FigurePtr light = FigurePtr(new Figures::TexturedPlane(
        Vec4(0.0f, -1.0f, 0.0f, 0.0f), -1.99f, whiteLight,
        Vec4(1.0f, 1.99f, -1.0f, 0.0f), Vec4(0.0f, 0.0f, 2.0f, 0.0f),
        Vec4(-2.0f, 0.0f, 0.0f, 0.0f), false));

    // build scene to BVH root node
    FigurePtrVector sceneElements = {
        // Cornell box walls
        // light,
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, whiteDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redDiffuse),
        // Cornell box content
        // teapot
        sphere(mirror, Vec4(1.0f, -1.25f, -1.0f, 1.0f), 0.75f),
        sphere(transparent, Vec4(0.0f, -1.35f, 1.0f, 1.0f), 0.6f)};

    FigurePtr rootNode = FigurePtr(new Figures::BVNode(sceneElements));
    Scene scene(rootNode, RGBColor::Black, maxLight);

    // Add points lights to the scene
    scene.light(Vec4(0.0f, 0.0f, 0.0f, 0.0f), RGBColor::White * maxLight);

#undef plane
#undef sphere

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

    // emitter.emitPointLights(scene, Medium::air);
    emitter.emitAreaLight(scene, light, RGBColor::White * maxLight,
                          Medium::air);

    // Generate debug image
    // PPMImage debug = emitter.debugPhotonsImage(film, true, false, false);
    // debug.writeFile("out/map.ppm");

    FilterPtr filter = FilterPtr(new Filter());
    RayTracerPtr mapper =
        RayTracerPtr(new PhotonMapper(1, film, emitter, 500, 100, 0, filter));
    Camera camera(film, mapper);
    camera.tracePixels(scene);
    camera.storeResult("out/map.ppm");

    return 0;
}