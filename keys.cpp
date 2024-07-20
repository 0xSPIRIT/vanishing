bool is_action_pressed() {
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

int key_right() {
    return (int) (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
}

int key_left() {
    return (int) (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
}

int key_down() {
    return (int) (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
}

int key_up() {
    return (int) (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP));
}

int key_up_pressed() {
    return (int) (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP));
}

int key_down_pressed() {
    return (int) (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN));
}
