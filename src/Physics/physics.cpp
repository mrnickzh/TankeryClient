// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "physics.h"

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including
// any other Jolt header. You can use Jolt.h in your precompiled header to speed
// up compilation.
#include <Jolt/Jolt.h> // NOLINT [misc-include-cleaner]
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CompoundShape.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>

#include <raylib.h>

// STL includes
#include <cstdarg>
#include <iostream>
#include <memory>
#include <raymath.h>
#include <thread>

// Disable common warnings triggered by Jolt, you can use
// JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the
// warning state
//JPH_SUPPRESS_WARNINGS

// If you want your code to compile using single or double precision write 0.0_r
// to get a Real value that compiles to double or float depending if
// JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

JPH::Vec3 AxisEnum[3] = { JPH::Vec3::sAxisX(), JPH::Vec3::sAxisY(), JPH::Vec3::sAxisZ() };

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char *inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1'024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    std::cout << buffer << '\n';
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression,
                             const char *inMessage,
                             const char *inFile,
                             JPH::uint inLine)
{
    // Print to the TTY
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") "
              << (inMessage != nullptr ? inMessage : "") << std::endl;

    // Breakpoint
    return true;
};

#endif // JPH_ENABLE_ASSERTS

PhysicsEngine::PhysicsEngine()
    : _body_activation_listener(std::make_unique<MyBodyActivationListener>()),
      _contact_listener(std::make_unique<MyContactListener>())
{
}

void PhysicsEngine::initialise()
{
    // Register allocation hook. In this example we'll just let Jolt use malloc /
    // free but you can override these if you want (see Memory.h). This needs to
    // be done before any other Jolt function is called.
    JPH::RegisterDefaultAllocator();

    // Install trace and assert callbacks
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

    // Create a factory, this class is responsible for creating instances of
    // classes based on their name or hash and is mainly used for deserialization
    // of saved data. It is not directly used in this example but still required.
    JPH::Factory::sInstance = new JPH::Factory();

    // Register all physics types with the factory and install their collision
    // handlers with the CollisionDispatch class. If you have your own custom
    // shape types you probably need to register their handlers with the
    // CollisionDispatch before calling this function. If you implement your own
    // default material (PhysicsMaterial::sDefault) make sure to initialize it
    // before this function or else this function will create one for you.
    JPH::RegisterTypes();

    // We need a temp allocator for temporary allocations during the physics
    // update. We're pre-allocating 10 MB to avoid having to do allocations during
    // the physics update. B.t.w. 10 MB is way too much for this example but it is
    // a typical value you can use. If you don't want to pre-allocate you can also
    // use TempAllocatorMalloc to fall back to malloc / free.
    _temp_allocator =
        std::make_unique<JPH::TempAllocatorImpl>(10 * 1'024 * 1'024);

    // We need a job system that will execute physics jobs on multiple threads.
    // Typically you would implement the JobSystem interface yourself and let Jolt
    // Physics run on top of your own job scheduler. JobSystemThreadPool is an
    // example implementation.
    _job_system = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        static_cast<int>(std::thread::hardware_concurrency()) - 1);

    // This is the max amount of rigid bodies that you can add to the physics
    // system. If you try to add more you'll get an error. Note: This value is low
    // because this is a simple test. For a real project use something in the
    // order of 65536.
    constexpr JPH::uint cMaxBodies = 65'536;

    // This determines how many mutexes to allocate to protect rigid bodies from
    // concurrent access. Set it to 0 for the default settings.
    constexpr JPH::uint cNumBodyMutexes = 0;

    // This is the max amount of body pairs that can be queued at any time (the
    // broad phase will detect overlapping body pairs based on their bounding
    // boxes and will insert them into a queue for the narrowphase). If you make
    // this buffer too small the queue will fill up and the broad phase jobs will
    // start to do narrow phase work. This is slightly less efficient. Note: This
    // value is low because this is a simple test. For a real project use
    // something in the order of 65536.
    constexpr JPH::uint cMaxBodyPairs = 65'536;

    // This is the maximum size of the contact constraint buffer. If more contacts
    // (collisions between bodies) are detected than this number then these
    // contacts will be ignored and bodies will start interpenetrating / fall
    // through the world. Note: This value is low because this is a simple test.
    // For a real project use something in the order of 10240.
    constexpr JPH::uint cMaxContactConstraints = 65'536;

    // Create mapping table from object layer to broadphase layer
    // Note: As this is an interface, PhysicsSystem will take a reference to this
    // so this instance needs to stay alive!
    _broad_phase_layer_interface = std::make_unique<BPLayerInterfaceImpl>();

    // Create class that filters object vs broadphase layers
    // Note: As this is an interface, PhysicsSystem will take a reference to this
    // so this instance needs to stay alive!
    _object_vs_broadphase_layer_filter =
        std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();

    // Create class that filters object vs object layers
    // Note: As this is an interface, PhysicsSystem will take a reference to this
    // so this instance needs to stay alive!
    _object_vs_object_layer_filter =
        std::make_unique<ObjectLayerPairFilterImpl>();

    // Now we can create the actual physics system.
    _physics_system = std::make_unique<JPH::PhysicsSystem>();
    _physics_system->Init(cMaxBodies,
                          cNumBodyMutexes,
                          cMaxBodyPairs,
                          cMaxContactConstraints,
                          *_broad_phase_layer_interface,
                          *_object_vs_broadphase_layer_filter,
                          *_object_vs_object_layer_filter);

    // A body activation listener gets notified when bodies activate and go to
    // sleep Note that this is called from a job so whatever you do here needs to
    // be thread safe. Registering one is entirely optional.
    _physics_system->SetBodyActivationListener(_body_activation_listener.get());

    // A contact listener gets notified when bodies (are about to) collide, and
    // when they separate again. Note that this is called from a job so whatever
    // you do here needs to be thread safe. Registering one is entirely optional.
    _physics_system->SetContactListener(_contact_listener.get());

    // The main way to interact with the bodies in the physics system is through
    // the body interface. There is a locking and a non-locking variant of this.
    // We're going to use the locking version (even though we're not planning to
    // access bodies from multiple threads)
    //JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
}

