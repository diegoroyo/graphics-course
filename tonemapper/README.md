# Tone Mapper

Tone Mapper capable of converting High Dynamic Range PPM images to Low Dynamic Range PNG or PPM images, while applying the specified tone mapping operator. It requires the `lib` modules included in the repository.

## Compilation

Compilation is easy using the `make` tool:

```bash
cd tonemapper
make -j
```

## Execution

```bash
Usage: tonemapper <ppm_image_in> [ -l ] -t <tone_mapper> [ -o <ppm_image_out> ] [ -p ]

Tone mapper options:
        -o Specify output file name (add .ppm/.png)
        -p Output as PNG instead of PPM
        -l Use LAB for color mapping instead of RGB
             Note: must go before -t parameter
        -t CLAMP_1
           EQUALIZE_CLAMP       <clamp_pct>
           CLAMP_GAMMA          <clamp_pct>     <gamma>
           REINHARD_02          <min_white>     <alpha>
             clamp_pct    Higher % luminance is clamped to white
             gamma        Gamma value for curve (default = 2.2)
             min_white    Similar to clamp_pct, for Reinhard02
             alpha        Image key (default = 0.18)
```