bool is_action_pressed() {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return true;

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
            return true;
        }
    }

    return false;
}

bool key_right() {
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        return true;
    }

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
            return true;
        }
    }

    return false;
}

bool key_left() {
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        return true;
    }

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
            return true;
        }
    }

    return false;
}

bool key_down() {
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        return true;
    }

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
            return true;
        }
    }

    return false;
}

bool key_up() {
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        return true;
    }

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
            return true;
        }
    }

    return false;
}

float input_movement_x_axis(float dt) {
    float result;

    result = key_right() - key_left();
    result += 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);

    return result;
}

float input_movement_y_axis(float dt) {
    float result;

    result = key_down() - key_up();
    result += 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

    return result;
}

Vector2 input_movement_cont_look_axis(float dt) {
    Vector2 result;

    result.x = 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    result.y = 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

    return result;
}

Vector2 get_mouse() {
    Vector2 result   = GetMousePosition();
    Rectangle screen = get_screen_rectangle();

    float scale_x = screen.width  / (float)render_width;
    float scale_y = screen.height / (float)render_height;

    result.x -= screen.x;
    result.y -= screen.y;
    result.x /= scale_x;
    result.y /= scale_y;

    return result;
}

Vector2 get_mouse_delta(void) {
    if (toggled_fullscreen_past_second)
        return {0, 0};

    Rectangle screen = get_screen_rectangle();

    float scale_x = screen.width  / (float)render_width;
    float scale_y = screen.height / (float)render_height;

    Vector2 result = GetMouseDelta();

    result.x /= scale_x;
    result.y /= scale_y;

    return result;
}

Vector2 input_movement_look(float dt) {
    Vector2 result = get_mouse_delta();

    const float mouse_sensitivity = 0.01f;

    result.x *= mouse_sensitivity;
    result.y *= mouse_sensitivity;

    if (IsGamepadAvailable(0)) {
        const float gamepad_sensitivity = 0.04f;

        Vector2 v = input_movement_cont_look_axis(dt);

        result.x += gamepad_sensitivity * v.x;
        result.y += gamepad_sensitivity * v.y;
    }

    return result;
}

float gamepad_movement_angle(float dt) {
    float x = input_movement_x_axis(dt);
    float y = -input_movement_y_axis(dt);

    float angle = atan2f(y, x);

    return angle;
}

float gamepad_movement_magnitude(float dt) {
    float x = input_movement_x_axis(dt);
    float y = input_movement_y_axis(dt);

    float result = sqrtf(x*x + y*y);

    return result;
}

int input_movement_x_axis_int(float dt) {
    float angle = gamepad_movement_angle(dt);

    if (gamepad_movement_magnitude(dt) < 0.05)
        return 0;

    if (angle > -3*PI/8 && angle < 3*PI/8)
        return 1;
    if (angle > 5*PI/8 && angle <= PI)
        return -1;
    if (angle < -5*PI/8 && angle >= -PI)
        return -1;

    return 0;
}

int input_movement_y_axis_int(float dt) {
    float angle = gamepad_movement_angle(dt);

    if (gamepad_movement_magnitude(dt) < 0.05)
        return 0;

    if (angle > PI/8 && angle < 7*PI/8)
        return -1;
    if (angle < -PI/8 && angle > -7*PI/8)
        return 1;

    return 0;
}

int key_up_pressed() {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        return true;
    }

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
            return true;
        }
    }

    return false;
}

int key_down_pressed() {
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        return true;
    }

    if (IsGamepadAvailable(0)) {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
            return true;
        }
    }

    return false;
}
