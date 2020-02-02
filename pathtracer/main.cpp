// Scene descriptions:
// Scene 0: (final) Cornell box with basic event tests
// Scene 1: (final) Cornell box with PLY bunnies
// Scene 2: (final) Diamond ore wall (UVMaterial test)
// Scene 3: (final) Portal scene 1
// Scene 4: (final) Spaceships (UVMaterial test 2)
// Scene 5: (final) Portal loop
#ifndef SCENE_NUMBER
#define SCENE_NUMBER 0
#endif

#include <iostream>
#include <memory>
#include "camera/camera.h"
#include "camera/medium.h"
#include "io/plymodel.h"
#include "pathtracer.h"
#include "scene/figures.h"
#include "scene/material.h"
#include "scene/scene.h"
#include "scene/uvmaterial.h"

/// test purposes ///

bool out(const Vec4 &pos) {
    // scene 0 cornell box test purposes
    return pos.x < -5.01f || pos.x > 2.01f || pos.y < -2.01f || pos.y > 2.01f ||
           pos.z < -2.01f || pos.z > 2.01f;
}

void addDebugHits(FigurePtrVector &root, Ray ray, const EventPtr &event,
                  const MaterialPtr &material) {
    FigurePtr rootNode = FigurePtr(new Figures::BVNode(root));

    // Debug spheres on ray hit
    RayHit hit;
    FigurePtrVector extra;
    int n = 0;
    while (n < 1000 && rootNode->intersection(ray, hit) && !out(hit.point)) {
        extra.push_back(
            FigurePtr(new Figures::Sphere(material, hit.point, 0.05f)));
        std::cout << "Ray hits at " << hit.point << " w/ normal " << hit.normal
                  << std::endl;
        while (!event->nextRay(ray, hit, ray)) {
            std::cout << "Event's next ray is invalid. Trying again..."
                      << std::endl;
        }
        std::cout << "New ray comes from " << ray.origin << " w/ dir "
                  << ray.direction << std::endl;
        n++;
    }
    if (n != 1000) {
        std::cout << "Ray number " << n << " failed on point " << hit.point
                  << std::endl;
    }
    root.insert(root.end(), extra.begin(), extra.end());
}

/// test purposes ///

