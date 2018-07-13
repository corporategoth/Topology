#include <memory>
#include <ctime>

#define WIDTH 100
#define HEIGHT 100
#define GRID_SIZE WIDTH*HEIGHT

#define MAJOR_FEATURES_ACROSS 2
#define MAJOR_FEATURES_DOWN 2
#define MAJOR_FEATURES MAJOR_FEATURES_ACROSS*MAJOR_FEATURES_DOWN
#define MAJOR_FEATURE_VARIANCE 3.0
#define CORNER_VARIANCE 1.0
#define MINOR_VARIANCE 0.02

float z[HEIGHT][WIDTH];

int major_features_x[MAJOR_FEATURES];
int major_features_y[MAJOR_FEATURES];

enum direction {
    NORTHEAST, SOUTHEAST, SOUTHWEST, NORTHWEST
};

std::pair<float, float> find_closest_feature(direction d, int start_x, int start_y) {
    int x = start_x;
    int y = start_y;
    int distance = 0;

    while (x > 0 && x < (WIDTH - 1) && y > 0 && y < (HEIGHT - 1)) {
        if (d == NORTHEAST) {
            if (y == start_y) {
                ++distance;
                x = start_x;
                y = start_y + distance;
            } else {
                --y;
                ++x;
                if(y == start_y)
                    continue;
            }
        } else if (d == SOUTHEAST) {
            if (x == start_x) {
                ++distance;
                x = start_x + distance;
                y = start_y;
            } else {
                --x;
                --y;
                if (x == start_x)
                    continue;
            }
        } else if (d == SOUTHWEST) {
            if (y == start_y) {
                ++distance;
                x = start_x;
                y = start_y - distance;
            } else {
                --x;
                ++y;
                if(y == start_y)
                    continue;
            }
        } else if (d == NORTHWEST) {
            if (x == start_x) {
                ++distance;
                x = start_x - distance;
                y = start_y;
            } else {
                ++x;
                ++y;
                if (x == start_x)
                    continue;
            }
        }

        bool found = false;
        for (int i = 0; i < MAJOR_FEATURES; ++i) {
            if (x == major_features_x[i] && y == major_features_y[i]) {
                found = true;
                break;
            }
        }

        if (found)
            break;
    }

    if (x == start_x || y == start_y)
        return std::make_pair(z[y][x], static_cast<float>(distance));

    return std::make_pair(z[y][x], sqrt(pow(static_cast<float>(x-start_x), 2.0) + pow(static_cast<float>(y-start_y), 2.0)));
}

