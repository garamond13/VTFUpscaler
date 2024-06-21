## Usage
In order to use user shaders you can put them anywhere you want, just add the path to each of them in your `config.txt` (`user_shaders=`).

## Special notes for specific user shaders
<b/>Anime4K\Anime4K_Upscale_CNN_x2_UL.hlsl</b>  
Expects: denoise_filter=0, scale_factor=2, scale_filter=0  
If you want to use internal denoise before it you can change inside the shader `//!HOOK MAIN` into `//!HOOK DENOISE`.

<b/>Anime4K\Anime4K_Upscale_Denoise_CNN_x2_UL.hlsl</b>  
Expects: denoise_filter=0, scale_factor=2, scale_filter=0

## API documentation

### API version (//!API_V)
For which API version is shader made. Current version is 1, so it should be set as `//!API_V1`.

### Hooks (//!HOOK )
After which internal shader pass should your user shader run. Note that even if internal shader pass that you hook into doest runs, your user shader still will as if it was hooked to first previous shader pass.

`MAIN`
Main is a source image extracted from VTF file as R8G8B8A8_UNORM. It has an SRC width and heigth.

`DENOISE`
Denoise is an image after internal denoise filter. It is R32G32B32A32_FLOAT and it has an SRC width and heigth.

`SCALE`
Scale is an image after internal scale filter. It is R32G32B32A32_FLOAT and it has a DST width and heigth.

`SHARPEN`
Sharpen is an image after internal sharpen filter. It is R32G32B32A32_FLOAT and it has a DST width and heigth.

`GRAIN`
Grain is an image after internal sharpen filter. It is R32G32B32A32_FLOAT and it has a DST width and heigth.

### (Optional) User config (//!BEGIN_USER_CONFIG and //!END_USER_CONFIG)
User config should begin with `//!BEGIN_USER_CONFIG` and it has to end with `//!END_USER_CONFIG`.
The format of it is `key=value`. Keys are actually macros (`#define key value`) passed to every pass in the user shader so you can use keys in your code.

Comments that start with `//` and white spaces in user config section are fully supported.

## Passes (//!BEGIN_PASS and //!END_PASS)
Everithing inbetween `//!BEGIN_PASS` and `//!END_PASS` is compiled and later run as single shader pass. Passes are run in the same order as they appear in user shader.

## Pass output dimensions (//!WIDTH  and //!HEGTH )
Only options here are `SRC` (the source image dimension) and `DST` (the final image dimension, after upscale).

## (Optional) Saving and binding pass (//!SAVE  and //!BIND )
Saves pass result as shader resource that can be bind later by some other pass in the same user shader. The argument of `//!SAVE ` has to be an integer number larger than 0 cause last pass result will always be set as shader resource in slot 0.
You access the saved pass result by binding that same slot number with `//BIND `. Internaly pass result will be set as shader resource under that same slot number.

## Samplers
Point sampler is set in slot 0 and linear sampler is set in slot 1.

## Texcoord
Texcoord is given via struct `struct Vs_out { float4 position : SV_POSITION; float2 texcoord : TEXCOORD; };`

## Pass entry point
Every pass has to have an entry point `main` function as `float4 main() : SV_Target`.

## Tips
- Look into existing user shaders, how are they written.  
- Use `//` comments instead of `/**/`.