JPH::BodyID PhysicsEngine::create_floor(const Vector3 &floor_dimensions,
                                 const Vector3 &floor_position)
{
    // Next we can create a rigid body to serve as the floor, we make a large box
    // Create the settings for the collision volume (the shape).
    // Note that for simple shapes (like boxes) you can also directly construct a
    // BoxShape.
    const JPH::BoxShapeSettings floor_shape_settings{
        //JPH::Vec3(100.F, 1.F, 100.F)};
        JPH::Vec3{floor_dimensions.x, floor_dimensions.y, floor_dimensions.z}};

    // Create the shape
    const JPH::ShapeSettings::ShapeResult floor_shape_result =
        floor_shape_settings.Create();
    const JPH::ShapeRefC floor_shape =
        floor_shape_result
            .Get(); // We don't expect an error here, but you can check
                    // floor_shape_result for HasError() / GetError()

    if (floor_shape_result.HasError())
    {
        std::cout << "Error creating floor: {}" <<
                      floor_shape_result.GetError() << std::endl;
    }

    // Create the settings for the body itself. Note that here you can also set
    // other properties like the restitution / friction.
    const JPH::BodyCreationSettings floor_settings(
        floor_shape,
        //JPH::RVec3(0.0_r, -1.0_r, 0.0_r),
        JPH::RVec3(floor_position.x, floor_position.y, floor_position.z),
        JPH::Quat::sIdentity(),
        JPH::EMotionType::Static,
        Layers::NON_MOVING);

    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();

    // Create the actual rigid body
    JPH::Body *floor = body_interface.CreateBody(
        floor_settings); // Note that if we run out of bodies this can return
                         // nullptr
    if (floor == nullptr)
    {
        std::cout << "Error creating floor body interface. There might be too "
                      "many bodies." << std::endl;
    }

    // Add it to the world
    body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

    _floor_id = floor->GetID();
    return _floor_id;
}

JPH::BodyID PhysicsEngine::create_static(const Vector3 &static_dimensions,
                                const Vector3 &static_positon,
                                const Vector3 &static_rotation)
{
    // Now create a dynamic body to bounce on the floor
    // Note that this uses the shorthand version of creating and adding a body to
    // the world
    const JPH::BodyCreationSettings static_settings(
        new JPH::BoxShape(JPH::Vec3(static_dimensions.x, static_dimensions.y, static_dimensions.z)),
        //JPH::RVec3(0.0_r, 2.0_r, 0.0_r),
        JPH::RVec3(static_positon.x, static_positon.y, static_positon.z),
        JPH::Quat::sEulerAngles(JPH::RVec3(static_rotation.x * DEG2RAD, static_rotation.y * DEG2RAD, static_rotation.z * DEG2RAD)),
        JPH::EMotionType::Static,
        Layers::NON_MOVING);
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::BodyID static_id = body_interface.CreateAndAddBody(static_settings,
                                                 JPH::EActivation::Activate);
    _static_ids.push_back(static_id);
    return static_id;
}

