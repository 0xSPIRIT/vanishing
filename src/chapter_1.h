enum Node_Type {
    NODE_INVALID,
    NODE_FIRST,
    NODE_SECOND,
    NODE_THIRD
};

enum Player_Walking_State {
    PLAYER_WALKING_STATE_NORMAL,
    PLAYER_WALKING_STATE_SLOWED_1,
    PLAYER_WALKING_STATE_SLOWED_2,
    PLAYER_WALKING_STATE_CRAWLING
};

// These are included in the Entity struct in game.h as a part of
// the union.
struct Chapter_1_Player {
    Vector2 stored_pos;
    Player_Walking_State walking_state;
    bool  is_hurt;
    bool  huffing_and_puffing;
    int   dir_x, dir_y;
    int   footsteps_done;
    float blood_probability;
};

struct Chapter_1_Node {
    bool active;
    Node_Type type;
};

struct Chapter_1_Phone {
    bool called;
};
