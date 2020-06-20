//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
    Eigen::Vector3f getColorBilinear(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        int u_dec = u_img;
        int v_dec = v_img;
        float u_coef = u_img - u_dec;
        float v_coef = v_img - v_dec;
        // printf("(%f, %f) -> (%d %d %f %f)\n", u, v, u_dec, v_dec, u_img, v_img);
        auto color_0 = image_data.at<cv::Vec3b>(v_dec, u_dec); // V first, U second
        auto color_1 = image_data.at<cv::Vec3b>(v_dec, u_dec+1);
        auto color_2 = image_data.at<cv::Vec3b>(v_dec+1, u_dec);
        auto color_3 = image_data.at<cv::Vec3b>(v_dec + 1, u_dec + 1);
        Eigen::Vector3f color;
        for(int i = 0; i < 3; ++ i){
            color[i] = (color_0[i] * (1.f - u_coef) + color_1[i] * u_coef) * (1.f - v_coef)
                    + (color_2[i] * (1.f - u_coef) + color_3[i] * u_coef) * v_coef;
        }
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
