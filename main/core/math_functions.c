#include "core/include/math_functions.h"
#include "math.h"

// I was used in the before times
// A matrix describes a transformation
point_vector_t ortographic_projection[2] = {
    {.x = 1, .y = 0, .z = 0},
    {.x = 0, .y = 1, .z = 0},
};

void matrix_multiply_2d(point_vector_t matrix[2], point_vector_t point, point_vector_t *result) {
    result->x = matrix[0].x * point.x + matrix[0].y * point.y + matrix[0].z * point.z;
    result->y = matrix[1].x * point.x + matrix[1].y * point.y + matrix[1].z * point.z;
    result->z = 0;
}

void matrix_multiply_3d(point_vector_t matrix[3], point_vector_t point, point_vector_t *result) {
    result->x = matrix[0].x * point.x + matrix[0].y * point.y + matrix[0].z * point.z;
    result->y = matrix[1].x * point.x + matrix[1].y * point.y + matrix[1].z * point.z;
    result->z = matrix[2].x * point.x + matrix[2].y * point.y + matrix[2].z * point.z;
}

void rotate_point_z(point_vector_t *point, point_vector_t *pivot, float angle_rad) {
    float cos_angle = cosf(angle_rad);
    float sin_angle = sinf(angle_rad);

    // Rotation matrix for X rotation:
    // [ cos(angle) -sin(angle), 0 ]
    // [ sin(angle)  cos(angle), 0 ]
    // [ 0           0           1 ]
    int16_t x_new = (int16_t)((point->x - pivot->x) * cos_angle - (point->y - pivot->y) * sin_angle + pivot->x);
    int16_t y_new = (int16_t)((point->x - pivot->x) * sin_angle + (point->y - pivot->y) * cos_angle + pivot->y);

    point->x = x_new;
    point->y = y_new;
}

void rotate_point_x(point_vector_t *point, point_vector_t *pivot, float angle_rad) {
    float cos_angle = cosf(angle_rad);
    float sin_angle = sinf(angle_rad);

    // Rotation matrix for X rotation:
    // [ 1  0           0          ]
    // [ 0  cos(angle) -sin(angle) ]
    // [ 0  sin(angle)  cos(angle) ]
    int16_t y_new = (int16_t)((point->y - pivot->y) * cos_angle - (point->z - pivot->z) * sin_angle + pivot->y);
    int16_t z_new = (int16_t)((point->y - pivot->y) * sin_angle + (point->z - pivot->z) * cos_angle + pivot->z);

    point->y = y_new;
    point->z = z_new;
}

void rotate_point_y(point_vector_t *point, point_vector_t *pivot, float angle_rad) {
    float cos_angle = cosf(angle_rad);
    float sin_angle = sinf(angle_rad);

    // Rotation matrix for Y rotation:
    // [  cos(angle)  0  -sin(angle) ]
    // [  0           1  0          ]
    // [ sin(angle)  0  cos(angle) ]
    int16_t x_new = (int16_t)((point->x - pivot->x) * cos_angle - (point->z - pivot->z) * sin_angle + pivot->x);
    int16_t z_new = (int16_t)((point->x - pivot->x) * sin_angle + (point->z - pivot->z) * cos_angle + pivot->z);

    point->x = x_new;
    point->z = z_new;
}

float lerp_f(float start, float end, float blend_factor) {
    return start + blend_factor * (end - start);
}

int16_t lerp_i(int16_t start, int16_t end, float blend_factor) {
    return (int16_t)(start + blend_factor * (end - start));
}

float get_blend_factor(int16_t value, int16_t value_start, int16_t value_end) {
    if (value_start == value_end) {
        return 0.0f;
    }

    if (value_start < value_end) {
        if (value <= value_start) {
            return 0.0f;
        }

        if (value >= value_end) {
            return 1.0f;
        }
    } else {
        if (value >= value_start) {
            return 0.0f;
        }

        if (value <= value_end) {
            return 1.0f;
        }
    }

    return (float)(value - value_start) / (value_end - value_start);
}

int32_t edge_function(vector2_t a, vector2_t b, vector2_t c) {
    return (int32_t)(b.x - a.x) * (c.y - a.y) - (int32_t)(b.y - a.y) * (c.x - a.x);
}

void perspective_projection(point_vector_t *point,
                            point_vector_t *pivot,
                            point_vector_t *screen_point,
                            float focal_length,
                            float camera_distance) {
    float z_cam = (float)point->z + camera_distance;

    if (z_cam < 1.0f) {
        z_cam = 1.0f;
    }

    float scale = focal_length / z_cam;

    screen_point->x = (int16_t)(pivot->x + ((float)point->x - pivot->x) * scale);
    screen_point->y = (int16_t)(pivot->y + ((float)point->y - pivot->y) * scale);
    screen_point->z = 0;
}