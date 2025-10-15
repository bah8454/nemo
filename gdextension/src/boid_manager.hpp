#ifndef GEIGEL_BOID_MANAGER
#define GEIGEL_BOID_MANAGER

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/variant/transform3d.hpp>

#include "utils.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cmath>

#include <osc++.hpp>

class BoidManager : public godot::Node3D {
    GDCLASS(BoidManager, godot::Node3D);
public:
    GETTER_SETTER(std::size_t, count);
    GETTER_SETTER(godot::real_t, collision_avoidance_weight);
    GETTER_SETTER(godot::real_t, velocity_matching_weight);
    GETTER_SETTER(godot::real_t, centering_weight);
    GETTER_SETTER(godot::real_t, following_weight);
    GETTER_SETTER(godot::real_t, boundary_weight);
    GETTER_SETTER(godot::real_t, boundary_radius);
    GETTER_SETTER(godot::real_t, range);
    GETTER_SETTER(godot::NodePath, prototype_path);
    GETTER_SETTER(godot::real_t, maximum_speed);
    GETTER_SETTER(godot::real_t, maximum_acceleration);

    void _ready() {
        if (utils::in_editor()) { return; }
        this->initialize_boids();
    }

    void _process(godot::real_t delta) {
        if (utils::in_editor()) { return; }

        for (Boid* boid : this->boids) {
            godot::Vector3 position = boid->get_position();
            godot::Vector3 velocity = boid->current_velocity;

            godot::Vector3 velocity_matching{};
            godot::Vector3 centering{};
            godot::Vector3 collision_avoidance{};
            godot::Vector3 following{};
            godot::Vector3 boundary_force{};
            std::size_t neighbor_count = 0;

            for (Boid* other : this->boids) {
                if (boid == other) { continue; }
                godot::Vector3 other_pos = other->get_position();
                godot::Vector3 diff = other_pos - position;
                godot::real_t dist = diff.length();
                if (dist < this->range) {
                    velocity_matching += other->current_velocity;
                    centering += other_pos;
                    collision_avoidance -= (diff / (dist * dist + 0.01f));
                    neighbor_count++;
                }
            }

            if (neighbor_count > 0) {
                velocity_matching = (velocity_matching / neighbor_count).normalized() * this->maximum_speed - velocity;
                centering = ((centering / neighbor_count) - position).normalized() * this->maximum_speed - velocity;
                collision_avoidance = collision_avoidance.normalized() * this->maximum_speed - velocity;

                velocity_matching = velocity_matching.limit_length(this->maximum_acceleration);
                centering = centering.limit_length(this->maximum_acceleration);
                collision_avoidance = collision_avoidance.limit_length(this->maximum_acceleration);
            }

            godot::real_t dist_from_center = position.length();
            if (dist_from_center > this->boundary_radius) {
                godot::Vector3 to_center = (-position).normalized();
                godot::real_t strength = (dist_from_center - this->boundary_radius) / this->boundary_radius;
                boundary_force = (to_center * strength * this->maximum_speed - velocity).limit_length(this->maximum_acceleration);
            }

            godot::Vector3 to_leader = (boid->destination - position).normalized() * this->maximum_speed - velocity;
            following = to_leader.limit_length(this->maximum_acceleration);

            godot::Vector3 acceleration =
                velocity_matching * this->velocity_matching_weight +
                centering * this->centering_weight +
                collision_avoidance * this->collision_avoidance_weight +
                following * this->following_weight +
                boundary_force * this->boundary_weight;

            velocity += acceleration * delta;
            velocity = velocity.limit_length(this->maximum_speed);
            boid->new_velocity = velocity;

            position += velocity * delta;
            boid->set_position(position);

            godot::Vector3 boid_up = boid->get_global_transform().basis.get_column(1);
            if (velocity.length() > 0.01) { // TODO: Epsilon.
                boid->look_at(position + velocity, boid_up);
            }
        }

        for (Boid* boid : this->boids) {
            boid->current_velocity = boid->new_velocity;
        }
    }

    static void _bind_methods() {
        using This = BoidManager;
        BIND_PROPERTY(count, godot::Variant::INT);
        BIND_PROPERTY(collision_avoidance_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(velocity_matching_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(centering_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(following_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(boundary_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(boundary_radius, godot::Variant::FLOAT);
        BIND_PROPERTY(range, godot::Variant::FLOAT);
        BIND_PROPERTY(prototype_path, godot::Variant::NODE_PATH);
        BIND_PROPERTY(maximum_speed, godot::Variant::FLOAT);
        BIND_PROPERTY(maximum_acceleration, godot::Variant::FLOAT);
    }

private:
    std::size_t count = 0;

    godot::real_t collision_avoidance_weight = 0;
    godot::real_t velocity_matching_weight = 0;
    godot::real_t centering_weight = 0;
    godot::real_t following_weight = 0;

    godot::real_t boundary_weight = 0;
    godot::real_t boundary_radius = 10;

    godot::real_t range = 0;

    godot::real_t maximum_speed = 10.0;
    godot::real_t maximum_acceleration = 1.0;

    godot::NodePath prototype_path{};
    godot::Node3D* prototype = nullptr;

    std::vector<Boid*> boids;

    void initialize_boids() {
        for (std::size_t i = 0; i < this->count; ++i) {
            // Allocate the node.
            Boid* boid = memnew(Boid);
            // Tell the manager the information it requires.
            this->boids.push_back(boid);
            // Obtain a handle to the prototype.
            this->prototype = this->get_node<godot::Node3D>(prototype_path);
            if (!prototype) {
                godot::print_error("Prototype node not found!");
                return;
            }
            // Duplicate the prototype and add it as a child to this boid.
            godot::Node* prototype_clone = this->prototype->duplicate();
            boid->add_child(prototype_clone);
            // Add the boid node to the scene tree as a child of this manager.
            this->add_child(boid);
        }
    }
};

#endif