enum {
    MOVIE_OFF,
    MOVIE_DRACULA,
    MOVIE_PICNIC,
    MOVIE_EMPTINESS
};

struct Movie {
    int     movie;
    int     frames;

    plm_t  *plm;
    double  last_time;
    double  framerate;

    void (*end_movie_callback)(struct Game *game);

    Texture texture;   // current frame
    Music   audio;
};

Movie game_movie;

void movie_init(Movie *movie, int which_movie);
void movie_run(Movie *movie);
