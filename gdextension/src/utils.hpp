#ifndef GEIGEL_HELPERS
#define GEIGEL_HELPERS

#define BIND_PROPERTY(PROPERTY, VARIANT_TYPE, ...)                                                              \
    godot::ClassDB::bind_method(godot::D_METHOD("set_" #PROPERTY, #PROPERTY), &InputManager::set_##PROPERTY);   \
    godot::ClassDB::bind_method(godot::D_METHOD("get_" #PROPERTY), &InputManager::get_##PROPERTY);              \
    godot::ClassDB::add_property(                                                                               \
        get_class_static(),                                                                                     \
        godot::PropertyInfo(                                                                                    \
            VARIANT_TYPE,                                                                                       \
            #PROPERTY,                                                                                          \
            __VA_ARGS__                                                                                         \
        ),                                                                                                      \
        "set_" #PROPERTY,                                                                                       \
        "get_" #PROPERTY                                                                                        \
    );

#define GETTER_SETTER(TYPE, PROPERTY)                                                                           \
    TYPE get_##PROPERTY() const { return this->##PROPERTY; }                                                    \
    void set_##PROPERTY(TYPE PROPERTY) { this->##PROPERTY = PROPERTY; }

#endif GEIGEL_INPUT_MANAGER