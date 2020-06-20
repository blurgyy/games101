# Assignment 3

- 完成的分数: **55 分** (`55 = 5 + 10 + 20 + 5 + 10 + 5`)
  - `[5 分]` 包括了 `CMakeLists.txt` 和所有的程序文件，所有的实验结果图片都保存在 `/images` 目录下
  - `[10 分]` 正确对参数进行了插值，并将参数传递给了 `fragment_shader_payload` 
  - `[20 分]` 正确实现了 Blinn-Phong 反射模型，结果图片保存在 [`images/phong.png`](images/phong.png) 
  - `[5 分]` 在以上 Blinn-Phong 反射模型的基础上， 正确实现了 Texture Mapping，结果图片为 [`images/texture.png`](images/texture.png) 
  - `[10 分]` 正确实现了 Bump Mapping 和 Displacement Mapping ，结果图片分别为 [`images/bump.png`](images/bump.png) 和 [`images/displacement.png`](images/displacement.png) 
  - `[Bonus 5 分]` 在 `Texture` 类中实现了方法 `Vector3f getColorBilinear(float u, float v)` 并调用。为了显示出插值的效果，使用了大小为 256x256 的纹理，得到的结果图片分别为: 
    - [`images/texture256.png`](images/texture256.png) (未插值)
    - [`images/texture256_bilinear.png`](images/texture256_bilinear.png) (双线性插值) 
    - [`images/comp.jpg`](images/comp.jpg) (两图横向拼接，用于对比)
      未做双线性插值的图片中，在两种颜色的边界处可以明显看到因为坐标 (u,v) 向下取整而出现的像素；做了双线性插值的图片中在颜色边缘有更加平滑的过渡。

***

- 各个函数中实现的功能: 
  - `rasterize_triangle`: 光栅化一个三角形，对三角形内部的点各个属性根据三角形顶点对应属性进行插值，传递给 fragment_shader 函数来获取像素颜色并进行深度测试，需要的话更新 frame buffer
  - `phong_fragment_shader`: 使用 Blinn-Phong 反射模型对于每个给定光源计算模型的漫反射项和高光项，最后将每个光源的反射项与环境光的叠加作为模型在该点的颜色。
  - `texture_fragment_shader`: 对于模型上给定的一点，获取纹理颜色作为该点的漫反射项系数 kd，然后使用 Blinn-Phong 反射模型计算并叠加每个光源的反射作为该点颜色。
  - `bump_fragment_shader`: 对于模型上给定的一点，根据 hmap 求出该点的法线，然后根据这一法线对模型上色。
  - `displacement_fragment_shader`: 对于模型上给定的一点，先沿着法线方向移动，然后重新计算该点法线，然后使用 Blinn-Phong 模型计算反射。
  - `getColorBilinear`: 查询纹理颜色时，使用双线性插值对查询的非整点根据周围的四个整点颜色进行插值并返回。



