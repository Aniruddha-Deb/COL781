#include "../src/a1.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <chrono>
#include <ctime>

// Program with perspective correct interpolation of vertex attributes.

namespace R = COL781::Software;
// namespace R = COL781::Hardware;

using namespace glm;
int main()
{
    R::Rasterizer r;
    int width = 800, height = 800;
    if (!r.initialize("Clock", width, height, 4))
        return EXIT_FAILURE;

    R::ShaderProgram program = r.createShaderProgram(r.vsTransform(), r.fsConstant());
    vec4 vertices[] = {vec4(-1.0, 1.0, 0.0, 1.0), vec4(1.0, 1.0, 0.0, 1.0), vec4(1.0, -1.0, 0.0, 1.0),
                       vec4(-1.0, -1.0, 0.0, 1.0)};
    // vec4 colors[] = {vec4(0.0, 0.4, 0.6, 1.0), vec4(1.0, 1.0, 0.4, 1.0), vec4(0.0, 0.4, 0.6, 1.0),
    //                  vec4(1.0, 1.0, 0.4, 1.0)};
    ivec3 triangles[] = {ivec3(0, 1, 3), ivec3(1, 2, 3)};
    R::Object shape = r.createObject();
    r.setVertexAttribs(shape, 0, 4, vertices);
    r.setTriangleIndices(shape, 2, triangles);
    r.enableDepthTest();
    // The transformation matrix.

    mat4 screen_scaling =
        scale(mat4(1.0f), vec3(min(height, width) / float(width), min(height, width) / float(height), 1.0f));
    float tick_width = 0.02f;
    float tick_height = 3 * tick_width;
    float radius = tick_width * 40;
    float small_tick_width = 0.4 * tick_width;
    float small_tick_height = 0.25 * tick_height;
    float hour_hand_height = tick_height * 6;
    float hour_hand_width = tick_width * 1.5;
    float minute_hand_height = tick_height * 9;
    float minute_hand_width = tick_width;
    float second_hand_width = small_tick_width;
    float second_hand_height = tick_height * 8;
    mat4 tick_scaling = scale(mat4(1.0f), vec3(tick_width, tick_height, 1.0f));
    mat4 tick_translation = translate(mat4(1.0f), vec3(0, radius, 0));

    mat4 small_tick_scaling = scale(mat4(1.0f), vec3(small_tick_width, small_tick_height, 1.0f));
    mat4 small_tick_translation = translate(mat4(1.0f), vec3(0, radius + tick_height / 2 + small_tick_height, 0));

    mat4 hour_scaling = scale(mat4(1.0f), vec3(hour_hand_width, hour_hand_height, 1.0f));
    mat4 hour_translation = translate(mat4(1.0f), vec3(0, radius / 2 - hour_hand_height / 4, 0));
    mat4 minute_scaling = scale(mat4(1.0f), vec3(minute_hand_width, minute_hand_height, 1.0f));
    mat4 minute_translation = translate(mat4(1.0f), vec3(0, radius / 2 - minute_hand_height / 5, 0));
    mat4 second_scaling = scale(mat4(1.0f), vec3(second_hand_width, second_hand_height, 1.0f));
    mat4 second_translation = translate(mat4(1.0f), vec3(0, radius / 2 - second_hand_height / 4, 0));
    while (!r.shouldQuit())
    {
        r.clear(vec4(1.0, 1.0, 1.0, 1.0));
        r.useShaderProgram(program);
        // model = rotate(mat4(1.0f), radians(speed * time), vec3(1.0f, 0.0f, 0.0f));
        r.setUniform<vec4>(program, "color", vec4(0.0, 0.0, 0.0, 1.0));
        for (int i = 0; i < 12; i++)
        {
            mat4 tick_rotation = rotate(mat4(1.0f), radians(i * 30.0f), vec3(0.0f, 0.0f, 1.0f));
            r.setUniform(program, "transform", screen_scaling * tick_rotation * tick_translation * tick_scaling);
            r.drawObject(shape);
        }
        for (int i = 0; i < 60; i++)
        {
            mat4 small_tick_rotation = rotate(mat4(1.0f), radians(i * 6.0f), vec3(0.0f, 0.0f, 1.0f));
            r.setUniform(program, "transform",
                         screen_scaling * small_tick_rotation * small_tick_translation * small_tick_scaling);
            r.drawObject(shape);
        }
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm local_time = *localtime(&time);

        float seconds = local_time.tm_sec;
        float minutes = local_time.tm_min + seconds / 60;
        float hours = (local_time.tm_hour % 12) + minutes / 60 + seconds / 3600;

        mat4 hour_rotation = rotate(mat4(1.0f), radians(hours * 30.0f), vec3(0.0f, 0.0f, -1.0f));
        r.setUniform(program, "transform", screen_scaling * hour_rotation * hour_translation * hour_scaling);
        r.drawObject(shape);

        mat4 minute_rotation = rotate(mat4(1.0f), radians(minutes * 6.0f), vec3(0.0f, 0.0f, -1.0f));
        r.setUniform(program, "transform", screen_scaling * minute_rotation * minute_translation * minute_scaling);
        r.drawObject(shape);

        mat4 second_rotation = rotate(mat4(1.0f), radians(seconds * 6.0f), vec3(0.0f, 0.0f, -1.0f));
        r.setUniform(program, "transform", screen_scaling * second_rotation * second_translation * second_scaling);
        r.setUniform(program, "color", vec4(1.0, 0.0, 0.0, 1.0));
        r.drawObject(shape);
        r.show();
    }
    r.deleteShaderProgram(program);
    return EXIT_SUCCESS;
}