int main(int argc, char **argv) {
    if (argc < 9) {
        std::cerr << "Usage: " << argv[0]
                  << " -w <width> -h <height> -p <ppp> -o <out_ppm>"
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "-w Output image width" << std::endl;
        std::cerr << "-h Output image height" << std::endl;
        std::cerr << "-p Paths per pixel" << std::endl;
        std::cerr << "-o Output file (PPM format)" << std::endl;
        return 1;
    }

    // Read options
    int width, height, ppp;
    std::string filenameOut;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            width = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0) {
            height = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-p") == 0) {
            ppp = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-o") == 0) {
            filenameOut = argv[i + 1];
            i++;
        }
    }

    // Set up camera & scene

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 2
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 3
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(1.5f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 4
    Vec4 origin(-6.5f, 0.0f, 0.0f, 1.0f), forward(4.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 5
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.25f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#endif

    Film film(width, height, origin, forward, up);
    // film.setDoFRadius(0.015f);
    RayTracerPtr pathTracer = RayTracerPtr(new PathTracer(ppp, film));
    Camera camera(film, pathTracer);

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

    // Add elements to scene

#if SCENE_NUMBER == 1
    // load & transform spaceship model, get scene kdtree node
    UVMaterialPtr cyanUvTexture =
        UVMaterial::builder(1, 1)
            .addPhongDiffuse(RGBColor(0.1f, 0.45f, 0.45f))
            .addPhongSpecular(0.3f, 75.0f)
            // .addPerfectSpecular(0.9f)
            .build();
    UVMaterialPtr magentaUvTexture =
        UVMaterial::builder(1, 1)
            .addPhongDiffuse(RGBColor(0.45f, 0.1f, 0.45f))
            .addPhongSpecular(0.3f, 5.0f)
            // .addPerfectSpecular(0.9f)
            .build();
    UVMaterialPtr yellowUvTexture =
        UVMaterial::builder(1, 1)
            .addPhongDiffuse(RGBColor(0.45f, 0.45f, 0.1f))
            .addPhongSpecular(0.3f, 1000.0f)
            // .addPerfectSpecular(0.9f)
            .build();
    PLYModel cyanBunnyModel("ply/bunny.ply", cyanUvTexture);
    cyanBunnyModel.transform(
        Mat4::translation(0.8f, -2.0f, 0.0f) * Mat4::rotationY(M_PI_2 * -1.0f) *
        Mat4::rotationX(M_PI_2 * -1.0f) * Mat4::scale(2.0f, 2.0f, 2.0f));
    PLYModel magentaBunnyModel("ply/bunny_low.ply", magentaUvTexture);
    magentaBunnyModel.transform(Mat4::translation(1.0f, 1.0f, -2.0f));
    PLYModel yellowBunnyModel("ply/bunny_low.ply", yellowUvTexture);
    yellowBunnyModel.transform(Mat4::translation(1.0f, 0.5f, 2.0f) *
                               Mat4::rotationZ(M_PI) * Mat4::rotationY(M_PI) *
                               Mat4::scale(1.5f, 1.5f, 1.5f));
    FigurePtr cyanBunny = cyanBunnyModel.getFigure(12);
    FigurePtr magentaBunny = magentaBunnyModel.getFigure(12);
    FigurePtr yellowBunny = yellowBunnyModel.getFigure(12);
#elif SCENE_NUMBER == 4
    UVMaterialPtr spaceshipTexture =
        UVMaterial::builder(512, 512)
            .addPerfectSpecular("ply/spaceship_v2_specular.ppm")
            .addPhongDiffuse("ply/spaceship_v2_diffuse.ppm")
            .build();
    PLYModel spaceshipModel("ply/spaceship.ply", spaceshipTexture);
    spaceshipModel.transform(
        Mat4::translation(3.5f, -0.6f, -0.5f) * Mat4::rotationX(-0.07f) *
        Mat4::rotationY(M_PI_4 * 0.22f) * Mat4::rotationZ(M_PI_4 * -2.9f) *
        Mat4::scale(2.0f, 2.0f, 2.0f));
    PLYModel spaceshipModel2("ply/spaceship.ply", spaceshipTexture);
    spaceshipModel2.transform(
        Mat4::translation(7.0f, 1.5f, 1.25f) * Mat4::rotationX(0.05f) *
        Mat4::rotationY(M_PI_4 * 0.25f) * Mat4::rotationZ(M_PI_4 * -1.8f) *
        Mat4::scale(2.0f, 2.0f, 2.0f));
    FigurePtr spaceship = spaceshipModel.getFigure(4);
    FigurePtr spaceship2 = spaceshipModel2.getFigure(4);
#endif

    float maxLight = 10000.0f;

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 2 || SCENE_NUMBER == 3 || \
    SCENE_NUMBER == 5
    MaterialPtr whiteLight =
        Material::light(RGBColor(maxLight, maxLight, maxLight));
    MaterialPtr whitePhong = Material::builder()
                                 .add(phongDiffuse(RGBColor::White * 0.5f))
                                 .add(phongSpecular(0.3f, 3.0f))
                                 .build();
    MaterialPtr greenPhong = Material::builder()
                                 .add(phongDiffuse(RGBColor(0.1f, 0.5f, 0.1f)))
                                 .add(phongSpecular(0.3f, 3.0f))
                                 .build();
    MaterialPtr redPhong = Material::builder()
                               .add(phongDiffuse(RGBColor(0.5f, 0.1f, 0.1f)))
                               .add(phongSpecular(0.3f, 3.0f))
                               .build();
    MaterialPtr bluePhong = Material::builder()
                                .add(phongSpecular(0.5f, 1.0f))
                                .add(phongDiffuse(RGBColor(0.05f, 0.05f, 0.4f)))
                                .build();
    MaterialPtr magentaPhong =
        Material::builder()
            .add(phongSpecular(0.5f, 10.0f))
            .add(phongDiffuse(RGBColor(0.4f, 0.05f, 0.4f)))
            .build();
    MaterialPtr yellowPhong =
        Material::builder()
            .add(phongSpecular(0.5f, 100.0f))
            .add(phongDiffuse(RGBColor(0.4f, 0.4f, 0.05f)))
            .build();
    MaterialPtr cyanPhong = Material::builder()
                                .add(phongSpecular(0.5f, 1000.0f))
                                .add(phongDiffuse(RGBColor(0.05f, 0.4f, 0.4f)))
                                .build();
    MediumPtr glass = Medium::create(1.5f);
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.97f, glass)).build();
    MaterialPtr mirror =
        Material::builder().add(perfectSpecular(0.97f)).build();
    MaterialPtr pureBlack = Material::none();
