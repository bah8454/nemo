#ifndef GEIGEL_UTILS
#define GEIGEL_UTILS

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

/// Macro to generate Godot reflection information for a property.
/// Requires a getter and setter for the property (via GETTER_SETTER or otherwise).
#define BIND_PROPERTY(PROPERTY, VARIANT_TYPE, ...)                                                              \
    godot::ClassDB::bind_method(godot::D_METHOD("set_" #PROPERTY, #PROPERTY), &This::set_##PROPERTY);   \
    godot::ClassDB::bind_method(godot::D_METHOD("get_" #PROPERTY), &This::get_##PROPERTY);              \
    godot::ClassDB::add_property(                                                                               \
        get_class_static(),                                                                                     \
        godot::PropertyInfo(                                                                                    \
            VARIANT_TYPE,                                                                                       \
            #PROPERTY __VA_OPT__(,)                                                                             \
            __VA_ARGS__                                                                                         \
        ),                                                                                                      \
        "set_" #PROPERTY,                                                                                       \
        "get_" #PROPERTY                                                                                        \
    );

/// Macro to generate getters and setters for a property, relied upon by BIND_PROPERTY.
#define GETTER_SETTER(TYPE, PROPERTY)                                                                           \
    TYPE get_##PROPERTY() const { return this->PROPERTY; }                                                      \
    void set_##PROPERTY(TYPE PROPERTY) { this->PROPERTY = PROPERTY; }

namespace utils {

/// Checks if the user is currently in the editor or in game.
/// @returns True if the user is in the editor, false if the user is in game.
bool in_editor() {
    return godot::Engine::get_singleton()->is_editor_hint();
}

/// Checks if the user is currently in game or in the editor.
/// @returns True if the user is in game, false if the user is in the editor.
bool in_game() {
    return !in_editor();
}

//template <typename... Args>
//void println(std::format_string<Args...> fmt, Args&&... args) {
//    auto formatted = std::format(fmt, std::forward<Args>(args)...);
//    godot::UtilityFunctions::print(formatted.c_str());
//}

}

#endif