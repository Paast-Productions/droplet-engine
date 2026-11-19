/*****************************************************************//**
 * \file   Time.hpp
 * \brief  Header-only timer for >C++11 using chrono
 *********************************************************************/
#pragma once

#include <chrono>

class Timer {
public:
    using Clock    = std::chrono::steady_clock;  // monotonic: never jumps backwards
    using Duration = std::chrono::duration<double>;

    // maxDelta caps a single frame's delta (e.g. after a breakpoint, window
    // drag, or loading hitch) so physics doesn't explode. Use 0 to disable.
    explicit Timer(double maxDelta = 0.25)
        : m_start(Clock::now()), m_last(m_start), m_maxDelta(maxDelta) {}

    /// Call once per frame. Returns seconds since the previous tick
    /// (or since construction/reset), scaled by timeScale and clamped.
    double tick() {
        const Clock::time_point now = Clock::now();
        double dt = Duration(now - m_last).count();
        m_last = now;

        if (m_maxDelta > 0.0 && dt > m_maxDelta) dt = m_maxDelta;
        m_unscaledDelta = dt;

        dt = m_paused ? 0.0 : dt * m_timeScale;
        m_delta = dt;
        m_time += dt;
        ++m_frameCount;
        return dt;
    }

    /// Restart timing; the next tick() measures from now.
    void reset() {
        m_start = m_last = Clock::now();
        m_delta = m_unscaledDelta = m_time = 0.0;
        m_frameCount = 0;
    }

    double delta() const          { return m_delta; }          // last scaled dt (s)
    double unscaledDelta() const  { return m_unscaledDelta; }  // last real dt, clamped (s)
    double time() const           { return m_time; }           // accumulated game time (s)
    unsigned long long frameCount() const { return m_frameCount; }

    // Wall-clock seconds since construction/reset (ignores pause/scale).
    double elapsed() const { return Duration(Clock::now() - m_start).count(); }

    // Instantaneous FPS based on the last unscaled delta.
    double fps() const { return m_unscaledDelta > 0.0 ? 1.0 / m_unscaledDelta : 0.0; }

    void   setTimeScale(double s) { m_timeScale = s < 0.0 ? 0.0 : s; }
    double timeScale() const      { return m_timeScale; }

    void setPaused(bool p) { m_paused = p; }
    bool paused() const    { return m_paused; }

    void   setMaxDelta(double s) { m_maxDelta = s; }
    double maxDelta() const      { return m_maxDelta; }

private:
    Clock::time_point m_start;
    Clock::time_point m_last;
    double m_maxDelta;
    double m_timeScale     = 1.0;
    double m_delta         = 0.0;
    double m_unscaledDelta = 0.0;
    double m_time          = 0.0;
    unsigned long long m_frameCount = 0;
    bool   m_paused        = false;
};