JPH::BodyID PhysicsEngine::create_dynamic(const Vector3 &dynamic_dimensions,
                                const Vector3 &dynamic_positon,
                                const Vector3 &dynamic_rotation)
{
    // Now create a dynamic body to bounce on the floor
    // Note that this uses the shorthand version of creating and adding a body to
    // the world
    const JPH::BodyCreationSettings dynamic_settings(
        new JPH::BoxShape(JPH::Vec3(dynamic_dimensions.x, dynamic_dimensions.y, dynamic_dimensions.z)),
        //JPH::RVec3(0.0_r, 2.0_r, 0.0_r),
        JPH::RVec3(dynamic_positon.x, dynamic_positon.y, dynamic_positon.z),
        JPH::Quat::sEulerAngles(JPH::RVec3(dynamic_rotation.x * DEG2RAD, dynamic_rotation.y * DEG2RAD, dynamic_rotation.z * DEG2RAD)),
        JPH::EMotionType::Dynamic,
        Layers::MOVING);
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::BodyID dynamic_id = body_interface.CreateAndAddBody(dynamic_settings,
                                                 JPH::EActivation::Activate);
    _dynamic_ids.push_back(dynamic_id);
    return dynamic_id;
}

JPH::BodyID PhysicsEngine::create_kinematic(const Vector3 &kinematic_dimensions,
                                const Vector3 &kinematic_positon,
                                const Vector3 &kinematic_rotation)
{
    // Now create a dynamic body to bounce on the floor
    // Note that this uses the shorthand version of creating and adding a body to
    // the world
    const JPH::BodyCreationSettings kinematic_settings(
        new JPH::BoxShape(JPH::Vec3(kinematic_dimensions.x, kinematic_dimensions.y, kinematic_dimensions.z)),
        //JPH::RVec3(0.0_r, 2.0_r, 0.0_r),
        JPH::RVec3(kinematic_positon.x, kinematic_positon.y, kinematic_positon.z),
        JPH::Quat::sEulerAngles(JPH::RVec3(kinematic_rotation.x * DEG2RAD, kinematic_rotation.y * DEG2RAD, kinematic_rotation.z * DEG2RAD)),
        JPH::EMotionType::Kinematic,
        Layers::MOVING);
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::BodyID kinematic_id = body_interface.CreateAndAddBody(kinematic_settings,
                                                 JPH::EActivation::Activate);
    _kinematic_ids.push_back(kinematic_id);
    return kinematic_id;
}

JPH::BodyID PhysicsEngine::create_sensor(const Vector3 &sensor_dimensions,
                                const Vector3 &sensor_positon,
                                const Vector3 &sensor_rotation)
{
    // Now create a dynamic body to bounce on the floor
    // Note that this uses the shorthand version of creating and adding a body to
    // the world
    const JPH::BodyCreationSettings sensor_settings(
        new JPH::BoxShape(JPH::Vec3(sensor_dimensions.x, sensor_dimensions.y, sensor_dimensions.z)),
        //JPH::RVec3(0.0_r, 2.0_r, 0.0_r),
        JPH::RVec3(sensor_positon.x, sensor_positon.y, sensor_positon.z),
        JPH::Quat::sEulerAngles(JPH::RVec3(sensor_rotation.x * DEG2RAD, sensor_rotation.y * DEG2RAD, sensor_rotation.z * DEG2RAD)),
        JPH::EMotionType::Static,
        Layers::MOVING);
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::Body *sensor = body_interface.CreateBody(sensor_settings);
    sensor->SetIsSensor(true);
    JPH::BodyID sensor_id = sensor->GetID();
    body_interface.AddBody(sensor_id, JPH::EActivation::Activate);
    _sensor_ids.push_back(sensor_id);
    return sensor_id;
}

void PhysicsEngine::create_hinge(const Vector3 &hinge_position,
                                const JPH::BodyID &body1_id,
                                const JPH::BodyID &body2_id, int hingeaxis, int normalaxis)
{
    JPH::HingeConstraintSettings hinge_settings;
    hinge_settings.mPoint1 = hinge_settings.mPoint2 = JPH::Vec3(hinge_position.x, hinge_position.y, hinge_position.z);
    hinge_settings.mHingeAxis1 = hinge_settings.mHingeAxis2 = AxisEnum[hingeaxis];
    hinge_settings.mNormalAxis1 = hinge_settings.mNormalAxis2 = AxisEnum[normalaxis];
    JPH::BodyID body_ids[] = { body1_id, body2_id };
    JPH::BodyLockMultiWrite lock(_physics_system->GetBodyLockInterface(), body_ids, 2);
    JPH::Body* body1 = lock.GetBody(0);
    JPH::Body* body2 = lock.GetBody(1);
    JPH::HingeConstraint* hinge_constraint = dynamic_cast<JPH::HingeConstraint*>(hinge_settings.Create(*body1, *body2));
    _physics_system->AddConstraint(hinge_constraint);
}