#elif SCENE_NUMBER == 1
    MaterialPtr whitePhong = Material::builder()
                                 .add(phongDiffuse(RGBColor::White * 0.40f))
                                 .add(phongSpecular(0.3f, 3.0f))
                                 .build();
    MaterialPtr greenPhong = Material::builder()
                                 .add(phongDiffuse(RGBColor(0.1f, 0.40f, 0.1f)))
                                 .add(phongSpecular(0.3f, 3.0f))
                                 .build();
    MaterialPtr redPhong = Material::builder()
                               .add(phongDiffuse(RGBColor(0.40f, 0.1f, 0.1f)))
                               .add(phongSpecular(0.3f, 3.0f))
                               .build();
#elif SCENE_NUMBER == 4
    MaterialPtr base =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.9f)).build();
    MaterialPtr planet = Material::builder()
                             .add(phongDiffuse(RGBColor(0.1f, 0.3f, 0.3f)))
                             .add(phongSpecular(0.6f, 100.0f))
                             .build();
#endif
#if SCENE_NUMBER == 2
    UVMaterialPtr diamondTexture =
        UVMaterial::builder(16, 16)
            .addPhongDiffuse("ply/diamondore_diffuse.ppm")
            .addPhongDiffuse("ply/diamondore_emission.ppm")
            .addPhongSpecular("ply/diamondore_emission.ppm", 15.0f)
            .build();
    UVMaterialPtr stoneTexture = UVMaterial::builder(16, 16)
                                     .addPhongDiffuse("ply/stone_diffuse.ppm")
                                     .build();
    UVMaterialPtr greenWoolTexture =
        UVMaterial::builder(16, 16)
            .addPhongDiffuse("ply/greenwool_diffuse.ppm")
            .build();
    UVMaterialPtr redWoolTexture =
        UVMaterial::builder(16, 16)
            .addPhongDiffuse("ply/redwool_diffuse.ppm")
            .build();
    UVMaterialPtr lavaTexture =
        UVMaterial::builder(32, 32)
            .addPerfectSpecular(0.1f)  // add whatever (override lights)
            .build();
    lavaTexture->overrideLights("ply/lava_emission.ppm", maxLight);
#elif SCENE_NUMBER == 3
    MaterialPtr yellowWall = Material::builder()
                                 .add(phongSpecular(0.3f, 3.0f))
                                 .add(phongDiffuse(RGBColor(0.5f, 0.5f, 0.1f)))
                                 .build();
    MaterialPtr cyanWall = Material::builder()
                               .add(phongSpecular(0.3f, 3.0f))
                               .add(phongDiffuse(RGBColor(0.1f, 0.5f, 0.5f)))
                               .build();

    FigurePortalPtr bluePortal = FigurePortalPtr(
        new Figures::TexturedPlane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 1.99f, false));
    FigurePortalPtr orangePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(0.0f, 1.0f, 0.0f, 0.0f), -1.99f, false));

    UVMaterialPtr bluePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_blue_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", bluePortal, orangePortal)
            .build();
    bluePortalTexture->overrideLights("ply/portal_blue_diffuse2.ppm", maxLight,
                                      0.3f);
    bluePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);
    UVMaterialPtr orangePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_orange_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", orangePortal, bluePortal)
            .build();
    orangePortalTexture->overrideLights("ply/portal_orange_diffuse2.ppm",
                                        maxLight, 0.3f);
    orangePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);

    bluePortal->setUVMaterial(
        bluePortalTexture, Vec4(1.99f, -2.0f, -2.0f, 1.0f),
        Vec4(0.0f, 0.0f, 2.5f, 0.0f), Vec4(0.0f, 4.0f, 0.0f, 0.0f));
    orangePortal->setUVMaterial(
        orangePortalTexture, Vec4(-2.0f, -1.99f, -0.5f, 1.0f),
        Vec4(0.0f, 0.0f, 2.5f, 0.0f), Vec4(4.0f, 0.0f, 0.0f, 0.0f));
