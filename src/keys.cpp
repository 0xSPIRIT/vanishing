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

Vector2 input_movement_look(float dt) {
    Vector2 result = get_mouse_delta();

    const float mouse_sensitivity = 0.005f;

    result.x *= mouse_sensitivity;
    result.y *= mouse_sensitivity;

    if (IsGamepadAvailable(0)) {
        const float gamepad_sensitivity = 0.05f;

        result.x += gamepad_sensitivity * 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        result.y += gamepad_sensitivity * 60 * dt * GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
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
