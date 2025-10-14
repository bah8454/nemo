#ifndef GEIGEL_BOID
#define GEIGEL_BOID

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>

#include "utils.hpp"

/// Representation of a single boid (fish) in the scene.
class Boid : public godot::Node3D {
    GDCLASS(Boid, godot::Node3D);
public:
    Boid() :
        current_velocity{
            static_cast<godot::real_t>(godot::UtilityFunctions::randf() * 2.0 - 1.0),
            static_cast<godot::real_t>(godot::UtilityFunctions::randf() * 2.0 - 1.0),
            static_cast<godot::real_t>(godot::UtilityFunctions::randf() * 2.0 - 1.0)
        }
    {}

    static void _bind_methods() {}

    godot::Vector3 current_velocity;
    godot::Vector3 new_velocity{ 0, 0, 0 };
    godot::Vector3 destination{ 0, 0, 0 };
};

#endif