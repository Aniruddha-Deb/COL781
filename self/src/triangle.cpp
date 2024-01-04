#include <iostream>
#include <glm/glm.hpp>

using namespace std;

const int SIDE = 32;

bool membership_check(glm::vec2 (&triangle)[3], glm::vec2 &pt) {
    for (int k=0; k<3; k++) {
        glm::vec2 v1 = triangle[k%3];
        glm::vec2 v2 = triangle[(k+1)%3];
        glm::vec2 s = v2-v1;
        glm::vec2 t(-s.y, s.x);

        float dot = glm::dot(t, pt-v1);
        if (dot < 0) {
            return false;
        }
    }
    return true;
}

void render_naive(glm::vec2 (&triangle)[3], bool (&display)[SIDE][SIDE]) {

    for (int i=0; i<SIDE; i++) {
        for (int j=0; j<SIDE; j++) {
            glm::vec2 pt((float(j)+0.5)/SIDE, (float(i)+0.5)/SIDE);
            display[SIDE-i-1][j] = membership_check(triangle, pt);
        }
    }
}

void render_bbox(glm::vec2 (&triangle)[3], bool (&display)[SIDE][SIDE]) {
    int min_x = floor(min(triangle[0].x, min(triangle[1].x, triangle[2].x))*SIDE);
    int max_x = ceil(max(triangle[0].x, max(triangle[1].x, triangle[2].x))*SIDE);
    int min_y = floor(min(triangle[0].y, min(triangle[1].y, triangle[2].y))*SIDE);
    int max_y = ceil(max(triangle[0].y, max(triangle[1].y, triangle[2].y))*SIDE);
    for (int i=min_y; i<max_y; i++) {
        for (int j=min_x; j<max_x; j++) {
            glm::vec2 pt((float(j)+0.5)/SIDE, (float(i)+0.5)/SIDE);
            display[SIDE-i-1][j] = membership_check(triangle, pt);
        }
    }
}

void print(bool (&display)[SIDE][SIDE]) {

    for (auto r : display) {
        for (int i = 0; i<SIDE; i++) {
            bool c = r[i];
            if (c) {
                cout << "＃";
                // cout << "[]";
            }
            else {
                cout << "＇";
                // cout << "··";
            }
        }
        cout << '\n';
    }
}

int main(int argc, char** argv) {

    // triangle in NDC, anticlock vertices
    glm::vec2 triangle[3] = { glm::vec2(0.2, 0.2), glm::vec2(0.75, 0.3), glm::vec2(0.5, 0.8) };

    bool display[SIDE][SIDE];

    for (auto r : display) {
        for (int i=0; i<SIDE; i++) r[i] = false;
    }

    // naive algorithm
    render_bbox(triangle, display);

    print(display);

    return 0;
}
