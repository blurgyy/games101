# Assignment 2

- 是否完成提高题：是

- 在各个函数中实现的功能：

  - `insideTriangle` ：
    输入参数为两个浮点数 `x` 和 `y` 和一个 `Eigen::Vector3f` 数组 `*_v` ，返回一个 `bool`代表屏幕上的点 `(x, y)` 是否在 `*_v` 所表示的三角形在屏幕上的投影内。

  - `rasterize_triangle()` ：
    函数首先创建了输入三角形的 bounding box，然后遍历 bounding box 内所有像素，对于每个像素，判断其中每一个"小像素"的中心是否在三角形在屏幕上的投影内，如果是，再判断小像素中心处的插值深度值是否比 `depth_buf` 中的深度更小，如果更小则更新 `depth_buf` 并将这一"小像素"的颜色设置为当前三角形的颜色。遍历完所有像素后，将每个像素的颜色设置为该像素内四个"小像素"的平均

- 输出的图像分别是 [output.png](output.png) (无MSAA) 和 [output_aa.png](output_aa.png) (MSAA 2x)

