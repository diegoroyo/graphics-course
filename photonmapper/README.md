# Photon Mapper

Photon Mapper based on Jensen's work [1]. It requires the `lib` modules included in the repository.

## Compilation

Compilation is easy using the `make` tool:

```bash
cd photonmapper
make -j
```

## Execution

You can see various examples on how to create a scene in the given `main` file. It contains multiple pre-built scenes, as seen in the main page.

```bash
Usage: photonmapper -w <width> -h <height> -p <ppp> -o <out_ppm>

-w Output image width
-h Output image height
-p Paths per pixel
-o Output file (PPM format)
```

## References

>_[1] Henrik Wann Jensen. Global illumination using photon maps. In Rendering Techniques’ 96, pages 21–30. Springer, 1996._