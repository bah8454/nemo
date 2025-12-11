#ifndef GEIGEL_BOID_MANAGER
#define GEIGEL_BOID_MANAGER

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>

#include "utils.hpp"
#include "input_manager.hpp"

#include <vector>
#include <cmath>

class BoidManager : public godot::Node3D {
    GDCLASS(BoidManager, godot::Node3D);
public:
    GETTER_SETTER(std::size_t, count);
    GETTER_SETTER(godot::real_t, collision_avoidance_weight);
    GETTER_SETTER(godot::real_t, velocity_matching_weight);
    GETTER_SETTER(godot::real_t, centering_weight);
    GETTER_SETTER(godot::real_t, following_weight);
    GETTER_SETTER(godot::real_t, orbit_weight);
    GETTER_SETTER(godot::real_t, boundary_weight);
    GETTER_SETTER(godot::real_t, boundary_radius);
    GETTER_SETTER(godot::real_t, upright_weight);
    GETTER_SETTER(godot::real_t, range);
    GETTER_SETTER(godot::NodePath, prototype_path);
    GETTER_SETTER(godot::real_t, maximum_speed);
    GETTER_SETTER(godot::real_t, maximum_acceleration);
    GETTER_SETTER(godot::NodePath, input_manager_path);

    void _ready() {
        if (utils::in_editor()) { return; }
        this->initialize_boids();
        this->mask_points.reserve(this->get_count());
    }