void createTopology() {
    memset(z, sizeof(float) * GRID_SIZE, 0);

    // First, create the boundaries.
    z[0][0] = (-CORNER_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * CORNER_VARIANCE));
    z[0][WIDTH-1] = (-CORNER_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * CORNER_VARIANCE));
    z[HEIGHT-1][0] = (-CORNER_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * CORNER_VARIANCE));
    z[HEIGHT-1][WIDTH-1] = (-CORNER_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * CORNER_VARIANCE));

    // Top and bottom row
    for (int i = 1; i < WIDTH-1; ++i) {
        float variance = (-MINOR_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * MINOR_VARIANCE));
        z[0][i] = z[0][i-1] - ((z[0][i-1] - z[0][WIDTH-1]) / static_cast<float>(WIDTH-i)) + variance;
        variance = (-MINOR_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * MINOR_VARIANCE));
        z[HEIGHT-1][i] = z[HEIGHT-1][i-1] - ((z[HEIGHT-1][i-1] - z[HEIGHT-1][WIDTH-1]) / static_cast<float>(WIDTH-i)) + variance;
    }

    // Left and right row.
    for (int i = 1; i < HEIGHT-1; ++i) {
        float variance = (-MINOR_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * MINOR_VARIANCE));
        z[i][0] = z[i-1][0] - ((z[i-1][0] - z[HEIGHT-1][0]) / static_cast<float>(HEIGHT-i)) + variance;
        variance = (-MINOR_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * MINOR_VARIANCE));
        z[i][WIDTH-1] = z[i-1][WIDTH-1] - ((z[i-1][WIDTH-1] - z[HEIGHT-1][WIDTH-1]) / static_cast<float>(HEIGHT-i)) + variance;
    }

    // Major
    for (int across = 0; across < MAJOR_FEATURES_ACROSS; ++across) {
        for (int down = 0; down < MAJOR_FEATURES_DOWN; ++down) {
            int x = (rand() % (WIDTH / MAJOR_FEATURES_ACROSS)) + (across * (WIDTH / MAJOR_FEATURES_ACROSS));
            if (x == 0)
                ++x;
            else if (x >= WIDTH-1)
                x = WIDTH-2;
            int y = (rand() % (HEIGHT / MAJOR_FEATURES_DOWN)) + (down * (HEIGHT / MAJOR_FEATURES_DOWN));
            if (y == 0)
                ++y;
            else if (y >= WIDTH-1)
                y = WIDTH-2;

            float value = (-MAJOR_FEATURE_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * MAJOR_FEATURE_VARIANCE));
#ifdef DEBUG
            printf("%d %d -> %.03f\n", y, x, value);
#endif
            major_features_y[across * MAJOR_FEATURES_ACROSS + down] = y;
            major_features_x[across * MAJOR_FEATURES_ACROSS + down] = x;
            z[y][x] = value;
        }
    }

    for (int y = 1; y < HEIGHT-1; ++y) {
        for (int x = 1; x < WIDTH-1; ++x) {
            bool found = false;
            for (int i = 0; i < MAJOR_FEATURES; ++i) {
                if (major_features_y[i] == y && major_features_x[i] == x) {
                    found = true;
                    break;
                }
            }
            if (found)
                continue;

            std::pair<float, float> ne = find_closest_feature(NORTHEAST, x, y);
            std::pair<float, float> se = find_closest_feature(SOUTHEAST, x, y);
            std::pair<float, float> sw = find_closest_feature(SOUTHWEST, x, y);
            std::pair<float, float> nw = find_closest_feature(NORTHWEST, x, y);
            float weights = (ne.second + se.second + sw.second + nw.second);
            float adj_weights = (1.0/(ne.second/weights)) + (1.0/(se.second/weights)) + (1.0/(sw.second/weights)) + (1.0/(nw.second/weights));
            float value = (ne.first * (1.0/(ne.second/weights))/adj_weights) + (se.first * (1.0/(se.second/weights))/adj_weights) +
                          (sw.first * (1.0/(sw.second/weights))/adj_weights) + (nw.first * (1.0/(nw.second/weights))/adj_weights);
            float variance = (-MINOR_VARIANCE) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (2.0 * MINOR_VARIANCE));
            z[y][x] = value + variance;
#ifdef DEBUG
            printf("NE (%.03f, %.03f) -> (%.03f, %.03f)\n", ne.first, ne.second, (1.0/(ne.second/weights))/adj_weights, ne.first*(1.0/(ne.second/weights))/adj_weights);
            printf("SE (%.03f, %.03f) -> (%.03f, %.03f)\n", se.first, se.second, (1.0/(se.second/weights))/adj_weights, se.first*(1.0/(se.second/weights))/adj_weights);
            printf("SW (%.03f, %.03f) -> (%.03f, %.03f)\n", se.first, sw.second, (1.0/(sw.second/weights))/adj_weights, sw.first*(1.0/(sw.second/weights))/adj_weights);
            printf("NW (%.03f, %.03f) -> (%.03f, %.03f)\n", ne.first, nw.second, (1.0/(nw.second/weights))/adj_weights, nw.first*(1.0/(nw.second/weights))/adj_weights);
            printf("WEIGHTS = %.03f, ADJ_WEIGHTS = %.03f, VALUE = %.03f, VARIANCE = %.03f\n", weights, adj_weights, value, variance);
            printf("-----\n");
#endif
        }
    }
}

int main() {
    srand(time(NULL));
    createTopology();

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            printf("% 2.03f ", z[y][x]);
        }
        printf("\n");
    }
}



