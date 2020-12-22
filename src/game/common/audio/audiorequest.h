/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object representing a request to the audio system.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "audioeventrts.h"
#include "mempoolobj.h"

union reqevent_t
{
    uintptr_t handle;
    AudioEventRTS *object;
};

enum RequestType
{
    REQUEST_PLAY,
    REQUEST_PAUSE,
    REQUEST_STOP,
};

class AudioRequest : public MemoryPoolObject
{
    IMPLEMENT_POOL(AudioRequest);
    friend class AudioManager;
    friend class MilesAudioManager;

protected:
    virtual ~AudioRequest() override {}

public:
    void Set_Music_Event_Object(AudioEventRTS *object)
    {
        m_event.object = object;
        m_requestType = REQUEST_PLAY;
    }
    void Set_Event_Handle(uintptr_t handle)
    {
        m_event.handle = handle;
        m_requestType = REQUEST_STOP;
    }
    void Set_Type(RequestType type) { m_requestType = type; }
    RequestType Request_Type() const { return m_requestType; }
    uintptr_t Event_Handle() const { return m_event.handle; }
    AudioEventRTS *Event_Object() const { return m_event.object; }
    bool Is_Adding() const { return m_adjustRequest; }

private:
    AudioRequest(bool adjust) : m_adjustRequest(adjust), m_adjustRequestCompleted(false) {}

private:
    RequestType m_requestType;
    reqevent_t m_event;
    bool m_adjustRequest;
    bool m_adjustRequestCompleted;
};