#elif SCENE_NUMBER == 5
    FigurePortalPtr bluePortal = FigurePortalPtr(
        new Figures::TexturedPlane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 1.99f, false));
    FigurePortalPtr orangePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(1.0f, 0.0f, 0.0f, 0.0f), -4.99f, false));

    UVMaterialPtr bluePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_blue_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", bluePortal, orangePortal)
            .build();
    bluePortalTexture->overrideLights("ply/portal_blue_diffuse2.ppm", maxLight,
                                      0.3f);
    bluePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);
    UVMaterialPtr orangePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_orange_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", orangePortal, bluePortal)
            .build();
    orangePortalTexture->overrideLights("ply/portal_orange_diffuse2.ppm",
                                        maxLight, 0.3f);
    orangePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);

    bluePortal->setUVMaterial(
        bluePortalTexture, Vec4(1.99f, -1.75f, -1.25f, 1.0f),
        Vec4(0.0f, 0.0f, 2.5f, 0.0f), Vec4(0.0f, 3.5f, 0.0f, 0.0f));
    orangePortal->setUVMaterial(
        orangePortalTexture, Vec4(-4.99f, -1.75f, 1.25f, 1.0f),
        Vec4(0.0f, 0.0f, -2.5f, 0.0f), Vec4(0.0f, 3.5f, 0.0f, 0.0f));
#endif

    // build scene to BVH root node
    FigurePtrVector sceneElements = {
#if SCENE_NUMBER == 0
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whitePhong),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenPhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redPhong),
        // Cornell box content
        sphere(bluePhong, Vec4(1.5f, 0.75f, -1.5f, 1.0f), 0.45f),
        sphere(magentaPhong, Vec4(1.5f, 0.75f, -0.5f, 1.0f), 0.45f),
        sphere(yellowPhong, Vec4(1.5f, 0.75f, 0.5f, 1.0f), 0.45f),
        sphere(cyanPhong, Vec4(1.5f, 0.75f, 1.5f, 1.0f), 0.45f),
        sphere(mirror, Vec4(1.0f, -1.25f, -1.0f, 1.0f), 0.75f),
        sphere(mirror, Vec4(1.0f, -1.25f, 1.0f, 1.0f), 0.75f),
        sphere(transparent, Vec4(0.5f, 0.5f, 0.0f, 1.0f), 0.5f)
#elif SCENE_NUMBER == 1
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whitePhong),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, redPhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, greenPhong),
        cyanBunny,
        magentaBunny,
        yellowBunny
