#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "input_manager.hpp"

void geigel_initialize_module(godot::ModuleInitializationLevel level) {
    if (level != godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    godot::ClassDB::register_class<InputManager>();
}

void geigel_uninitialize_module(godot::ModuleInitializationLevel level) {
    if (level != godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {

GDExtensionBool GDE_EXPORT geigel_library_init(GDExtensionInterfaceGetProcAddress get_proc_address, const GDExtensionClassLibraryPtr library, GDExtensionInitialization* initialization) {
    godot::GDExtensionBinding::InitObject init_object{ get_proc_address, library, initialization };

    init_object.register_initializer(geigel_initialize_module);
    init_object.register_terminator(geigel_uninitialize_module);
    init_object.set_minimum_library_initialization_level(godot::ModuleInitializationLevel::MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_object.init();
}

}
