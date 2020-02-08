# Path Tracer

This Path Tracer uses Monte Carlo techniques to approximate the correct render result. It requires the `lib` modules included in the repository.

## Compilation

Compilation is easy using the `make` tool:

```bash
cd pathtracer
make -j
```

## Execution

You can see various examples on how to create a scene in the given `main` file. It contains multiple pre-built scenes, as seen in the main page.

```bash
Usage: pathtracer -w <width> -h <height> -p <ppp> -o <out_ppm>

-w Output image width
-h Output image height
-p Paths per pixel
-o Output file (PPM format)
```