#include <iostream>
#include <glm/glm.hpp>

using namespace std;

const int WIDTH = 120;
const int HEIGHT = 40;
// const float YX_RATIO = 2;

const uint8_t DARK_4 = 250;
const uint8_t DARK_3 = 200;
const uint8_t DARK_2 = 150;
const uint8_t DARK_1 = 100;
const uint8_t DARK_0 = 50;

bool membership_check(glm::vec2 (&triangle)[3], glm::vec2 &pt)
{
    for (int k = 0; k < 3; k++)
    {
        glm::vec2 v1 = triangle[k % 3];
        glm::vec2 v2 = triangle[(k + 1) % 3];
        glm::vec2 s = v2 - v1;
        glm::vec2 t(-s.y, s.x);

        float dot = glm::dot(t, pt - v1);
        if (dot < 0)
        {
            return false;
        }
    }
    return true;
}

glm::vec2 pix_to_pt(int x, int y)
{
    return glm::vec2((float(x) + 0.5) / WIDTH, (float(y) + 0.5) / HEIGHT);
}

int value_4xmsaa_square(glm::vec2 (&triangle)[3], int x, int y)
{
    glm::vec2 pt1 = pix_to_pt(x, y) + glm::vec2(0.25 / WIDTH, 0.25 / HEIGHT);
    glm::vec2 pt2 = pix_to_pt(x, y) + glm::vec2(-0.25 / WIDTH, 0.25 / HEIGHT);
    glm::vec2 pt3 = pix_to_pt(x, y) + glm::vec2(0.25 / WIDTH, -0.25 / HEIGHT);
    glm::vec2 pt4 = pix_to_pt(x, y) + glm::vec2(-0.25 / WIDTH, -0.25 / HEIGHT);

    uint8_t color = DARK_0;

    if (membership_check(triangle, pt1))
        color += 50;
    if (membership_check(triangle, pt2))
        color += 50;
    if (membership_check(triangle, pt3))
        color += 50;
    if (membership_check(triangle, pt4))
        color += 50;

    return color;
}

int value_4xmsaa_rot(glm::vec2 (&triangle)[3], int x, int y)
{
    glm::vec2 pt1 = pix_to_pt(x, y) + glm::vec2(0.3 / WIDTH, 0.2 / HEIGHT);
    glm::vec2 pt2 = pix_to_pt(x, y) + glm::vec2(-0.2 / WIDTH, 0.3 / HEIGHT);
    glm::vec2 pt3 = pix_to_pt(x, y) + glm::vec2(-0.3 / WIDTH, -0.2 / HEIGHT);
    glm::vec2 pt4 = pix_to_pt(x, y) + glm::vec2(0.2 / WIDTH, -0.3 / HEIGHT);

    uint8_t color = DARK_0;

    if (membership_check(triangle, pt1))
        color += 50;
    if (membership_check(triangle, pt2))
        color += 50;
    if (membership_check(triangle, pt3))
        color += 50;
    if (membership_check(triangle, pt4))
        color += 50;

    return color;
}

int value_binary(glm::vec2 (&triangle)[3], int x, int y)
{
    glm::vec2 pt = pix_to_pt(x, y);
    if (membership_check(triangle, pt))
        return DARK_4;
    else
        return DARK_0;
}

void render_naive(glm::vec2 (&triangle)[3], uint8_t (&display)[HEIGHT][WIDTH])
{

    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            display[HEIGHT - i - 1][j] = value_4xmsaa_rot(triangle, j, i);
        }
    }
}

void render_bbox(glm::vec2 (&triangle)[3], uint8_t (&display)[HEIGHT][WIDTH])
{
    int min_x = floor(min(triangle[0].x, min(triangle[1].x, triangle[2].x)) * WIDTH);
    int max_x = ceil(max(triangle[0].x, max(triangle[1].x, triangle[2].x)) * WIDTH);
    int min_y = floor(min(triangle[0].y, min(triangle[1].y, triangle[2].y)) * HEIGHT);
    int max_y = ceil(max(triangle[0].y, max(triangle[1].y, triangle[2].y)) * HEIGHT);
    for (int i = min_y; i < max_y; i++)
    {
        for (int j = min_x; j < max_x; j++)
        {
            display[HEIGHT - i - 1][j] = value_4xmsaa_rot(triangle, j, i);
        }
    }
}

void render_scanline(glm::vec2 (&triangle)[3], bool (&display)[HEIGHT][WIDTH])
{

    glm::vec2 a = triangle[0];
    glm::vec2 b = triangle[1];
    glm::vec2 c = triangle[2];

    glm::vec2 line_left = c - a;
    glm::vec2 line_right = c - b;
}

void print(uint8_t (&display)[HEIGHT][WIDTH])
{

    for (auto r : display)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            uint8_t c = r[i];
            if (c == DARK_4)
                cout << "█";
            else if (c == DARK_3)
                cout << "▓";
            else if (c == DARK_2)
                cout << "▒";
            else if (c == DARK_1)
                cout << "░";
            else
                cout << " ";
        }
        cout << '\n';
    }
}

int main(int argc, char **argv)
{

    // triangle in NDC, anticlock vertices
    glm::vec2 triangle[3] = {glm::vec2(0.3, 0.2), glm::vec2(0.6, 0.3), glm::vec2(0.5, 0.8)};

    uint8_t display[HEIGHT][WIDTH];

    for (auto r : display)
    {
        for (int i = 0; i < WIDTH; i++)
            r[i] = false;
    }

    // naive algorithm
    render_bbox(triangle, display);

    print(display);

    return 0;
}