#elif SCENE_NUMBER == 2
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, stoneTexture,
            Vec4(0.0f, 2.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
            Vec4(1.0f, 0.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, stoneTexture,
            Vec4(0.0f, -2.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, -1.0f, 0.0f),
            Vec4(-1.0f, 0.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), -1.99f, lavaTexture,
            Vec4(2.0f, -1.99f, 1.0f, 1.0f), Vec4(0.0f, 0.0f, -2.0f, 0.0f),
            Vec4(-2.0f, 0.0f, 0.0f, 0.0f), false)),
        // diamond ore wall
        FigurePtr(new Figures::TexturedPlane(
            Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, diamondTexture,
            Vec4(2.0f, 0.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
            Vec4(0.0f, 1.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenWoolTexture,
            Vec4(0.0f, 0.0f, 2.0f, 1.0f), Vec4(1.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 1.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redWoolTexture,
            Vec4(0.0f, 0.0f, -2.0f, 1.0f), Vec4(-1.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, -1.0f, 0.0f, 0.0f))),
        sphere(magentaPhong, Vec4(1.5f, -1.55f, -1.5f, 1.0f), 0.45f),
        sphere(yellowPhong, Vec4(1.1f, -1.6f, 1.5f, 1.0f), 0.4f),
        sphere(mirror, Vec4(0.5f, -1.6f, -1.3f, 1.0f), 0.4f),
        sphere(cyanPhong, Vec4(-0.5f, -1.65f, 1.4f, 1.0f), 0.35f)
#elif SCENE_NUMBER == 3
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, yellowWall),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, cyanWall),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, whitePhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenPhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redPhong),
        // Cornell box content
        bluePortal,
        orangePortal,
        sphere(magentaPhong, Vec4(-1.0f, -1.25f, -1.0f, 1.0f), 0.5f),
        sphere(mirror, Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.8f)
    // TODO simular objetos entrando-saliendo de portales
#elif SCENE_NUMBER == 4
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, base),
        sphere(planet, Vec4(-1.0f, 1.8f, -1.3f, 0.0f), 1.25f),
        spaceship,
        spaceship2
#elif SCENE_NUMBER == 5
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whitePhong),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whitePhong),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, whitePhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenPhong),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redPhong),
        // Cornell box content
        bluePortal,
        orangePortal,
        sphere(whitePhong, Vec4(-1.7f, -0.8f, 0.0f, 1.0f), 0.3f),
        sphere(mirror, Vec4(1.1f, -1.3f, -1.1f, 1.0f), 0.7f),
        sphere(transparent, Vec4(0.9f, 0.0f, 0.9f, 1.0f), 0.5f)
#endif
    };

    // Debug a random path
    // MaterialPtr debugSphere = Material::light(RGBColor::Cyan * maxLight);
    // EventPtr debugEvent = phongDiffuse(RGBColor::White);
    // addDebugHits(sceneElements, Ray(origin, forward.normalize(),
    // Medium::air),
    //              debugEvent, debugSphere);

    FigurePtr rootNode = FigurePtr(new Figures::BVNode(sceneElements));

#if SCENE_NUMBER == 4
    Scene scene(rootNode, RGBColor::White * 0.01f * maxLight, maxLight);
#else
    Scene scene(rootNode, RGBColor::Black, maxLight);
#endif

    // Add points lights to the scene

#if SCENE_NUMBER == 0
    scene.light(Vec4(1.0f, 1.7f, 0.0f, 1.0f), RGBColor::White * maxLight);
    scene.light(Vec4(0.0f, 1.7f, 1.0f, 1.0f), RGBColor::White * maxLight);
    scene.light(Vec4(0.0f, 1.7f, -1.0f, 1.0f), RGBColor::White * maxLight);
#elif SCENE_NUMBER == 1
    scene.light(Vec4(0.0f, 1.7f, 0.0f, 1.0f), RGBColor::White * maxLight);
#elif SCENE_NUMBER == 2
    scene.light(Vec4(0.0f, 1.4f, -1.0f, 1.0f), RGBColor::White * maxLight);
    scene.light(Vec4(0.0f, 1.4f, 1.0f, 1.0f), RGBColor::White * maxLight);
#elif SCENE_NUMBER == 3
    scene.light(Vec4(0.0f, 1.7f, 0.0f, 1.0f), RGBColor::White * maxLight);
#elif SCENE_NUMBER == 4
    scene.light(Vec4(1.0f, 4.0f, 0.0f, 1.0f), RGBColor::White * maxLight);
#elif SCENE_NUMBER == 5
    scene.light(Vec4(0.0f, 1.7f, 0.0f, 1.0f), RGBColor::White * maxLight);
#endif

#undef plane
#undef sphere

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

    // Generate render using argument options and save as PPM
    camera.tracePixels(scene);
    camera.storeResult(filenameOut);

    return 0;
}