    void _process(godot::real_t delta) {
        if (utils::in_editor()) { return; }

        this->handle_input();

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

            ////godot::Vector3 to_leader = (boid->destination - position).normalized() * this->maximum_speed - velocity;
            ////following = to_leader.limit_length(this->maximum_acceleration);
            //godot::Vector3 to_target = boid->destination - position;
            //godot::real_t dist = to_target.length();

            //if (dist > 0.001f) {
            //    godot::real_t slow_radius = this->range * 5;
            //    godot::real_t min_speed = this->maximum_speed * 0.1;
            //    godot::real_t target_speed;

            //    if (dist < slow_radius) {
            //        godot::real_t t = dist / slow_radius;
            //        target_speed = min_speed + t * (this->maximum_speed - min_speed);
            //    } else {
            //        target_speed = this->maximum_speed;
            //    }

            //    godot::Vector3 desired_velocity = to_target.normalized() * target_speed;

            //    following = (desired_velocity - velocity)
            //        .limit_length(this->maximum_acceleration);

            //} else {
            //    following = godot::Vector3();
            //}
            ////

            //// Orbiting calculations.
            //godot::Vector3 forward = velocity.length() > 0.01f
            //    ? velocity.normalized()
            //    : to_target.normalized();
            //godot::Vector3 right = forward.cross(godot::Vector3(0, 1, 0)).normalized();
            //godot::real_t rand_val = (this->rng->randf() * 2.0 - 1.0); // -1 to +1
            //godot::Vector3 orbit_force = right * rand_val;
            //if (dist > 5.0) {
            //    orbit_force = godot::Vector3();
            //}
            //// End orbiting calculations.

            //godot::Vector3 acceleration =
            //    velocity_matching * this->velocity_matching_weight +
            //    centering * this->centering_weight +
            //    collision_avoidance * this->collision_avoidance_weight +
            //    following * this->following_weight +
            //    boundary_force * this->boundary_weight +
            //    orbit_force * this->orbit_weight;

            //velocity += acceleration * delta;
            //velocity = velocity.limit_length(this->maximum_speed);
            //boid->new_velocity = velocity;

            //position += velocity * delta;
            //boid->set_position(position);

            ////godot::Vector3 boid_up = boid->get_global_transform().basis.get_column(1);
            ////if (velocity.length() > 0.01) { // TODO: Epsilon.
            ////    godot::Vector3 world_up{0, 1, 0};

            ////    godot::Vector3 desired_up = boid_up.lerp(world_up, this->upright_weight).normalized();
            ////    boid->look_at(position + velocity, desired_up);
            ////}
            //godot::Vector3 boid_up = boid->get_global_transform().basis.get_column(1);
            //godot::Vector3 world_up{ 0, 1, 0 };

            //if (velocity.length() > 0.01f) {
            //    godot::Vector3 desired_forward = -velocity.normalized();
            //    float max_turn_rate = 4.0f; // 1–4
            //    godot::Vector3 current_forward =
            //        boid->get_global_transform().basis.get_column(2).normalized();
            //    float angle = current_forward.angle_to(desired_forward);
            //    float max_step = max_turn_rate * delta;
            //    float t = (angle < 1e-4f) ? 1.0f : godot::Math::min(1.0f, max_step / angle);
            //    godot::Vector3 new_forward = current_forward.slerp(desired_forward, t);
            //    godot::Vector3 desired_up = boid_up.lerp(world_up, this->upright_weight).normalized();
            //    godot::Basis new_basis;
            //    new_basis.set_column(2, new_forward);
            //    new_basis.set_column(1, desired_up);
            //    new_basis.set_column(0, new_forward.cross(desired_up).normalized());

            //    boid->set_global_transform(
            //        godot::Transform3D(new_basis, position)
            //    );
            //}

            godot::Vector3 to_target = boid->destination - position;
            godot::real_t dist_to_target = to_target.length();
            godot::real_t slow_radius = this->range * 5;
            godot::real_t min_speed = this->maximum_speed * 0.01;
            godot::real_t desired_target_speed;

            if (dist_to_target > 0.001f) {
                if (dist_to_target < slow_radius) {
                    godot::real_t t = dist_to_target / slow_radius;
                    desired_target_speed = min_speed + t * (this->maximum_speed - min_speed);
                }
                else {
                    desired_target_speed = this->maximum_speed;
                }
            }
            else {
                desired_target_speed = min_speed;
            }


            godot::Vector3 steer_sum = godot::Vector3();
            auto add_dir = [&](const godot::Vector3& vec, godot::real_t weight) {
                if (vec.length() > 1e-5f) {
                    steer_sum += vec.normalized() * weight;
                }
                };
            add_dir(velocity_matching, this->velocity_matching_weight);
            add_dir(centering, this->centering_weight);
            add_dir(collision_avoidance, this->collision_avoidance_weight);
            add_dir(boundary_force, this->boundary_weight);
            if (dist_to_target > 0.001f) {
                add_dir(to_target.normalized(), this->following_weight);
            }

            godot::Vector3 desired_heading;
            if (steer_sum.length() > 1e-6f) {
                desired_heading = steer_sum.normalized();
            }
            else if (velocity.length() > 1e-5f) {
                desired_heading = velocity.normalized();
            }
            else if (dist_to_target > 0.001f) {
                desired_heading = to_target.normalized();
            }
            else {
                desired_heading = godot::Vector3(0, 0, 1); // Fallback.
            }

            godot::Vector3 current_forward = boid->get_global_transform().basis.get_column(2).normalized();
            godot::real_t max_turn_rate = 2.5; // 1 to 4
            godot::real_t angle = current_forward.angle_to(desired_heading);
            godot::real_t max_step = max_turn_rate * delta;
            godot::real_t slerp_t = 1.0f;
            if (angle > 1e-6f) {
                slerp_t = godot::Math::min(1.0f, max_step / angle);
            }
            godot::Vector3 new_forward = current_forward.slerp(desired_heading, slerp_t).normalized();
            godot::real_t current_speed = velocity.length();
            godot::real_t max_speed_change = this->maximum_acceleration * delta;
            godot::real_t new_speed = current_speed;
            if (current_speed < desired_target_speed) {
                new_speed = godot::Math::min(desired_target_speed, current_speed + max_speed_change);
            } else if (current_speed > desired_target_speed) {
                new_speed = godot::Math::max(desired_target_speed, current_speed - max_speed_change);
            }
            // Enforce minimum forward speed.
            new_speed = godot::Math::max(new_speed, min_speed);
            // Build final velocity.
            velocity = new_forward * new_speed;
            boid->new_velocity = velocity;
            // Position integration.
            position += velocity * delta;
            boid->set_position(position);
            // Force upright.
            godot::Vector3 boid_up = boid->get_global_transform().basis.get_column(1);
            godot::Vector3 world_up{ 0, 1, 0 };
            godot::Vector3 desired_up = boid_up.lerp(world_up, this->upright_weight).normalized();
            godot::Vector3 right = new_forward.cross(desired_up);
            if (right.length() < 1e-5f) {
                // degenerate: pick some right vector
                right = godot::Vector3(1, 0, 0);
            }
            right = right.normalized();
            godot::Vector3 orth_up = right.cross(new_forward).normalized();
            godot::Basis new_basis;
            new_basis.set_column(0, right);
            new_basis.set_column(1, orth_up);
            new_basis.set_column(2, new_forward);
            // Final transform.
            boid->set_global_transform(godot::Transform3D(new_basis, position));
        }

