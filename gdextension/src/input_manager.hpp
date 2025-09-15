#ifndef GEIGEL_INPUT_MANAGER
#define GEIGEL_INPUT_MANAGER

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>

#include "utils.hpp"

class InputManager : public godot::Node3D {
    GDCLASS(InputManager, godot::Node3D);
public:
    enum Mode {
        keyboard, camera
    };

    GETTER_SETTER(Mode, mode);
    GETTER_SETTER(godot::String, current);

    static void _bind_methods() {
        BIND_ENUM_CONSTANT(Mode::keyboard);
        BIND_ENUM_CONSTANT(Mode::camera);

        BIND_PROPERTY(mode, godot::Variant::INT, godot::PropertyHint::PROPERTY_HINT_ENUM, "keyboard,camera");
        BIND_PROPERTY(current, godot::Variant::STRING);
    }

    void _unhandled_input(const godot::Ref<godot::InputEvent>& event) {
        if (this->mode != Mode::keyboard) { return; }
        godot::Ref<godot::InputEventKey> key_input = event;
        if (key_input.is_valid() && key_input->is_pressed()) {
            char template_string[] = "_";
            template_string[0] = static_cast<char>(key_input->get_keycode());
            const char* final_string = template_string;
            this->current = godot::String{ final_string };
            godot::UtilityFunctions::print(this->current);
        }
    }


private:
    Mode mode = Mode::keyboard;
    godot::String current = "";
};

VARIANT_ENUM_CAST(InputManager::Mode);

#endif