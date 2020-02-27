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

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    float ang = rotation_angle / 180 * MY_PI;
    float cos_ang = std::cos(ang);
    float sin_ang = std::sin(ang);
    model << cos_ang, -sin_ang, 0, 0, // z
        sin_ang, cos_ang, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    // model << cos_ang, 0, sin_ang, 0, // y
    //     0, 1, 0, 0,
    //     -sin_ang, 0, cos_ang, 0,
    //     0, 0, 0, 1;
    // model << 1, 0, 0, 0, // x
    //     0, cos_ang, -sin_ang, 0,
    //     0, sin_ang, cos_ang, 0,
    //     0, 0, 0, 1;
    // std::cout << model << std::endl;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    
    Eigen::Matrix4f persp_ortho = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f ortho_scale = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f ortho_trans = Eigen::Matrix4f::Identity();
    persp_ortho << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        0, 0, 1, 0;
        // 0, 0, -1, 0;
    float fov = eye_fov / 180 * MY_PI;
    float top = std::tan(fov / 2) * zNear;

    float right = top / aspect_ratio;
    ortho_scale << 1 / right, 0, 0, 0,
        0, 1 / top, 0, 0,
        0, 0, 2 / (zFar - zNear), 0,
        0, 0, 0, 1;
    ortho_trans << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -(zFar + zNear) / 2,
        0, 0, 0, 1;
    projection = ortho_scale * ortho_trans * persp_ortho;
    // char x;
    // std::cout << projection << std::endl;
    // std::cout << projection * Eigen::Vector4f(0,0,1,1) << std::endl;
    // scanf("%c", &x);
    // projection = persp_ortho;

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
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27 && key != 'q') {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

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
