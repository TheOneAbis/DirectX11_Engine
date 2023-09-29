# DX11 Engine Source Code
Source Code for DX11 Engine project.

Interesting Code Examples:

- Terrain.h/.cpp and TerrainEntity.h/.cpp, as well as Helpers.h/.cpp (perlin noise for terrain generation)
- VS_Terrain.hlsl and Noise.hlsli (perlin noise vertex shader)
- MagicMirrorManager and associated shaders, responsible for rendering portals (I call them magic mirrors in here)

# A Brief Documentation on the Mirror (Portal) Rendering algorithm

Note: In this documentation I refer to portals as “mirrors” because in my head, portals are round, and here they're square and look more like mirrors.

## How to Think About Mirrors

Basically, I think of 2 magic mirrors like 2 sides of a window. The other side of the mirror that I am looking through would, as a result, look the same as if I were actually behind that other mirror, looking through it like a normal window. Thus, my line of thinking is: render the entire scene from the point of view of the destination mirror, then paste that over the source mirror I’m actually looking through. We will call these mirrors the “look-in” mirror (the one I am physically looking into) and the “look-out” mirror (the other side, which shows me what I would see through the look-in mirror).

## Calculating the View Matrix

The first thing that happens in my render loop is calculating this point of view from the look-out mirror. To do so, I simply transform my camera’s position into a local space relative to the look-in mirror’s origin by multiplying by the inverse of the look-in mirror’s world matrix, then transform it back into world space relative to the look-out mirror by multiplying by its regular world matrix.

There is a caveat to this, however. When looking at the look-in mirror, my camera would need to be in front of it. However, to see what’s on the other side, my imaginary camera would need to be behind the look-out mirror. Therefore I will need to “flip” the X and Z coords of my camera while it is in the look-in mirror’s local space, as seen below:

![image](https://github.com/TheOneAbis/DirectX11_Engine/assets/26350896/1f5ba4cb-d912-4434-9963-c007a0ab0251)

Next is computing the new forward and up axes of this imaginary camera. To do this requires a similar approach, but this time the main camera’s forward and up axes are rotated by the rotational difference between the look-in and look-out mirrors, which is done by calculating a single quaternion to represent this rotation. Multiplying the inverse of the look-in mirror’s transform rotation by the look-out mirror’s transform rotation will get us what we need (making sure to multiply the inverse look-in rotation quaternion by a 180-degree rotation around the y-axis first to account for the behind-the-mirror problem mentioned previously). Then, rotating the camera’s forward and up axes by this resulting quaternion will get the last couple vectors required for creating this imaginary camera’s view matrix.

## Rendering the Scene Through the Mirror

We can now re-render the scene using this new view matrix. However, some setup needs to be done first.
Figuring out how to only re-render part of the scene that can be seen through the mirror
Avoiding situations where objects between the look-out mirror and the imaginary camera may be drawn (we only want to be able to see things on the other side)

To solve the first issue, the mirrors themselves are drawn to a separate 2D texture, which I call the Mirror Map. This map’s dimensions must exactly match those of the viewport. The region on this map with the mirror is drawn white, while the rest remains black. This map can then be sent to a “Mirror View” version of the normal Pixel Shader. This new version of the normal pixel shader takes this new mirror map texture and tests the pixel’s screen position against it; if the pixel’s screen position matches a texel on the mirror map that is black (meaning it is not inside the mirror), then discard the pixel entirely. We don’t want to render it. Conversely, if it is white, meaning the pixel lies within the mirror, then we render it over the normal scene.

 ![image](https://github.com/TheOneAbis/DirectX11_Engine/assets/26350896/aa92c2be-3d19-459f-9f9e-35f41a734881)

We can now see through the mirror. However, the second issue needs to be addressed. The simple fix to this is to add another condition to rendering a pixel through the mirror: only render if its world position lies in front of this mirror’s plane. Basically, if the dot product between the vector from the mirror’s position to the pixel (normalized) and the mirror’s normal (going outward, so in my case just its forward axis) is less than 0, meaning it’s behind the mirror, discard it.
 
As seen below, parts of objects that are behind the look-out mirror are now culled.

![image](https://github.com/TheOneAbis/DirectX11_Engine/assets/26350896/3bb365da-0bcc-46e8-9b52-4ef5a4921f2d)

## Rendering Mirrors Through Mirrors

The fun part (the hard part). This took some time to figure out, but the best way to think of this for me is to move the imaginary camera, rather than the mirrors themselves, when rendering them through other mirrors. To do this, I basically take the vector between the imaginary and actual cameras and rotate it by the rotational difference between the two mirrors. Then I add that onto the imaginary camera to calculate the new position for rendering the inside mirror. As for the new forward and up axes of the imaginary camera, this is no different from how I’m already doing it; rotate them again by the rotational difference between the two mirrors. It is fortunate that this rotation quaternion was already calculated at the top, because it is very useful for this exact reason. Now, the new view matrix can be created from this.

As for actually rendering through the mirrors again with this new view matrix, it’s more of the same; create a mirror map, use it to re-render the scene. My implementation of this is through the use of two texture resources that I constantly alternate between with each iteration of my mirror rendering algorithm. Requiring a unique map for each mirror if I am rendering through 8 or more is a waste of resources, especially if I only need the map for one iteration. I can instead simply overwrite them as I render deeper through the mirrors.

## Conclusion

This is a quick run-down of how my mirror rendering algorithm works. I left out some of the finer details related to working with DirectX 11 specifically to get this working. I am also still in the process of figuring out how to render objects that are positioned partly through mirrors.

Here are some sketches representing potential outlier cases I am trying to account for (side-view):

![image](https://github.com/TheOneAbis/DirectX11_Engine/assets/26350896/3d341136-dff5-4659-94ee-800af106d3f3)

