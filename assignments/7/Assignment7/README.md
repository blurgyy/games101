# Assignment 6

- 完成的分数: **60 分** (`60 = 5 + 20 + 15 + 20`)
  - `[5 分]` 包括了 `CMakeLists.txt` 和所有的程序文件, 所有的实验结果图片都保存在 `/images` 目录下.
  - `[45 分]` 正确实现了 Path Tracing 算法, 渲染结果图片为在 `/images/` 目录下的 [1.ppm](images/1.ppm)
  - `[加分项 10 分]` 多线程: 使用 OpenMP 实现了多线程, 将渲染的主循环的内层循环中的 `framebuffer` 索引计算方式改为本地计算, 解决了各线程更新 `framebuffer` 的冲突
  - `[加分项 10 分]` 在 Material.cpp 中实现了 Cook-Torrance Microfacet 模型. 修改了场景物体并提交了四种不同 `roughness` 下的渲染的结果.

- 提交结果说明:
  - 1.ppm 是原始场景渲染结果;
  - 2.ppm 到 5.ppm 是将地板和墙面改为 Microfacet 材质, short box 换成一个使用 Microfacet 材质的球, tall box 仍采用 Diffuse 材质, 并调整 Microfacet 的粗糙度分别为 `0.2`, `0.4`, `0.6`, `0.8` 的渲染结果.

- 提交结果信息:
  - [1.ppm](images/1.ppm):
    - 分辨率 `1080*1080`
    - 采样数 `256`
    - 计算时间 691 分钟 (g++ O2 优化, 1 核 2 线程)
  - [2.ppm](images/2.ppm), [3.ppm](images/3.ppm), [4.ppm](images/4.ppm), [5.ppm](images/5.ppm):
    - 分辨率 `784*784`
    - 采样数 `32`
    - 每张图片计算时间 9 分钟 (g++ O2 优化, 4 核 8 线程)

***

- 各个函数中实现的功能:
  - `castRay()`: 递归调用, 实现 Path Tracing. 如果着色点是光源则直接返回光源的颜色; 直接光部分, 先在场景中光源上采样(此处是均匀随机采样)出一个点并得到光源到着色点的概率密度, 然后判断光源到着色点之间是否被障碍物遮挡, 如果没有遮挡则根据着色点 BRDF 计算出此处的直接光照的贡献; 然后用 Russian Roulette 方式判断本次光线是否弹射, 如果判断结果为是, 再采样出一个光线的出射方向(实际是物理上的入射方向), 调用自身, 取返回值作为此处的间接光照. 最后返回直接光照与间接光照之和作为着色点的颜色.
  - `Material::eval()`: 添加了材质种类为 MICROFACET 时 BRDF 的计算. 使用 GGX 计算法线分布 D, 模型使用的是 Cook-Torrance, 即 `f_r = Kd * f_lambert + Ks * f_cook`.

***

附: 不同 roughness(0.2 - 0.8) 的微表面模型渲染结果比较 (四张渲染结果图中 tall box 均为 Diffuse 材质; 地板, 背墙和球面是微表面材质):

![comparison](images/comparison.jpg)
