## Usage

In order to use user shaders you can put them anyware you want, just add path to each of them in your `config.txt` (`user_shaders=`).

## Special notes for specific user shaders

<b/>Anime4K\Anime4K_Upscale_CNN_x2_UL.hlsl</b>  
Expects: denoise_filter=0, scale_factor=2, scale_filter=0  
If you want to use internal denoise before it you can change inside the shader `//!HOOK MAIN` into `//!HOOK DENOISE`.

<b/>Anime4K\Anime4K_Upscale_Denoise_CNN_x2_UL.hlsl</b>  
Expects: denoise_filter=0, scale_factor=2, scale_filter=0