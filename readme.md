By Audrey Fuller, Benson Haley & Spencer Kurtz

TODO: Insert Web Build for game?
& controls info

# Project Overview
Do you remember that one scene in _Finding Nemo_ where the school of tiny fish form shapes to give directions to Marlin?

This was the main inspiration for our project, where we implement a custom tool in the Godot game engine to have fish to flock in any number of arbitrary shapes! This offers a streamlined visualization tool for technical artists and developers.  

# Goals
This project implents the following functionality:
- A generic **fish swarming algorithm** using **Boids**
- A custom **bounding-box seeking** algorithm component
- **User** input to select the shape
- **Fish models**
- **Underwater Effects**

---

# Algorithm Background
## Boids
At the core of the project lies the Boids algorithm, developed by Craig Reynolds in 1986 and designed to simulate the emergent, collective behavior group movements. The algorithm achieves this realistic simulation through a decentralized model where each individual "boid" (short for bird-oid object) follows a few simple, local rules based on its interaction with nearby boids, rather than being directed by a central leader.

For our implementation, we represent a boid with three fields:
* It's current velocity
* A velocity that's updated every frame
* A destination it's traveling to, aka

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

## Input Parameters
For our algorithm imputs, we include the following parameters:
**Steering behavior weights:**
* ```collision_avoidance_weight```
* ```velocity_matching_weight```
* ```centering_weight```
* ```following_weight```
* ```boundary_weight```
* ```upright_weight```

**Global flock attributes:**
* ```range```: neighbor detection radius
* ```boundary_radius```
* ```maximum_speed```
* ```maximum_acceleration```

These can be fine-tuned to change how the fish move and react to their environmemt. 

## Flocking Forces
For each boid and every frame, we calculate flocking forces using variables for the three classic boids rules:
* **velocity_matching**: align with neighbors’ velocities
* **centering**: move toward neighbors’ average position
* **collision_avoidance**: steer away from nearby boids

```cpp
    velocity_matching = (velocity_matching / neighbor_count).normalized() * this->maximum_speed - velocity;
    centering = ((centering / neighbor_count) - position).normalized() * this->maximum_speed - velocity;
    collision_avoidance = collision_avoidance.normalized() * this->maximum_speed - velocity;
```

We also normalize, scale to maximum acceleration and combine these values using weights.

## Bounding Box

In addition to the base flocking algorithm our algorithm uses a custom boundary force parameter to re-adjust the boid's position to push inward when outside of the letter shape.

TODO: Add updated code for letter input, not just center

```cpp
godot::real_t dist_from_center = position.length();
            if (dist_from_center > this->boundary_radius) {
                godot::Vector3 to_center = (-position).normalized();
                godot::real_t strength = (dist_from_center - this->boundary_radius) / this->boundary_radius;
                boundary_force = (to_center * strength * this->maximum_speed - velocity).limit_length(this->maximum_acceleration);
            }
```

TODO: Explain following/targeting behavior

# Implementation
We use C++ for the sake of efficiency, compiled into a dynamic link library that can be loaded by the Godot engine.  The engine exposes symbols through an implementation of GDExtension called godot-cpp.

The project is designed to compile cross-platform, as we have developers using both Windows and Linux, so we support MSVC and GCC by default.

## 3D Models
# TODO

---

# Build Instructions
Interested in running this project locally? 

1. Note: If you've built in a previous environment already, empty the `gdextension/build` folder.
## Windows
2. Open the `gdextension` folder in Visual Studio.
    * If you don't have visual studio pass `-DCMAKE_CXX_COMPILER="path/to/compiler"` as an argument into CMAKE.
3. Build as a CMAKE project.
## Linux
2. `cd` to `gdextension/build`.
3. Run `cmake ..` / `cmake .. -G Ninja`.
4. Run `make` / `ninja`.

# Results
- TODO: How it looked (results...screen shots)

# Future Work
Some possible extentions to this project:
* A larger variety of 3D models (Custom fish, birds, etc.)
* A free look camera
* An ocean environment
* A web deployment 
* Dynamic fish animation 