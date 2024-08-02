enum Chapter_5_State {
    CHAPTER_5_STATE_INTRO,
    CHAPTER_5_STATE_TRAIN_STATION_1,
    CHAPTER_5_STATE_TRAIN_STATION_2,
};

struct Chapter_5_Train {
    Vector2 position;
    Vector2 velocity;
};

struct Level_Chapter_5 {
    Chapter_5_State state;
};
