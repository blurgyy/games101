# Assignment 5

- 完成的分数: **30 分** 
  - `[5 分]` 包括了 `CMakeLists.txt` 和所有的程序文件, 所有的实验结果图片都保存在 `/images` 目录下. 
  - `[10 分]` 正确实现了光线生成, 并且能够看到图像中的两个球体. 
  - `[１5 分]` 正确实现了 Moller-Trumbore 算法, 并且能够看到图像中的地面. 

***

- 各个函数中实现的功能: 
  - `Renderer::Render()`: 遍历图像上的每个像素, 对于每个像素生成了一条穿过像素中心的光线, 然后将光线参数传递给函数 `castRay()` , 对当前像素进行上色. 
  - `rayTriangleIntersect()`: 实现了 Moller-Trumbore 算法求解 `t`, `b1`, `b2`, 判断三个值都是非负时, 更新 `tnear`, `u`, `v`. 

