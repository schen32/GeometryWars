# GeometryWars

A simple 2d top-down shooter game based off the retro shooter game: Geometry Wars.

![ezgif-6efe3115867c24](https://github.com/user-attachments/assets/b94512b2-4930-4f81-bee2-603899c05758)

As my first project in C++ game development, I've made a simple 2d shooter game emulating the retro game Geometry wars to practice and better understand game engine architecture, following the course COMP4300 by Dave Churchill. The game makes use of Entity-Component-System (ECS) game architecture, separating data from logic, making for very modularized components that are simple to add and remove to whichever entity you desire. The overall file structure goes like this: there is a overaching Game object that contains a list of Entities which contain a set of Components. Then, various Systems within the Game class get called every frame to act upon the Entities' Components, depending on if the Entity has a Component or not.

![ezgif-69d169811c89ed](https://github.com/user-attachments/assets/0b9bf417-72c2-426a-8bd9-abedca05cac5)

For example, the player and the enemies, which are in the form of primitive polygonal shapes, are all entities. The bullets that the player fires are also entities. We want some of these entities to have different properties, which is where components come in. Virtually every entity will have a position and velocity, so we store those values in a Transform component. Every entity has a shape, we use a Shape component to represent the shape. What if we want the bullet to dissappear after a short amount of time? In comes the Lifespan component, where we can specify how long the entity lives for. Lets also add in a Collision Component, which the Collision System will act upon.

Now that we have a bunch of data stored within these components, we need systems to modify the data accordingly. If an entity has a Lifespan component, we decrease the lifespan every second, but if it doesn't, we make sure not to accidently make it dissappear. For that purpose, the system checks if an entity has a given component first, ensuring we modify only the entities that we want to change. Then, whenever we want an entity to disspear after a certain amount of time, we simply attach a Lifespan component with a specified time to it.

Essentially, every frame, our game loops over every entity, calling every system. Each system checks if the entity has the appropriate components, then modifies the data within those components accordingly. If we want any entity to collide, we attach a Collision Component. The Collision system will then check every entity with a Collision Component and resolve collision that way. Also, collidable entites now bounce off the edges of the screen, since we check for boundaries in our system too. So, once we've implemented a system for a given component, we can simply add the corresponding components to any entity, and now it will behave according to the logic of our system.