void PhysicsEngine::create_tracked_chassis(const Vector3 &hinge_position,
                                const JPH::BodyID &body1_id,
                                const JPH::BodyID &body2_id, int hingeaxis, int normalaxis)
{
    JPH::VehicleConstraintSettings vehicle;
}

void PhysicsEngine::set_dynamic_linear_velocity(JPH::BodyID dynamicid, const Vector3 dynamic_velocity) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.SetLinearVelocity(
        dynamicid,
        JPH::Vec3(dynamic_velocity.x, dynamic_velocity.y, dynamic_velocity.z));
}

void PhysicsEngine::set_dynamic_angular_velocity(JPH::BodyID dynamicid, const Vector3 dynamic_velocity) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.SetAngularVelocity(
        dynamicid,
        JPH::Vec3(dynamic_velocity.x * DEG2RAD, dynamic_velocity.y * DEG2RAD, dynamic_velocity.z * DEG2RAD));
}

void PhysicsEngine::add_dynamic_linear_velocity(JPH::BodyID dynamicid, const Vector3 dynamic_velocity) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.AddLinearVelocity(
        dynamicid,
        JPH::Vec3(dynamic_velocity.x, dynamic_velocity.y, dynamic_velocity.z));
}

void PhysicsEngine::add_dynamic_angular_velocity(JPH::BodyID dynamicid, const Vector3 dynamic_velocity) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.AddAngularImpulse(
        dynamicid,
        JPH::Vec3(dynamic_velocity.x * DEG2RAD, dynamic_velocity.y * DEG2RAD, dynamic_velocity.z * DEG2RAD));
}

Vector3 PhysicsEngine::get_dynamic_linear_velocity(JPH::BodyID dynamicid) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::Vec3 linearvelocity = body_interface.GetLinearVelocity(dynamicid);
    return Vector3(linearvelocity.GetX(), linearvelocity.GetY(), linearvelocity.GetZ());
}

Vector3 PhysicsEngine::get_dynamic_angular_velocity(JPH::BodyID dynamicid) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::Vec3 angularvelocity = body_interface.GetAngularVelocity(dynamicid);
    return Vector3(angularvelocity.GetX() * RAD2DEG, angularvelocity.GetY() * RAD2DEG, angularvelocity.GetZ() * RAD2DEG);
}

void PhysicsEngine::set_body_position(JPH::BodyID bodyid, const Vector3 position) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.SetPosition(
        bodyid,
        JPH::Vec3(position.x, position.y, position.z),
        JPH::EActivation::Activate);
}

void PhysicsEngine::set_body_rotation(JPH::BodyID bodyid, const Vector3 rotation) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.SetRotation(
        bodyid,
        JPH::Quat::sEulerAngles(JPH::RVec3(rotation.x * DEG2RAD, rotation.y * DEG2RAD, rotation.z * DEG2RAD)),
        JPH::EActivation::Activate);
}

Vector3 PhysicsEngine::get_body_position(JPH::BodyID bodyid) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::Vec3 position = body_interface.GetPosition(bodyid);
    return Vector3(position.GetX(), position.GetY(), position.GetZ());
}

Vector3 PhysicsEngine::get_body_rotation(JPH::BodyID bodyid) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::Quat rotation = body_interface.GetRotation(bodyid);
    JPH::Vec3 eulerrotation = rotation.GetEulerAngles();
    return Vector3(eulerrotation.GetX() * RAD2DEG, eulerrotation.GetY() * RAD2DEG, eulerrotation.GetZ() * RAD2DEG);
}

Vector3 PhysicsEngine::get_body_cofm(JPH::BodyID bodyid) {
    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    JPH::Vec3 cofm = body_interface.GetCenterOfMassPosition(bodyid);
    return Vector3(cofm.GetX(), cofm.GetY(), cofm.GetZ());
}

