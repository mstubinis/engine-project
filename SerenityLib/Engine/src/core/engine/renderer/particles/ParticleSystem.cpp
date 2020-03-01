#include <core/engine/renderer/particles/ParticleSystem.h>
#include <core/engine/renderer/particles/ParticleEmitter.h>
#include <core/engine/renderer/particles/Particle.h>
#include <core/engine/renderer/particles/ParticleEmissionProperties.h>
#include <core/engine/renderer/pipelines/IRenderingPipeline.h>

#include <core/engine/mesh/Mesh.h>
#include <core/engine/materials/Material.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/system/Engine.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/system/Engine.h>

#include <execution>
#include <glm/gtx/norm.hpp>


using namespace std;
using namespace Engine;

priv::ParticleSystem::ParticleSystem() {
    m_ParticleEmitters.reserve(NUMBER_OF_PARTICLE_EMITTERS_LIMIT);
    m_Particles.reserve(NUMBER_OF_PARTICLE_LIMIT);

    MaterialToIndex.reserve(10);
    MaterialToIndexReverse.reserve(10);
    MaterialIDToIndex.reserve(10);

    const auto num_cores = Engine::priv::threading::hardware_concurrency();
    THREAD_PART_1.resize(num_cores);
    THREAD_PART_2.resize(num_cores);
    THREAD_PART_3.resize(num_cores);

    THREAD_PART_4.resize(num_cores);
    THREAD_PART_5.resize(num_cores);
    for (auto& _4 : THREAD_PART_4)
        _4.reserve(10);
    for (auto& _5 : THREAD_PART_5)
        _5.reserve(10);
}
priv::ParticleSystem::~ParticleSystem() {

}


void priv::ParticleSystem::internal_update_emitters(const float dt) {
    if (m_ParticleEmitters.size() == 0) {
        return;
    }

    auto lamda_update_emitter = [&](ParticleEmitter& emitter, const size_t& j, const size_t& k) {
        emitter.update(j, dt, *this, true);
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_emitter, m_ParticleEmitters, true);
}

void priv::ParticleSystem::internal_update_particles(const float dt, const Camera& camera) {
    if (m_Particles.size() == 0) {
        return;
    }

    //update individual particles
    auto lamda_update_particle = [&](Particle& particle, const size_t& j, const unsigned int k) {
        if (particle.m_Timer > 0.0f) {
            particle.m_Timer           += dt;
            auto& prop                  = *particle.m_EmitterSource->m_Properties;

            particle.m_Scale           += prop.m_ChangeInScaleFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);
            particle.m_Color            = prop.m_ColorFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);
            particle.m_AngularVelocity += prop.m_ChangeInAngularVelocityFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);
            particle.m_Angle           += particle.m_AngularVelocity;
            particle.m_Velocity        += prop.m_ChangeInVelocityFunctor(prop, particle.m_Timer, dt, particle.m_EmitterSource, particle);

            particle.m_Position        += (particle.m_Velocity * dt);

            if (particle.m_Timer >= prop.m_Lifetime) {
                particle.m_Timer = 0.0f;
                std::lock_guard lock(m_Mutex);
                m_ParticleFreelist.push(j);
            }
        }
    };
    priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_update_particle, m_Particles, true);
}

ParticleEmitter* priv::ParticleSystem::add_emitter(ParticleEmitter& emitter) {
    if (m_ParticleEmitterFreelist.size() > 0) { //first, try to reuse an empty
        const auto freeindex = m_ParticleEmitterFreelist.top();
        m_ParticleEmitterFreelist.pop();
        if (freeindex >= m_ParticleEmitters.size()) {
            return nullptr;
        }
        using std::swap;
        swap(m_ParticleEmitters[freeindex], emitter);
        return &m_ParticleEmitters[freeindex];
    }
    if (m_ParticleEmitters.size() < m_ParticleEmitters.capacity()) {
        m_ParticleEmitters.push_back(std::move(emitter));
        return &m_ParticleEmitters[m_ParticleEmitters.size() - 1];
    }
    return nullptr;
}
const bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter, const glm::vec3& emitterPosition, const glm::quat& emitterRotation) {
    if (m_ParticleFreelist.size() > 0) { //first, try to reuse an empty
        const auto freeindex = m_ParticleFreelist.top();
        m_ParticleFreelist.pop();
        if (freeindex >= m_Particles.size()) {
            return false;
        }
        m_Particles[freeindex].init(emitterPosition, emitterRotation, emitter);
        return true;
    }
    if (m_Particles.size() < m_Particles.capacity()) {
        m_Particles.emplace_back(emitterPosition, emitterRotation, emitter);
        return true;
    }
    return false;
}
const bool priv::ParticleSystem::add_particle(ParticleEmitter& emitter) {
    const auto& body = *emitter.getComponent<ComponentBody>();
    return add_particle(emitter, body.position(), body.rotation());
}

void priv::ParticleSystem::update(const float dt, const Camera& camera) {
    internal_update_particles(dt, camera);
    internal_update_emitters(dt);
}

