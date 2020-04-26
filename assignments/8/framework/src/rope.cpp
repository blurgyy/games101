#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        for(int i = 0; i < num_nodes; ++ i){
            Vector2D pos = start + i * (end - start) / (num_nodes - 1);
            Mass* mass = new Mass(pos, node_mass, false);
            mass->velocity = Vector2D(0, 0);
            masses.push_back(mass);
        }
        for(int i = 1; i < num_nodes; ++ i){
            Spring* spring = new Spring(masses[i-1], masses[i], k);
            springs.push_back(spring);
        }
//        Comment-in this part when you implement the constructor
       for (auto &i : pinned_nodes) {
           masses[i]->pinned = true;
       }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            //  x -------- x
            // m1          m2
            Vector2D m1tom2 = s->m2->position - s->m1->position;
            double curlen = m1tom2.norm();
            Vector2D force = s->k * m1tom2 / curlen * (curlen - s->rest_length);
            s->m1->forces += force;
            s->m2->forces -= force;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity;
                Vector2D accel = m->forces / m->mass;
                // Explicit Euler method
                // m->position += m->velocity * delta_t;
                // m->velocity += accel * delta_t;

                // Semi-implicit Euler method
                m->velocity += accel * delta_t;
                m->position += m->velocity * delta_t;
                // TODO (Part 2): Add global damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                
                // TODO (Part 4): Add global Verlet damping
            }
        }
    }
}
