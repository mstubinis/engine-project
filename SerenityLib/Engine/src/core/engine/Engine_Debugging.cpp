#include <core/engine/Engine_Debugging.h>
#include <SFML/Window.hpp>
#include <iomanip>
#include <sstream>

#include <chrono>
#include <ctime>

using namespace Engine;
using namespace std;

epriv::DebugManager::DebugManager(const char* name, uint w, uint h){ 
    clock = sf::Clock();
    output = "";
    m_logicTime = m_physicsTime = m_renderTime = m_soundTime = m_displayTime = 0;

    m_deltaTime = 0;
    m_totalTime = 0.0;
    divisor = 1000000.0;

    output_frame_delay = 4;
    output_frame = 0;
    decimals = 4;
}
epriv::DebugManager::~DebugManager() {
}
void epriv::DebugManager::_init(const char* name, uint w, uint h) {
	GLuint cast = static_cast<GLuint>(queryID);
    glGenQueries(1, &cast);
    glQueryCounter(queryID, GL_TIMESTAMP);// dummy query to prevent OpenGL errors from popping out 
}

void epriv::DebugManager::addDebugLine(const char* message) {
    text_queue.emplace_back(message);
}
void epriv::DebugManager::addDebugLine(string& message) {
    text_queue.emplace_back(message);
}
void epriv::DebugManager::addDebugLine(string message) {
    text_queue.emplace_back(message);
}
void epriv::DebugManager::calculate() {
    m_deltaTime = (m_logicTime + m_physicsTime + m_renderTime + m_displayTime + m_soundTime);
    m_totalTime += dt();
    ++output_frame;
    if (output_frame >= output_frame_delay) {
        output_frame = 0;
    }
}
void epriv::DebugManager::beginGLQuery() { 
    glBeginQuery(GL_TIME_ELAPSED, queryID); 
}
void epriv::DebugManager::endGLQuery(const char* tag) { 
    string msg = string(tag) + ": %f ms\n";
    glEndQuery(GL_TIME_ELAPSED);
    glGetQueryObjectuiv(queryID, GL_QUERY_RESULT, &queryObject);
    printf(msg.c_str(), queryObject / 1000000.0);
}
void epriv::DebugManager::stop_clock() { clock.restart(); }

void epriv::DebugManager::calculate_logic() { m_logicTime = clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_physics() { m_physicsTime = clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_sounds() { m_soundTime = clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_render() { m_renderTime = clock.restart().asMicroseconds(); }
void epriv::DebugManager::calculate_display() { m_displayTime = clock.restart().asMicroseconds(); }

const double epriv::DebugManager::dt() const { return (double)((double)m_deltaTime / divisor); }
const double epriv::DebugManager::logicTime() const { return (double)((double)m_logicTime / divisor); }
const double epriv::DebugManager::physicsTime() const { return (double)((double)m_physicsTime / divisor); }
const double epriv::DebugManager::renderTime() const { return (double)((double)m_renderTime / divisor); }
const double epriv::DebugManager::soundsTime() const { return (double)((double)m_soundTime / divisor); }
const double epriv::DebugManager::displayTime() const { return (double)((double)m_displayTime / divisor); }
const double epriv::DebugManager::totalTime() const{ return m_totalTime; }

string epriv::DebugManager::timestamp() {
    const auto end = std::chrono::system_clock::now();
    const std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    return std::ctime(&end_time);
}
string& epriv::DebugManager::reportTime() { return reportTime(decimals); }
string& epriv::DebugManager::reportTime(uint _decimals) {
    decimals = _decimals;
    if ((output_frame >= output_frame_delay - 1) || output_frame_delay == 0) {
        uint fps = uint(1.0 / ((double)m_deltaTime / divisor));
        stringstream st1, st2, st3, st4, st5, st6;
        st1 << std::fixed << std::setprecision(decimals) << logicTime() * 1000.0;
        st2 << std::fixed << std::setprecision(decimals) << physicsTime() * 1000.0;
        st5 << std::fixed << std::setprecision(decimals) << soundsTime() * 1000.0;
        st3 << std::fixed << std::setprecision(decimals) << renderTime() * 1000.0;
        st4 << std::fixed << std::setprecision(decimals) << dt() * 1000.0;
        st6 << std::fixed << std::setprecision(decimals) << displayTime() * 1000.0;
        string s1 = st1.str(); string s2 = st2.str(); string s3 = st3.str(); string s4 = st4.str(); string s5 = st5.str(); string s6 = st6.str();

        output = "Update Time:  " + s1 + " ms" + "\nPhysics Time: " + s2 + " ms" + "\nSounds Time:  " + s5 + " ms" +
            "\nRender Time:  " + s3 + " ms" + "\nDisplay Time: " + s6 + " ms" + "\nDelta Time:   " + s4 + " ms" +
            "\nFPS: " + to_string(fps);
    }
    return output;
}
string epriv::DebugManager::reportDebug() {
    string out = "\n";
    for (auto& str : text_queue) {
        out += str + "\n";
    }
    text_queue.clear();
    return out;
}