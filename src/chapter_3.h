struct Chapter_3_Cubicle {
    int number;
    struct Entity *cubicle_chair, *cubicle_top, *cubicle_vertical;
};

struct Chapter_3_Circler {
    float radius_x;
    float radius_y;
    float current_angle;
    Vector2 center_position;
};
