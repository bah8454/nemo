By Audrey Fuller, Benson Haley & Spencer Kurtz

<video width="1920" height="1080" autoplay="" loop="" muted="">
    <source src="./media/final.mp4" type="video/mp4">
</video>

# Project Overview
Do you remember the scene in _Finding Nemo_ where the school of fish forms shapes to give Marlin directions?

![Finding Nemo Gif](/media/finding_nemo.gif)

This project is inspired by that idea.
We built a custom **Godot tool**, backed by a C++ GDExtension, that lets a swarm of fish **dynamically form an arbitrary shape** based on user-selected input. This provides a streamlined visualization tool for technical artists and developers working with flocking simulations or shape-based formations.

---

# Goals
This project implements the following features:
* A generic **Boids flocking algorithm**
* A custom **shape-based bounding/targeting system**
* **User input** for selecting the desired formation
* **3D fish models**
* **Underwater visual effects**

![RIT Logo but Fish](/media/fish_logo.png)

---

# Algorithm Background
## Boids Overview
The flocking system is based on the classic Boids algorithm, developed by Craig Reynolds (1986) to simulate emergent group behavior.
Each boid is an autonomous agent, not controlled by a global manager. This leads to complex swarm motion emerging naturally from simple rules.

In our project, each boid tracks:
* Its **current velocity**
* A **new velocity** computed each frame
* A **destination vector** used for shape-targeting

```cpp
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
```

---

## Input Parameters
Our implementation exposes several tunable parameters that control flock behavior:

**Steering Behavior Weights:**
* ```collision_avoidance_weight```: How strongly boids avoid colliding with neighbors
* ```velocity_matching_weight```: Tendency to align with nearby boids
* ```centering_weight```: Move toward the flock’s center
* ```following_weight```: Strength of movement toward the assigned target pixel
* ```boundary_weight```: How strongly the boid stays inside the formation boundary
* ```upright_weight```: How quickly the boid rotates back toward world-up

**Global flock attributes:**
* ```range```: Neighbor detection radius
* ```boundary_radius```: Outer limit before the boid is pushed inward
* ```maximum_speed```: Velocity cap
* ```maximum_acceleration```: Acceleration cap

These parameters allow custom “behaviors” such as tight flocking, loose swimming, jittery motion, or smooth schooling.

---

## Flocking Forces
Each frame, each boid computes the classic Boids forces:
* **Velocity Matching**: Align with neighbors’ velocities
* **Centering**: Move toward neighbors’ average position
* **Collision Avoidance**: Steer away from nearby boids

![Boid Rules](/media/boids_rules.png)

```cpp
    velocity_matching = (velocity_matching / neighbor_count).normalized() * this->maximum_speed - velocity;
    centering = ((centering / neighbor_count) - position).normalized() * this->maximum_speed - velocity;
    collision_avoidance = collision_avoidance.normalized() * this->maximum_speed - velocity;
```

These are normalized, limited by max acceleration, and combined using user-provided weights. 

---

## Bounding Box
In addition to the base flocking algorithm, our project uses a custom shape-bounding system to define new targets for the boids from an input image. This results in them re-adjust the boid's position to 'fill' the image shape, thus giving the illusion of forming a letter in 3D space. 

![Fish Making an A](/media/fish_a.png)

With our initial implementation of this, we were running into an issue where the boids would either overshoot the target entirely, OR become stationary at it's target destination. To fix this, we implemented an `offset` that angles the movement of the boid slightly adjacent to it's target position, resulting in a more orbit-like path.

```cpp
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
```

This is done in **Three Steps**:
1. Keyboard input is received and saved into a shared buffer that the Boid management algorithm can read from.

```cpp
static void _bind_methods() {
    BIND_ENUM_CONSTANT(Mode::keyboard);
    BIND_ENUM_CONSTANT(Mode::camera);
    using This = InputManager;
    BIND_PROPERTY(mode, godot::Variant::INT, godot::PropertyHint::PROPERTY_HINT_ENUM, "keyboard,camera");
    BIND_PROPERTY(current, godot::Variant::STRING);
}
```

2. Destination points are randomly scattered across the letter shape using Poisson-Disk sampling.
Chaotic fish movement means we do not have to implement a more uniform (but slower) algorithm like Voronoi pattern initialization.

```cpp
    godot::Vector2 candidate = masked_pixels[this->rng->randi_range(0, masked_pixels.size())];
    bool ok = true;
    for (godot::Vector2& point : this->mask_points) {
        if (point.distance_to(candidate) < minimum_distance) {
            ok = false;
            break;
        }
    }
```

3. Boid steering is weighted with a following parameter, but minimum linear speed is maintained to prevent freezing once the destination is reached.

```cpp
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
```

![Bounding Box Algorithm Diagram](/media/boid_y_diagram.svg)

# Implementation
This project uses C++ (via GDExtension) for performance.
The resulting shared library can be loaded by Godot 4.x and constructed cross-platform.

**Why C++?**
* High-performance vector math
* Tight control over memory layout for thousands of boids
* Faster iterations inside _process()

**Build System**
The extension supports:
* MSVC on Windows
* GCC/Clang on Linux

Both environments use CMake for configuration.

## 3D Models
3D Fish Models are provided by **WalterSalmon** On CGTrader [Source](https://www.cgtrader.com/free-3d-models/animals/fish/trout-rainbow-freshwater).

![Fish Model](/media/trout_model.jpg)

They're stored and loaded in the .glb format.

---

# Build Instructions
Interested in running this project locally? 

1. NOTE: If you've built in a previous environment already, empty the `gdextension/build` folder.
## Windows
2. Open the `gdextension` folder in Visual Studio.
    * If you don't have visual studio pass `-DCMAKE_CXX_COMPILER="path/to/compiler"` as an argument into CMAKE.
3. Make a build directory and enter it `mkdir build` / `cd build`
3. Run the CMAKE Command to configure the project `cmake .. -G "MinGW Makefiles"` 
4. Build the project `cmake --build .`
NOTE: This may take a while to build!
## Linux
2. `cd` to `gdextension/build`.
3. Run `cmake ..` / `cmake .. -G Ninja`.
4. Run `make` / `ninja`.

---

# Results
Here's a small sample of the finished project!

## Fish Reacting to Different Input Letters
<video width="640" height="480" autoplay="" loop="" muted="">
    <source src="./media/fish.mp4" type="video/mp4">
</video>

## Fish Forming the letter Y
<video width="640" height="480" autoplay="" loop="" muted="">
    <source src="./media/good.mp4" type="video/mp4">
</video>

---

# Future Work
Some possible extentions to this project:
* More diverse 3D models (custom fish, birds, schooling creatures)
* Free-look camera system
* Full underwater ocean biome
* Web-build deployment
* Dynamic animation based on velocity/turning (faster tail beats, body roll, etc.)

![Thanks for Reading!](/media/spinning_fish.gif)