void PhysicsEngine::set_dynamic_linear_velocity_with_rotation(JPH::BodyID dynamicid, const Vector3 dynamic_velocity, const Vector3 dynamic_rotation) {
    float yawRad = dynamic_rotation.y * DEG2RAD;
    float pitchRad = dynamic_rotation.x * DEG2RAD;

    Vector3 real_velocity = Vector3(0, 0, 0);
    real_velocity.x = dynamic_velocity.x * std::sin(yawRad) * std::cos(pitchRad);
    real_velocity.y = dynamic_velocity.y * std::sin(pitchRad);
    real_velocity.z = dynamic_velocity.z * std::cos(yawRad) * std::cos(pitchRad);

    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.SetLinearVelocity(
        dynamicid,
        JPH::Vec3(real_velocity.z, real_velocity.y, real_velocity.x));
}

void PhysicsEngine::add_dynamic_linear_velocity_with_rotation(JPH::BodyID dynamicid, const Vector3 dynamic_velocity, const Vector3 dynamic_rotation) {
    float yawRad = dynamic_rotation.y * DEG2RAD;
    float pitchRad = dynamic_rotation.x * DEG2RAD;

    Vector3 real_velocity = Vector3(0, 0, 0);
    real_velocity.x = dynamic_velocity.x * std::sin(yawRad) * std::cos(pitchRad);
    real_velocity.y = dynamic_velocity.y * std::sin(pitchRad);
    real_velocity.z = dynamic_velocity.z * std::cos(yawRad) * std::cos(pitchRad);

    std::cout << real_velocity.x << " " << real_velocity.y << " " << real_velocity.z << std::endl;

    JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
    body_interface.AddLinearVelocity(
        dynamicid,
        JPH::Vec3(real_velocity.z, real_velocity.y, real_velocity.x));
}

void PhysicsEngine::start_simulation()
{
    _physics_system->OptimizeBroadPhase();
}

bool PhysicsEngine::update(const float cDeltaTime, Vector3 &body_position, Vector3 &body_rotation, JPH::BodyID body_id)
{
    ++_step;

    const JPH::BodyInterface &body_interface{
        _physics_system->GetBodyInterface()};
    if (!body_interface.IsActive(body_id))
    {
        //std::cout << "Body " << body_id.GetIndex() << " is not active" << std::endl;
        return false;
    }

    // Output current position and velocity of the sphere
    const JPH::RVec3 position{
        body_interface.GetCenterOfMassPosition(body_id)};
    const JPH::Quat rotation{
        body_interface.GetRotation(body_id)};
    const JPH::Vec3 velocity{body_interface.GetLinearVelocity(body_id)};
    //std::cout << "Body " << body_id.GetIndex() << " Step " << _step << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")\n" << std::endl;

    body_position =
        Vector3{position.GetX(), position.GetY(), position.GetZ()};
    JPH::Vec3 eulerrotation = rotation.GetEulerAngles();
    body_rotation = Vector3{eulerrotation.GetX() * RAD2DEG, eulerrotation.GetY() * RAD2DEG, eulerrotation.GetZ() * RAD2DEG};
    // If you take larger steps than 1 / 60th of a second you need to do
    // multiple collision steps in order to keep the simulation stable. Do 1
    // collision step per 1 / 60th of a second (round up).
    constexpr int cCollisionSteps{1};

    // Step the world
    JPH::TempAllocatorImpl temp_allocator{10 * 1'024 * 1'024};
    _physics_system->Update(cDeltaTime,
                            cCollisionSteps,
                            &temp_allocator,
                            _job_system.get());
    return true;
}

void PhysicsEngine::cleanup()
{
    JPH::BodyInterface &body_interface{_physics_system->GetBodyInterface()};

    for (JPH::BodyID id : _static_ids) {
        // Remove the sphere from the physics system. Note that the sphere itself
        // keeps all of its state and can be re-added at any time.
        body_interface.RemoveBody(id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(id);
    }

    for (JPH::BodyID id : _dynamic_ids) {
        // Remove the sphere from the physics system. Note that the sphere itself
        // keeps all of its state and can be re-added at any time.
        body_interface.RemoveBody(id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(id);
    }

    for (JPH::BodyID id : _kinematic_ids) {
        // Remove the sphere from the physics system. Note that the sphere itself
        // keeps all of its state and can be re-added at any time.
        body_interface.RemoveBody(id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(id);
    }

    for (JPH::BodyID id : _sensor_ids) {
        // Remove the sphere from the physics system. Note that the sphere itself
        // keeps all of its state and can be re-added at any time.
        body_interface.RemoveBody(id);

        // Destroy the sphere. After this the sphere ID is no longer valid.
        body_interface.DestroyBody(id);
    }

    // Remove and destroy the floor
    body_interface.RemoveBody(_floor_id);
    body_interface.DestroyBody(_floor_id);

    // Unregisters all types with the factory and cleans up the default material
    JPH::UnregisterTypes();

    // Destroy the factory
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}
