//
// Photon mapper - Informática gráfica 2019-20
// Diego Royo (740388@unizar.es)
//
// Modified for render contest image generation
//

#include "camera/camera.h"
#include "camera/film.h"
#include "camera/homambmedium.h"
#include "filter.h"
#include "homisomedium.h"
#include "math/geometry.h"
#include "photonemitter.h"
#include "photonmapper.h"
#include "scene/light.h"

int main(int argc, char** argv) {

    // PART 4: Add all three images
    // PPMImage base, blue, orange;
    // base.readFile("out/final/base.ppm");
    // blue.readFile("out/final/blue.ppm");
    // orange.readFile("out/final/orange.ppm");
    // base.addImage(blue);
    // base.addImage(orange);
    // base.writeFile("out/map.ppm");
    // return 0;

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

    /// Paritcipative media configuration ///
    Medium::air = HomIsoMedium::create(  // homogeneous isotropic
        1.0f, 0.2f, 0.07f,  // refractive index, extinction, scattering
        0.6f                // delta_d for ray marching
    );
    /// Participative media configuration ///

    /// Emitting configuration ///
    int photonsGlobal = 3500;
    bool useCausticMap = true;
    int photonsCaustic = 2000;
    bool useVolumeMap = true;
    int photonsVolume = 10000;
    int numRays = 4000;
    bool storeDirectLight = true;
    /// Emiting configuration ///

    /// Estimating configuration ///
    bool debugGlobal = false;  // overrides photon mapping
    bool debugCaustic = false;
    bool debugVolume = false;
    int kGlobal = 80;  // kNN search for maps
    int kCaustic = 50;
    int kVolume = 75;
    FilterPtr filter(new Filter());
    // FilterPtr filter(new ConeFilter());
    /// Estimating configuration ///

    // normal view
    Vec4 origin(-4.5f, 2.5f, 2.5f, 1.0f), forward(2.0f, 0.0f, -0.7f, 0.0f),
        up(0.0f, 2.0f, 0.0f, 0.0f);
    // room view
    // Vec4 origin(0.0f, 6.0f, -4.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
    //     up(0.0f, 2.0f, 0.0f, 0.0f);
    // back view
    // Vec4 origin(-12.5f, 2.5f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
    //     up(0.0f, 3.0f, 0.0f, 0.0f);

    // Configure scene
    Film film(width, height, origin, forward, up);
    PhotonEmitter emitter(photonsGlobal, storeDirectLight, numRays);
    if (useCausticMap) {
        emitter.setCaustic(photonsCaustic);
    }
    if (useVolumeMap) {
        emitter.setVolume(photonsVolume);
    }
    // Maximum light value (use this instead of constant numbers)
    float maxLight = 5000.0f;
    // Disable fresnel events for russian roulette (less noise for low ppp)
    // PerfectRefraction::disableFresnel();

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

    // Models (maybe with scattering events)
    MediumPtr mediumGlass = Medium::create(1.0f);
    MediumPtr medium1 = Medium::create(1.0f);
    UVMaterialPtr wallTexture = UVMaterial::builder(420, 420)
                                    .addPhongDiffuse("ply/portalwall.ppm")
                                    .build();
    UVMaterialPtr floorTexture = UVMaterial::builder(420, 420)
                                     .addPhongDiffuse("ply/floor_diffuse.ppm")
                                     .build();
    UVMaterialPtr ceilingTexture =
        UVMaterial::builder(2048, 2560)
            .addPhongDiffuse("ply/ceiling_diffuse.ppm")
            .build();
    UVMaterialPtr wallWindowTexture =
        UVMaterial::builder(1050, 840)
            .addPhongDiffuse("ply/window_diffuse.ppm")
            .addPerfectRefraction("ply/window_refraction.ppm", medium1)
            .build();
    UVMaterialPtr windowBackTexture =
        UVMaterial::builder(420, 420)
            .addPhongDiffuse("ply/window_back_diffuse.ppm")
            .build();
    UVMaterialPtr signTexture = UVMaterial::builder(512, 1024)
                                    .addPhongDiffuse("ply/signtest.ppm")
                                    .build();

    // Normal cubes
    UVMaterialPtr normalCubeTex = UVMaterial::builder(512, 512)
                                      .addPhongDiffuse("ply/cube_diffuse.ppm")
                                      .build();
    PLYModel normalCubeModel("ply/cube.ply", normalCubeTex);
    normalCubeModel.transform(Mat4::translation(0.5f, 0.7f, -0.7f) *
                              Mat4::rotationY(M_PI_4 * 1.5f) *
                              Mat4::scale(0.035f, 0.035f, 0.035f));
    FigurePtr normalCube = normalCubeModel.getFigure(10);
    PLYModel normalCubeModel2("ply/cube.ply", normalCubeTex);
    normalCubeModel2.transform(Mat4::translation(0.3f, 2.1f, -0.8f) *
                               Mat4::rotationY(M_PI_4 * 2.1f) *
                               Mat4::scale(0.035f, 0.035f, 0.035f));
    FigurePtr normalCube2 = normalCubeModel2.getFigure(10);
    PLYModel normalCubeModel3("ply/cube.ply", normalCubeTex);
    normalCubeModel3.transform(Mat4::translation(0.4f, 0.7f, -2.4f) *
                               Mat4::rotationY(M_PI_4 * 1.7f) *
                               Mat4::scale(0.035f, 0.035f, 0.035f));
    FigurePtr normalCube3 = normalCubeModel3.getFigure(10);

    // Companion cube
    UVMaterialPtr companionCubeTex =
        UVMaterial::builder(512, 512)
            .addPhongDiffuse("ply/companion_cube_diffuse.ppm")
            .build();
    PLYModel companionCubeModel("ply/cube.ply", companionCubeTex);
    companionCubeModel.transform(Mat4::translation(-3.0f, 0.72f, -4.0f) *
                                 Mat4::rotationY(M_PI_4 * 2.5f) *
                                 Mat4::scale(0.035f, 0.035f, 0.035f));
    FigurePtr companionCube = companionCubeModel.getFigure(10);

    // Refraction cube
    UVMaterialPtr refractionCubeTex =
        UVMaterial::builder(512, 512)
            .addPhongDiffuse("ply/lasercube_diffuse.ppm")
            .addPerfectRefraction("ply/lasercube_refraction.ppm", mediumGlass)
            .build();
    PLYModel refractionCubeModel("ply/lasercube.ply", refractionCubeTex);
    refractionCubeModel.transform(Mat4::translation(-1.8f, 0.7f, -2.9f) *
                                  Mat4::rotationY(M_PI_4 * -1.5f) *
                                  Mat4::scale(0.035f, 0.035f, 0.035f));
    FigurePtr refractionCube = refractionCubeModel.getFigure(10);

    // Teapot
    UVMaterialPtr teapotTex = UVMaterial::builder(1, 1)
                                  .addPhongDiffuse(RGBColor(0.85f, 0.2f, 0.02f))
                                  .build();
    PLYModel teapotModel("ply/teapot.ply", teapotTex);
    teapotModel.transform(Mat4::translation(-0.45f, 1.35f, -3.5f) *
                          Mat4::rotationX(M_PI_2 * -1.0f) *
                          Mat4::rotationZ(M_PI_4 * -1.0f) *
                          Mat4::scale(0.3f, 0.3f, 0.3f));
    FigurePtr teapot = teapotModel.getFigure(10);

    // Button
    UVMaterialPtr buttonTex = UVMaterial::builder(1024, 1024)
                                  .addPhongDiffuse("ply/button_diffuse.ppm")
                                  .build();
    PLYModel buttonModel("ply/button.ply", buttonTex);
    buttonModel.transform(Mat4::translation(-3.5f, 2.1f, -1.0f) *
                          Mat4::rotationX(M_PI_2 * -0.999f) *
                          Mat4::scale(0.03f, 0.035f, 0.03f));
    FigurePtr button = buttonModel.getFigure(10);

    // Portals
    FigurePortalPtr bluePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(0.0f, 0.0f, -1.0f, 0.0f), -3.99f, false));
    FigurePortalPtr orangePortal = FigurePortalPtr(
        new Figures::TexturedPlane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 0.01f, false));

    UVMaterialPtr bluePortalTexture =
        UVMaterial::builder(512, 512)
            .addPortal("ply/portal_any_portal2.ppm", bluePortal, orangePortal)
            .build();
    bluePortalTexture->overrideLights("ply/portal_blue_diffuse2.ppm",
                                      maxLight * 0.001f, 0.3f);
    bluePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);
    UVMaterialPtr orangePortalTexture =
        UVMaterial::builder(512, 512)
            .addPortal("ply/portal_any_portal2.ppm", orangePortal, bluePortal)
            .build();
    orangePortalTexture->overrideLights("ply/portal_orange_diffuse2.ppm",
                                        maxLight * 0.001f, 0.3f);
    orangePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);

    bluePortal->setUVMaterial(bluePortalTexture, Vec4(1.75f, 0.0f, 3.99f, 1.0f),
                              Vec4(-2.0f, 0.0f, 0.0f, 0.0f),
                              Vec4(0.0f, 4.0f, 0.0f, 0.0f));
    orangePortal->setUVMaterial(
        orangePortalTexture, Vec4(0.0f, 0.01f, -1.0f, 1.0f),
        Vec4(-2.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 4.0f, 0.0f));

    // Materials for walls
    MaterialPtr whiteLight =
        Material::light(RGBColor(maxLight, maxLight, maxLight));
    UVMaterialPtr whiteAreaLight = UVMaterial::fill(1, 1, whiteLight);
    UVMaterialPtr mirrorArea = UVMaterial::builder(1, 1)
                                   .addPhongDiffuse(RGBColor(0.1f, 0.2f, 0.5f))
                                   .addPhongSpecular(0.49f, 3.0f)
                                   .build();
    UVMaterialPtr mirrorAreaDiffuse = UVMaterial::builder(1, 1)
                                   .addPhongDiffuse(RGBColor(0.5f, 0.6f, 0.8f))
                                   .build();

    MaterialPtr whiteDiffuse =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.85f)).build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.085f, 0.85f, 0.085f)))
            .build();
    MaterialPtr redDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.85f, 0.085f, 0.085f)))
            .build();
    MaterialPtr orangeDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.85f, 0.6f, 0.02f)))
            .build();
    MaterialPtr bluePhong = Material::builder()
                                .add(phongSpecular(0.3f, 10.0f))
                                .add(phongDiffuse(RGBColor(0.05f, 0.2f, 0.6f)))
                                .build();
    MaterialPtr yellowPhong =
        Material::builder()
            .add(phongSpecular(0.3f, 100.0f))
            .add(phongDiffuse(RGBColor(0.6f, 0.6f, 0.05f)))
            .build();
    MaterialPtr mirror =
        Material::builder().add(perfectSpecular(0.85f)).build();

    // Trasparent medium (maybe with events, see above)
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.85f, mediumGlass)).build();

    // Rectangle area light on top of cornell box
    FigurePtr light = FigurePtr(new Figures::TexturedPlane(
        Vec4(0.0f, -1.0f, 0.0f, 0.0f), -1.99f, whiteAreaLight,
        Vec4(0.5f, 1.99f, -0.5f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
        Vec4(-1.0f, 0.0f, 0.0f, 0.0f), false));
    FigurePtr signLevel = FigurePtr(new Figures::TexturedPlane(
        Vec4(0.0f, 0.0f, -1.0f, 0.0f), -3.99f, signTexture,
        Vec4(-0.75f, 0.5f, 3.99f, 1.0f), Vec4(-2.0f, 0.0f, 0.0f, 0.0f),
        Vec4(0.0f, 4.0f, 0.0f, 0.0f), false));
    UVMaterialPtr signOneTex = UVMaterial::builder(1392, 1536)
                                   .addPhongDiffuse("ply/cartel_1.ppm")
                                   .build();
    FigurePtr signOne = FigurePtr(new Figures::TexturedPlane(
        Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -1.99f, signOneTex,
        Vec4(1.99f, 2.5f, 0.5f, 1.0f), Vec4(0.0f, 0.0f, 2.5f, 0.0f),
        Vec4(0.0f, 2.8f, 0.0f, 0.0f), false));
    UVMaterialPtr signTwoTex = UVMaterial::builder(1600, 1000)
                                   .addPhongDiffuse("ply/cartel_2.ppm")
                                   .build();
    FigurePtr signTwo = FigurePtr(new Figures::TexturedPlane(
        Vec4(0.0f, 0.0f, 1.0f, 0.0f), -5.99f, signTwoTex,
        Vec4(-2.25f, 2.25f, -5.99f, 1.0f), Vec4(2.5f, 0.0f, 0.0f, 0.0f),
        Vec4(0.0f, 2.0f, 0.0f, 0.0f), false));

    // Build scene to BVH root node
    FigurePtrVector sceneElements = {
        // Cornell box walls
        // Front wall
        FigurePtr(new Figures::TexturedPlane(
            Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -2.0f, wallWindowTexture,
            Vec4(2.0f, 0.0f, -6.0f, 1.0f), Vec4(0.0f, 0.0f, 10.0f, 0.0f),
            Vec4(0.0f, 8.0f, 0.0f, 0.0f), false)),
        // Back wall
        FigurePtr(new Figures::TexturedPlane(
            Vec4(1.0f, 0.0f, 0.0f, 0.0f), -6.0f, wallTexture,
            Vec4(-6.0f, -4.0f, -4.0f, 1.0f), Vec4(0.0f, 0.0f, 4.0f, 0.0f),
            Vec4(0.0f, 4.0f, 0.0f, 0.0f), true)),
        // Right wall
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), 4.0f, wallTexture,
            Vec4(4.0f, -4.0f, 4.0f, 1.0f), Vec4(-4.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 4.0f, 0.0f, 0.0f), true)),
        // Left wall
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), -6.0f, wallTexture,
            Vec4(4.0f, -4.0f, -6.0f, 1.0f), Vec4(-4.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 4.0f, 0.0f, 0.0f), true)),
        // Floor
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), 0.0f, floorTexture,
            Vec4(0.0f, 0.0f, 0.0f, 1.0f), Vec4(2.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 0.0f, 2.0f, 0.0f), true)),
        // Ceiling
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), 8.0f, ceilingTexture,
            Vec4(2.0f, 8.0f, -6.0f, 1.0f), Vec4(-8.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 0.0f, 10.0f, 0.0f), false)),
        // Back room
        // bottom/top
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), 4.01f, mirrorArea,
            Vec4(2.0f, 4.01f, -6.0f, 1.0f), Vec4(8.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 0.0f, 4.0f, 0.0f), false)),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), 9.0f, mirrorAreaDiffuse,
            Vec4(2.0f, 9.0f, -6.0f, 1.0f), Vec4(8.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 0.0f, 4.0f, 0.0f), false)),
        // left/right
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), -5.99f, mirrorAreaDiffuse,
            Vec4(2.0f, 4.0f, -5.99f, 1.0f), Vec4(8.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 8.0f, 0.0f, 0.0f), false)),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.01f, mirrorArea,
            Vec4(2.0f, 4.0f, -2.01f, 1.0f), Vec4(8.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 8.0f, 0.0f, 0.0f), false)),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -10.0f, mirrorArea,
            Vec4(10.0f, 4.0f, -6.0f, 1.0f), Vec4(0.0f, 0.0f, 4.0f, 0.0f),
            Vec4(0.0f, 8.0f, 0.0f, 0.0f), false)),
        // Items
        signLevel, teapot, normalCube, normalCube2, normalCube3, refractionCube,
        companionCube, button, bluePortal, orangePortal, signOne, signTwo,
        sphere(mirror, Vec4(0.7f, 3.5f, -0.7f, 1.0f), 0.8f),
        sphere(bluePhong, Vec4(0.7f, 1.9f, -2.2f, 1.0f), 0.5f)};

    FigurePtr rootNode = FigurePtr(new Figures::BVNode(sceneElements));
    Scene scene(rootNode, RGBColor::Black, maxLight);

