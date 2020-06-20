#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_rotation(Eigen::Vector3f axis, float rotation_angle)
{
    float ang = rotation_angle / 180.f * MY_PI;
    float cos_ang = std::cos(ang);
    float sin_ang = std::sin(ang);
    float nx = axis[0];
    float ny = axis[1];
    float nz = axis[2];
    Eigen::Matrix3f rot = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f comp1 = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f comp2 = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f comp3 = Eigen::Matrix3f::Identity();
    comp1 *= cos_ang;
    comp2 *= (1-cos_ang) * axis * axis.transpose();
    comp3 << 0, -nz, ny,
        nz, 0, -nx,
        -ny, nx, 0;
    comp3 *= sin_ang;
    rot = comp1 + comp2 + comp3;
    
    Eigen::Matrix4f rot_hom;
    rot_hom.block(0, 0, 3, 3) << rot;
    rot_hom.col(3).head(3) << 0, 0, 0;
    rot_hom.row(3) << 0, 0, 0, 1;

    return rot_hom;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    float ang = rotation_angle / 180.f * MY_PI;
    float cos_ang = std::cos(ang);
    float sin_ang = std::sin(ang);
    model << cos_ang, -sin_ang, 0, 0,
        sin_ang, cos_ang, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    // return get_rotation(Eigen::Vector3f({0, 0, 1}), rotation_angle);
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    
    Eigen::Matrix4f persp_ortho = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f ortho_scale = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f ortho_trans = Eigen::Matrix4f::Identity();
    persp_ortho << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        // 0, 0, -1, 0;
        0, 0, 1, 0;
    float fov = eye_fov / 180.f * MY_PI;
    // float top = std::tan(fov / 2) * zNear;
    float top = std::tan(fov / 2) * fabs(zNear);

    float right = top / aspect_ratio;
    ortho_scale << 1 / right, 0, 0, 0,
        0, 1 / top, 0, 0,
        // 0, 0, 2 / (zFar - zNear), 0,
        0, 0, 2 / fabs(zFar - zNear), 0,
        0, 0, 0, 1;
    ortho_trans << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -(zFar + zNear) / 2,
        0, 0, 0, 1;
    projection = ortho_scale * ortho_trans * persp_ortho;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        // r.set_model(get_rotation(Eigen::Vector3f({1, 1, 1}), angle));
        r.set_view(get_view_matrix(eye_pos));
        // r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27 && key != 'q') {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        // r.set_model(get_rotation(Eigen::Vector3f({1, 1, 1}), angle));
        r.set_view(get_view_matrix(eye_pos));
        // r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        // std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
            printf("angle: %f\n", angle);
        }
        else if (key == 'd') {
            angle -= 10;
            printf("angle: %f\n", angle);
        }
    }

    return 0;
}