        for (Boid* boid : this->boids) {
            boid->current_velocity = boid->new_velocity;
        }
    }

    godot::Vector3 offset_destination(const godot::Vector3& base_destination, godot::real_t maximum_offset_distance = 1.0) {
        godot::real_t theta = this->rng->randf() * Math_TAU;
        godot::real_t phi = acos(2.0 * this->rng->randf() - 1);

        godot::Vector3 dir{
            sin(phi) * cos(theta),
            cos(phi),
            sin(phi) * sin(theta)
        };

        // Random distance within radius
        godot::real_t dist = this->rng->randf() * maximum_offset_distance;

        return base_destination + dir * dist;
    }

    static void _bind_methods() {
        using This = BoidManager;
        BIND_PROPERTY(count, godot::Variant::INT);
        BIND_PROPERTY(collision_avoidance_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(velocity_matching_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(centering_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(following_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(orbit_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(boundary_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(boundary_radius, godot::Variant::FLOAT);
        BIND_PROPERTY(upright_weight, godot::Variant::FLOAT);
        BIND_PROPERTY(range, godot::Variant::FLOAT);
        BIND_PROPERTY(prototype_path, godot::Variant::NODE_PATH);
        BIND_PROPERTY(maximum_speed, godot::Variant::FLOAT);
        BIND_PROPERTY(maximum_acceleration, godot::Variant::FLOAT);
        BIND_PROPERTY(input_manager_path, godot::Variant::NODE_PATH);
    }

private:
    std::size_t count = 0;

    godot::real_t collision_avoidance_weight = 0;
    godot::real_t velocity_matching_weight = 0;
    godot::real_t centering_weight = 0;
    godot::real_t following_weight = 0;
    godot::real_t orbit_weight = 0;

    godot::real_t boundary_weight = 0;
    godot::real_t boundary_radius = 10;

    godot::real_t upright_weight = 0.25;

    godot::real_t range = 0;

    godot::real_t maximum_speed = 10.0;
    godot::real_t maximum_acceleration = 1.0;

    godot::NodePath prototype_path{};
    godot::Node3D* prototype = nullptr;

    std::vector<Boid*> boids;

    godot::NodePath input_manager_path{};
    InputManager* input_manager = nullptr;
    godot::String current_input = "O";
    std::vector<godot::Vector2> mask_points{};

    godot::RandomNumberGenerator* rng = memnew(godot::RandomNumberGenerator);

    void initialize_boids() {
        for (std::size_t i = 0; i < this->count; ++i) {
            // Allocate the node.
            Boid* boid = memnew(Boid);
            // Tell the manager the information it requires.
            this->boids.push_back(boid);
            // Obtain a handle to the prototype.
            this->prototype = this->get_node<godot::Node3D>(this->prototype_path);
            if (!this->prototype) {
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

    void handle_input() {
        if (this->input_manager == nullptr) {
            // Obtain a handle to the input manager.
            this->input_manager = this->get_node<InputManager>(this->input_manager_path);
            if (!this->input_manager) {
                godot::print_error("Input manager node not found!");
                return;
            }
        }
        //godot::UtilityFunctions::print("Input: ", this->input_manager->get_current());
        if (this->current_input == this->input_manager->get_current()) { return; }
        this->current_input = this->input_manager->get_current();
        godot::String mask_path = "res://masks/" + this->input_manager->get_current() + ".png";
        this->set_mask_points(mask_path);
        this->set_boid_destinations();
    }

    void set_boid_destinations() {
        for (std::size_t i = 0; i < this->boids.size(); ++i) {
            godot::Vector2 point = this->mask_points[i % this->mask_points.size()];
            boids[i]->destination = godot::Vector3{ (point.x - 1000)/23, -(point.y - 390)/23, 0 };
        }
    }

    void set_mask_points(godot::String mask_path) {
        this->mask_points.clear();
        godot::Ref<godot::Image> image = godot::Image::load_from_file(mask_path);
        image->decompress();
        image->convert(godot::Image::Format::FORMAT_RGBA8);
        std::size_t width = image->get_width();
        std::size_t height = image->get_height();
        std::vector<godot::Vector2> masked_pixels{};
        for (std::size_t y = 0; y < height; ++y) {
            for (std::size_t x = 0; x < width; ++x) {
                godot::Color c = image->get_pixel(x, y);
                if (c.a > 0.5) {
                    masked_pixels.emplace_back(static_cast<godot::real_t>(x), static_cast<godot::real_t>(y));
                }
            }
        }
        if (masked_pixels.empty()) { return; }
        std::size_t attempts = 0;
        std::size_t maximum_attempts = 5000;
        godot::real_t minimum_distance = 5;
        while (this->mask_points.size() < this->get_count() && attempts < maximum_attempts) {
            attempts += 1;
            godot::Vector2 candidate = masked_pixels[this->rng->randi_range(0, masked_pixels.size())];
            bool ok = true;
            for (godot::Vector2& point : this->mask_points) {
                if (point.distance_to(candidate) < minimum_distance) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                this->mask_points.push_back(candidate);
            }   
        }
        godot::UtilityFunctions::print("Got ", this->mask_points.size(), " points.");
    }
};

#endif