#undef plane
#undef sphere

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

    // PART 1: General light
    // // room light
    // scene.light(Vec4(5.99f, 6.5f, -2.5f, 0.0f), RGBColor::White * maxLight);
    // // top of level sign
    // scene.light(Vec4(-1.5f, 4.75f, 3.8f, 0.0f),
    //             RGBColor::White * maxLight * 0.008f);
    // // top of sign one
    // scene.light(Vec4(1.9f, 5.5f, 1.75f, 0.0f),
    //             RGBColor::White * maxLight * 0.005f);
    // emitter.emitPointLights(scene, Medium::air);

    // PART 2: Blue portal glow
    // emitter.emitAreaLight(
    //     scene, bluePortal,
    //     RGBColor(61.0f, 173.0f, 240.0f) * maxLight * 0.000025f, Medium::air);

    // PART 3: Orange portal glow
    emitter.emitAreaLight(
        scene, orangePortal,
        RGBColor(240.0f, 132.0f, 60.0f) * maxLight * 0.000025f, Medium::air);

    if (debugGlobal || debugCaustic || debugVolume) {
        // Project photons to viewport to view stored photons
        PPMImage debug = emitter.debugPhotonsImage(film, debugGlobal,
                                                   debugCaustic, debugVolume);
        debug.writeFile(filenameOut.c_str());
    } else {
        // Radiance estimate step, save image
        RayTracerPtr mapper = RayTracerPtr(new PhotonMapper(
            ppp, film, emitter, kGlobal, kCaustic, kVolume, filter));
        Camera camera(film, mapper);
        camera.tracePixels(scene);
        camera.storeResult(filenameOut);
    }

    return 0;
}