template<class T1, class T2, class T3> void sort_multiple_vectors_at_once(vector<T1>& vin1, vector<T2>& vin2, vector<T3>& keys, const Camera& camera) {
    const glm::vec3 cameraPosition = glm::vec3(camera.getPosition());

    vector<std::size_t> indices; indices.reserve(vin1.size());
    for (auto&& unused : keys) {
        indices.push_back(indices.size());
    }

    auto lambda = [&](const std::size_t l, const std::size_t r) {
        const glm::vec3 position1 = glm::vec3(keys[l].x, keys[l].y, keys[l].z);
        const glm::vec3 position2 = glm::vec3(keys[r].x, keys[r].y, keys[r].z);
        return glm::distance2(position1, cameraPosition) > glm::distance2(position2, cameraPosition);
    };
    if (indices.size() < 100)
        std::sort(std::begin(indices), std::end(indices), lambda);
    else
        std::sort(std::execution::par_unseq, std::begin(indices), std::end(indices), lambda);


    vector<T1> r1; r1.reserve(vin1.size());
    for (size_t i = 0; i < vin1.size(); ++i) {
        r1.push_back(vin1[indices[i]]);
    }

    vector<T2> r2; r2.reserve(vin2.size());
    for (size_t i = 0; i < vin2.size(); ++i) {
        r2.push_back(vin2[indices[i]]);
    }

    vector<T3> r3; r3.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i) {
        r3.push_back(keys[indices[i]]);
    }

    vin1 = r1;
    vin2 = r2;
    keys = r3;
}
void priv::ParticleSystem::render(const Viewport& viewport, const Camera& camera, ShaderProgram& program, Renderer& renderer) {
    const auto particles_size = m_Particles.size();
    if (particles_size == 0 || !viewport.getRenderFlags().has(ViewportRenderingFlag::Particles)) {
        return;
    }

    //now cull, sort, and populate their render lists
    auto& planeMesh = priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getPlaneMesh();
    PositionAndScaleX.clear();
    ScaleYAndAngle.clear();
    MatIDAndPackedColor.clear();

    MaterialToIndex.clear();
    MaterialToIndexReverse.clear();
    MaterialIDToIndex.clear();

    if (PositionAndScaleX.capacity() < particles_size) {
        PositionAndScaleX.reserve(particles_size);
        ScaleYAndAngle.reserve(particles_size);
        MatIDAndPackedColor.reserve(particles_size);
    }

    //auto start = chrono::high_resolution_clock::now();

    const auto reserve_size = particles_size / Engine::priv::threading::hardware_concurrency();

    for (auto& _1 : THREAD_PART_1) _1.clear();
    for (auto& _2 : THREAD_PART_2) _2.clear();
    for (auto& _3 : THREAD_PART_3) _3.clear();
    for (auto& _4 : THREAD_PART_4) _4.clear();
    for (auto& _5 : THREAD_PART_5) _5.clear();

    for (auto& _1 : THREAD_PART_1) _1.reserve(reserve_size);
    for (auto& _2 : THREAD_PART_2) _2.reserve(reserve_size);
    for (auto& _3 : THREAD_PART_3) _3.reserve(reserve_size);

    const glm::vec3 camPos = glm::vec3(camera.getPosition());
    auto lamda_culler_particle = [&](Particle& particle, const size_t& j, const unsigned int k) {
        const float radius = planeMesh.getRadius() * Math::Max(particle.m_Scale.x, particle.m_Scale.y);
        const glm::vec3& pos = particle.position();
        const uint sphereTest = camera.sphereIntersectTest(pos, radius);
        const float comparison = radius * 3100.0f; //TODO: this is obviously different from the other culling functions
        if (particle.isActive() && (glm::distance2(pos, camPos) <= (comparison * comparison)) && sphereTest > 0) {
            //is is just pretty expensive in general...
            if (!THREAD_PART_4[k].count(particle.m_Material)) {
                THREAD_PART_4[k].try_emplace(particle.m_Material, particle.m_Material->id());
                THREAD_PART_5[k].try_emplace(particle.m_Material->id(), particle.m_Material);
            }
            ///////////////////////////////////////////

            THREAD_PART_1[k].emplace_back(pos.x - camPos.x, pos.y - camPos.y, pos.z - camPos.z, particle.m_Scale.x);
            THREAD_PART_2[k].emplace_back(particle.m_Scale.y, particle.m_Angle);
            THREAD_PART_3[k].emplace_back(THREAD_PART_4[k].at(particle.m_Material), particle.m_Color.toPackedInt());
        }
    };
    Core::m_Engine->m_ThreadManager.add_job_engine_controlled_split_vectored(lamda_culler_particle, m_Particles, true);

    //merge the thread collections into the main collections
    for (auto& _1 : THREAD_PART_1) { PositionAndScaleX.insert(PositionAndScaleX.end(), std::make_move_iterator(_1.begin()), std::make_move_iterator(_1.end())); }
    for (auto& _2 : THREAD_PART_2) { ScaleYAndAngle.insert(ScaleYAndAngle.end(), std::make_move_iterator(_2.begin()), std::make_move_iterator(_2.end())); }
    for (auto& _3 : THREAD_PART_3) { MatIDAndPackedColor.insert(MatIDAndPackedColor.end(), std::make_move_iterator(_3.begin()), std::make_move_iterator(_3.end())); }
    for (auto& _4 : THREAD_PART_4) { MaterialToIndex.merge(_4); }
    for (auto& _5 : THREAD_PART_5) { MaterialToIndexReverse.merge(_5); }

    //auto z = (chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - start));
    //float f = static_cast<float>(z.count()) / 1000000.0f;
    //Core::m_Engine->m_DebugManager.addDebugLine(to_string(f));

    //sorting
    sort_multiple_vectors_at_once(ScaleYAndAngle, MatIDAndPackedColor, PositionAndScaleX, camera);

    renderer.m_Pipeline->renderParticles(*this, camera, program);
}

vector<ParticleEmitter>& priv::ParticleSystem::getParticleEmitters() {
    return m_ParticleEmitters;
}
vector<Particle>& priv::ParticleSystem::getParticles() {
    return m_Particles;
}
stack<size_t>& priv::ParticleSystem::getParticleEmittersFreelist() {
    return m_ParticleEmitterFreelist;
}
stack<size_t>& priv::ParticleSystem::getParticlesFreelist() {
    return m_ParticleFreelist;
}