Assignment 3-2: PathTracer
====================
Fanyu Meng

## Overview
In this project, we added more features to our path tracer in the 
previous project. The added features include enabling reflection and 
refraction,  rendering microfacet surfaces, environment light and depth 
of field.

## Part 1: Mirror and Glass Materials

We render mirror and glass materials by allowing light rays to be 
reflected and refracted during calculation. For mirror, we 
characterize mirror by setting its BRDF to constantly return the 
reflected ray w.r.t. surface normal. For glass BRDF, we return either 
the reflected ray or the refracted ray according to Schlick's 
reflection coefficient. The refraction direction is calculated using 
the Fresnel equation.

## Part 2: Microfacet Materials

We implement this feature with the microface BRDF by multiplying the 
Fresnel term, the shadowing-masking term, the normal distribution 
function (NDF) and some scalars. The Fresnel term is calculated using 
the refractive index and the extinction coefficient of the material, 
and the NDF is calculated using the Beckmann distribution. 

We also apply importance sampling to increase convergence rate. We do 
this by importance the half vector from the NDF and correctly scale 
the result. 

<div align="middle">
    <table width="100%" align="middle">
        <tr>
            <td align="middle">
                <img src="images/p2_dragon_alpha_0.5.png" width="100%"/>
                <figcaption align="middle">alpha=0.5</figcaption>
            </td>
            <td align="middle">
                <img src="images/p2_dragon_alpha_0.25.png" width="100%"/>
                <figcaption align="middle">alpha=0.25</figcaption>
            </td>
        </tr>
        <tr>
            <td align="middle">
                <img src="images/p2_dragon_alpha_0.05.png" width="100%"/>
                <figcaption align="middle">alpha=0.05</figcaption>
            </td>
            <td align="middle">
                <img src="images/p2_dragon_alpha_0.005.png" width="100%"/>
                <figcaption align="middle">alpha=0.005</figcaption>
            </td>
        </tr>
    </table>
</div>

As we can see, a higher alpha value means that the macro surface tends 
to be diffuse, and is glossy when alpha is small.

<div align="middle">
    <table width="100%" align="middle">
        <tr>
            <td align="middle">
                <img src="images/p2_bunny_hemisphere.png" width="100%"/>
                <figcaption align="middle">Cosine hemisphere sampling</figcaption>
            </td>
            <td align="middle">
                <img src="images/p2_bunny_importance.png" width="100%"/>
                <figcaption align="middle">Importance sampling</figcaption>
            </td>
        </tr>
    </table>
</div>

The image rendered with importance sampling much less noisy since it 
converges much faster than uniform sampling.

<div align="middle">
    <img src="images/p2_dragon_co.png" width="60%"/>
    <figcaption align="middle">
        Rendering metal cobalt using a different set of eta and k parameters.
    </figcaption>
</div>

## Part 3: Environment Light

The idea of environment light is that the environmental texture is 
considered as light sources infinitely far away and is mapped onto the 
materials. This way we can have a more realistic scene as if the 
objects are reflection the world.

We implement environment light by sampling from the environment 
texture from each directions. The 3D direction is converted into 2D 
texture coordinate and the corresponding value is recorded. We also 
incorporate importance sampling by sampling brighter points with a 
higher probability. We do this by pre-calculating the marginal 
environment light density for each pixel. 

<div align="middle">
    <table width="100%" align="middle">
        <tr>
            <td align="middle">
                <img src="images/p3_envmap.png" width="100%"/>
                <figcaption align="middle">Used environment map</figcaption>
            </td>
            <td align="middle">
                <img src="images/p3_probability_debug.png" width="100%"/>
                <figcaption align="middle">Corresponding probability CDF</figcaption>
            </td>
        </tr>
    </table>
</div>

## Part 4: Depth of Field

For a pinhole camera, all light received by the camera comes from the 
direction of the pinhole. However, this might not the be case for a 
thin lens camera. Light could be distorted by the rim of the thin 
lens and come from different directions. This created the idea of 
depth of field, that only objects in a certain range of distance are 
on focus, and other objects are blurred in the image.
