#include "motchan.h"
#include "gamedebug.h"
#include "gamemath.h"

// clang-format off
static float filtertable[256] =
{
	1.0e-08f, 1.0e-07f, 1.0e-06f, 1.0e-05f,
		0.0001f, 0.001f, 0.01f, 0.1f,
		1.0f, 10.0f, 100.0f, 1000.0f,
		10000.0f, 100000.0f, 1000000.0f, 10000000.0f
};
// clang-format on

bool table_valid;

MotionChannelClass::MotionChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_vectorLen(0),
    m_unusedFloat1(0),
    m_unusedFloat2(0),
    m_unusedBuffer(nullptr),
    m_data(nullptr),
    m_firstFrame(-1),
    m_lastFrame(-1)
{
}

MotionChannelClass::~MotionChannelClass()
{
    Free();
}

void MotionChannelClass::Free()
{
    if (m_unusedBuffer) {
        delete[] m_unusedBuffer;
        m_unusedBuffer = nullptr;
    }

    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

// ZH version
bool MotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);
    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_firstFrame = chan.first_frame;
    m_lastFrame = chan.last_frame;
    m_vectorLen = chan.vector_len;
    m_type = chan.flags;
    m_pivotIdx = chan.pivot;
    unsigned int bytes = 4 * m_vectorLen * (m_lastFrame - m_firstFrame + 1);
    unsigned int bytesleft = bytes - 4;
    m_data = new float[bytes];
    m_data[0] = chan.data[0];

    if (cload.Read(&m_data[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    if (chunk_size != bytesleft) {
        cload.Seek(chunk_size - bytesleft);
    }

    // Do_Data_Compression(bytesleft);
    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int MotionChannelClass::Estimate_Size()
{
    return 4 * m_vectorLen * (m_lastFrame - m_firstFrame + 1) + sizeof(MotionChannelClass);
}

// From Ren
void MotionChannelClass::Get_Vector(int a2, float *a3)
{
    if (a2 < m_firstFrame || a2 > m_lastFrame) {
        Set_Identity(a3);
    } else {
        int v3 = a2 - m_firstFrame;

        if (m_data) {
            for (int i = 0; i < m_vectorLen; ++i) {
                a3[i] = m_data[i + m_vectorLen * v3];
            }
        } else {
            long double v5 = m_unusedFloat2 / 65535.0;
            for (int j = 0; j < m_vectorLen; ++j) {
                a3[j] = v5 * m_unusedBuffer[j + m_vectorLen * v3] + m_unusedFloat1;
            }
        }
    }
}

void MotionChannelClass::Set_Identity(float *setvec)
{
    setvec = 0;
    if (m_type == 6) {
        setvec[0] = 0;
        setvec[1] = 0;
        setvec[2] = 0;
        setvec[3] = 1.0;
    }
}

BitChannelClass::BitChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_defaultVal(0),
    m_firstFrame(-1),
    m_lastFrame(-1),
    m_bits(nullptr)
{
}

BitChannelClass::~BitChannelClass()
{
    Free();
}

void BitChannelClass::Free()
{
    if (m_bits) {
        delete[] m_bits;
        m_bits = nullptr;
    }
}

// BFME2 version
bool BitChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dBitChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length();
    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }
    m_firstFrame = chan.first_frame;
    m_lastFrame = chan.last_frame;
    m_type = chan.flags;
    m_pivotIdx = chan.pivot;
    m_defaultVal = chan.default_val;
    // need to clean this up
    unsigned int bytes = ((m_lastFrame - m_firstFrame + 1) + 7) / 8;
    unsigned int bytesleft = bytes - 1;
    DEBUG_ASSERT((sizeof(W3dBitChannelStruct) + bytesleft) == chunk_size);
    m_bits = new unsigned char[bytes];
    //
    DEBUG_ASSERT(m_bits);
    m_bits[0] = chan.data[0];

    if (bytesleft && cload.Read(&m_bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int BitChannelClass::Estimate_Size()
{
    return ((m_lastFrame - m_firstFrame + 1) + 7) / 8 + sizeof(BitChannelClass);
}

int BitChannelClass::Get_Bit(int frame)
{
    if (frame < m_firstFrame || frame > m_lastFrame) {
        return m_defaultVal;
    }

    return ((1 * ((frame - LOBYTE(m_firstFrame)) & 7)) & m_bits[(frame - m_firstFrame) / 8]) != 0;
}

TimeCodedMotionChannelClass::TimeCodedMotionChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_vectorLen(0),
    m_packetSize(0),
    m_numTimeCodes(0),
    m_lastTimeCodeIdx(0),
    m_cachedIdx(0),
    m_data(nullptr)
{
}

TimeCodedMotionChannelClass::~TimeCodedMotionChannelClass() {}

void TimeCodedMotionChannelClass::Free()
{
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
}

bool TimeCodedMotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dTimeCodedAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);
    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_numTimeCodes = chan.num_timecodes;
    m_vectorLen = chan.vector_len;
    m_type = chan.flags;
    m_pivotIdx = chan.pivot;
    m_packetSize = m_vectorLen + 1;
    m_cachedIdx = 0;
    m_lastTimeCodeIdx = m_packetSize * (m_numTimeCodes - 1);
    m_data = new unsigned int[((chunk_size / 4) + 1)];
    m_data[0] = chan.data[0];

    if (cload.Read(&m_data[1], chunk_size) != chunk_size) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int TimeCodedMotionChannelClass::Estimate_Size()
{
    return 4 * m_packetSize * m_numTimeCodes + sizeof(TimeCodedMotionChannelClass);
}

void TimeCodedMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    int index = Get_Index(frame);

    if (index == m_packetSize * (m_numTimeCodes - 1)) {
        float *data = (float *)&m_data[index + 1];
        for (int i = 0; i < m_vectorLen; ++i) {
            setvec[i] = data[i];
        }
    } else {
        int index2 = m_packetSize + index;
        unsigned int val = m_data[index2];

        if (GameMath::Fast_Is_Float_Positive(val)) {
            float v8 = (m_data[index] & 0x7FFFFFFF);
            float *data1 = (float *)&m_data[index + 1];
            float *data2 = (float *)&m_data[index2 + 1];
            for (int k = 0; k < m_vectorLen; ++k) {
                float f1 = (val & 0x7FFFFFFF);
                float f2 = (frame - v8) / (f1 - v8);
                setvec[k] = GameMath::Lerp(data1[k], data2[k], f2);
            }
        } else {
            float *data3 = (float *)&m_data[index + 1];

            for (int j = 0; j < m_vectorLen; ++j) {
                setvec[j] = data3[j];
            }

        }
    }
}

Quaternion TimeCodedMotionChannelClass::Get_Quat_Vector(float frame_idx)
{
    DEBUG_ASSERT(m_vectorLen == 4);
    Quaternion q1(true);
    unsigned int a2a = frame_idx;
    int index = Get_Index(a2a);
    if (index == m_packetSize * (m_numTimeCodes - 1)) {
        Quaternion *dq1 = (Quaternion *)&m_data[index + 1];
        q1.Set(dq1->X, dq1->Y, dq1->Z, dq1->W);
        return q1;
    } else {
        int index2 = m_packetSize + index;
        unsigned int val = m_data[index2];
        if (GameMath::Fast_Is_Float_Positive(val)) {
            float v10 = (m_data[index] & 0x7FFFFFFF);
            float v9 = (val & 0x7FFFFFFF);
            float alpha = (frame_idx - v10) / (v9 - v10);
            Quaternion *dq3 = (Quaternion *)&m_data[index + 1];
            Quaternion *dq4 = (Quaternion *)&m_data[index2 + 1];
            Quaternion q2(true);
            Quaternion q3(true);
            q2.Set(dq3->X, dq3->Y, dq3->Z, dq3->W);
            q3.Set(dq4->X, dq4->Y, dq4->Z, dq4->W);
            Fast_Slerp(q1, q2, q3, alpha);
            return q1;
        } else {
            Quaternion *dq2 = (Quaternion *)&m_data[index + 1];
            q1.Set(dq2->X, dq2->Y, dq2->Z, dq2->W);
            return q1;
        }
    }
}

void TimeCodedMotionChannelClass::Set_Identity(float *setvec)
{
    setvec = 0;
    if (m_type == 6) {
        setvec[0] = 0;
        setvec[1] = 0;
        setvec[2] = 0;
        setvec[3] = 1.0;
    }
}

unsigned int TimeCodedMotionChannelClass::Get_Index(unsigned int timecode)
{
    unsigned int result;

    DEBUG_ASSERT(m_cachedIdx <= m_lastTimeCodeIdx);
    if (timecode < (m_data[m_cachedIdx] & 0x7FFFFFFF)) {
        goto LABEL_15;
    }

    if (m_cachedIdx == m_lastTimeCodeIdx) {
        return m_cachedIdx;
    }

    if (timecode < (m_data[m_packetSize + m_cachedIdx] & 0x7FFFFFFF)) {
        return m_cachedIdx;
    }

    m_cachedIdx += m_packetSize;
    if (m_cachedIdx == m_lastTimeCodeIdx) {
        return m_cachedIdx;
    }

    if (timecode >= (m_data[m_packetSize + m_cachedIdx] & 0x7FFFFFFF)) {
    LABEL_15:
        m_cachedIdx = Binary_Search_Index(timecode);
        result = m_cachedIdx;
    } else {
        result = m_cachedIdx;
    }

    return result;
}

unsigned int TimeCodedMotionChannelClass::Binary_Search_Index(unsigned int timecode)
{
    int result;
    int count2;

    int count = 0;
    int rightIdx = m_numTimeCodes - 2;
    if (timecode >= (m_data[m_lastTimeCodeIdx] & 0x7FFFFFFF)) {
        result = m_lastTimeCodeIdx;
    } else {
        while (1) {
            while (1) {
                count2 = m_packetSize * (count + ((rightIdx - count) >> 1));
                if (timecode >= (m_data[count2] & 0x7FFFFFFF)) {
                    break;
                }
                rightIdx = count + ((rightIdx - count) >> 1);
            }
            if (timecode < (m_data[m_packetSize + count2] & 0x7FFFFFFF)) {
                break;
            }
            if (count + ((rightIdx - count) >> 1) == count) {
                ++count;
            } else {
                count += (rightIdx - count) >> 1;
            }
        }
        result = m_packetSize * (count + ((rightIdx - count) >> 1));
    }
    return result;
}

TimeCodedBitChannelClass::TimeCodedBitChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_defaultVal(0),
    m_numTimeCodes(0),
    m_cachedIdx(0),
    m_bits(nullptr)
{
}

TimeCodedBitChannelClass::~TimeCodedBitChannelClass()
{
    Free();
}

void TimeCodedBitChannelClass::Free()
{
    if (m_bits) {
        delete[] m_bits;
    }
}

bool TimeCodedBitChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dTimeCodedBitChannelStruct chan;
    Free();
    unsigned int chunk_size = cload.Cur_Chunk_Length();
    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_numTimeCodes = chan.num_timecodes;
    m_type = chan.flags;
    m_pivotIdx = chan.pivot;
    m_defaultVal = chan.default_val;
    m_cachedIdx = 0;
    int bytesleft = 4 * m_numTimeCodes - 4;
    DEBUG_ASSERT((sizeof(chan) + bytesleft) == (unsigned)chunk_size);
    m_bits = new unsigned int[m_numTimeCodes];
    DEBUG_ASSERT(m_bits);
    m_bits[0] = chan.data[0];

    if (bytesleft && cload.Read(&m_bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int TimeCodedBitChannelClass::Estimate_Size()
{
    return 4 * m_numTimeCodes + sizeof(TimeCodedBitChannelClass);
}

int TimeCodedBitChannelClass::Get_Bit(int frame)
{
    DEBUG_ASSERT(frame >= 0);
    DEBUG_ASSERT(m_cachedIdx < m_numTimeCodes);
    unsigned int count = 0;

    if (frame >= (m_bits[m_cachedIdx] & 0x7FFFFFFF)) {
        count = m_cachedIdx + 1;
    }

    while (count < m_numTimeCodes && frame >= (m_bits[count] & 0x7FFFFFFF)) {
        ++count;
    }

    int index = count - 1;
    if (index < 0) {
        index = 0;
    }

    m_cachedIdx = index;
    return (m_bits[index] & 0x80000000) == 0x80000000;
}

AdaptiveDeltaMotionChannelClass::AdaptiveDeltaMotionChannelClass() :
    m_pivotIdx(0),
    m_type(0),
    m_vectorLen(0),
    m_numFrames(0),
    m_scale(0),
    m_data(nullptr),
    m_cacheFrame(0),
    m_cacheData(nullptr)
{
    if (!table_valid) {
        for (int i = 0; i < 240; ++i) {
            filtertable[i + 16] = 1.0 - GameMath::Sin(i / 240.0 * DEG_TO_RAD(90.0));
        }
        table_valid = 1;
    }
}

AdaptiveDeltaMotionChannelClass::~AdaptiveDeltaMotionChannelClass()
{
    Free();
}

void AdaptiveDeltaMotionChannelClass::Free()
{
    if (m_data) {
        delete[] m_data;
        m_data = nullptr;
    }
    if (m_cacheData) {
        delete m_cacheData;
        m_cacheData = nullptr;
    }
}

bool AdaptiveDeltaMotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dAdaptiveDeltaAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    m_vectorLen = chan.vector_len;
    m_type = chan.flags;
    m_pivotIdx = chan.pivot;
    m_numFrames = chan.num_frames;
    m_scale = chan.scale;
    m_cacheFrame = 0x7FFFFFFF; // todo find out what this is
    m_cacheData = new float[2 * m_vectorLen];
    m_data = new unsigned int[((chunk_size / 4) + 1)]; // fix this
    m_data[0] = chan.data[0];

    if (cload.Read(&m_data[1], chunk_size) != chunk_size) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int AdaptiveDeltaMotionChannelClass::Estimate_Size()
{
    // needs to be verified it uses m_cacheFrame, think it could be chunksize..
    return m_cacheFrame + 8 * m_vectorLen + sizeof(AdaptiveDeltaMotionChannelClass);
}

void AdaptiveDeltaMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    unsigned int v3 = frame;
    float v4 = frame - frame; // wat
    float value1 = AdaptiveDeltaMotionChannelClass::Get_Frame(v3, 0);
    float value2 = AdaptiveDeltaMotionChannelClass::Get_Frame(v3 + 1, 0);
    // code in zh mac
    /*
    if (v3 < 0) { // help!
        *setvec = ((frame - ((v3 & 1 | (v3 >> 1)) + (v3 & 1 | (v3 >> 1)))) * (value2 - value1)) + value1;
    } else {
        *setvec = ((frame - v3) * (value2 - value1)) + value1;
    }
    */
    *setvec = GameMath::Lerp(value1, value2, v4);
}

Quaternion AdaptiveDeltaMotionChannelClass::Get_Quat_Vector(float frame_idx)
{
    unsigned int frame = frame_idx;
    unsigned int next_frame = frame + 1;
    float alpha = frame_idx - frame; // wat
    Quaternion q2;
    q2.Set(Get_Frame(frame, 0), Get_Frame(frame, 1), Get_Frame(frame, 2), Get_Frame(frame, 3));
    Quaternion q3;
    q3.Set(Get_Frame(next_frame, 0), Get_Frame(next_frame, 1), Get_Frame(next_frame, 2), Get_Frame(next_frame, 3));
    Quaternion q(true);
    Fast_Slerp(q, q2, q3, alpha);
    return q;
}

float AdaptiveDeltaMotionChannelClass::Get_Frame(unsigned int frame_idx, unsigned int vector_idx)
{
    float Dst[4];

    if (frame_idx >= m_numFrames) {
        frame_idx = m_numFrames - 1;
    }
    if (m_cacheFrame == frame_idx) {
        return m_cacheData[vector_idx];
    } else if (m_cacheFrame + 1 == frame_idx) {
        return m_cacheData[m_vectorLen + vector_idx];
    } else if (frame_idx < m_cacheFrame) {
        Decompress(frame_idx, m_cacheData);

        if (frame_idx != m_numFrames - 1) {
            Decompress(frame_idx, m_cacheData, frame_idx + 1, &m_cacheData[m_vectorLen]);
        }

        m_cacheFrame = frame_idx;
        return m_cacheData[vector_idx];
    } else if (frame_idx == m_cacheFrame + 2) {
        memcpy(m_cacheData, &m_cacheData[m_vectorLen], 4 * m_vectorLen);
        Decompress(++m_cacheFrame, m_cacheData, frame_idx, &m_cacheData[m_vectorLen]);
        return m_cacheData[vector_idx + m_vectorLen];
    } else {
        DEBUG_ASSERT(m_vectorLen <= 4);
        memcpy(Dst, &m_cacheData[m_vectorLen], 4 * m_vectorLen);
        Decompress(m_cacheFrame, Dst, frame_idx, m_cacheData);
        m_cacheFrame = frame_idx;

        if (frame_idx != m_numFrames - 1) {
            Decompress(m_cacheFrame, m_cacheData, frame_idx + 1, &m_cacheData[m_vectorLen]);
        }

        return m_cacheData[vector_idx];
    }
}

void AdaptiveDeltaMotionChannelClass::Decompress(
    unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata)
{
    char v7[4];

    DEBUG_ASSERT(src_idx < frame_idx);
    unsigned int src_idxa = src_idx + 1;
    float *base = (float *)&m_data[m_vectorLen];
    bool done = 0;

    for (int i = 0; i < m_vectorLen; ++i) {
        float *v14 = (float *)((char *)base + 9 * i + ((src_idxa - 1) >> 4) * 9 * m_vectorLen);
        int v13 = ((char)src_idxa - 1) & 0xF;
        float v12 = srcdata[i];
        unsigned int v11 = src_idxa;
        while (v11 <= frame_idx) {
            int v10 = *(char *)v14;
            float *v15 = (float *)((char *)v14 + 1);
            while (v13 < 0x10) {
                int v9 = v13 >> 1;
                if (v13 & 1) {
                    *(int *)v7 = (signed int)*((char *)v15 + v9) >> 4;
                } else {
                    v7[0] = *((char *)v15 + v9);
                }
                int v8 = v7[0] & 0xF;
                if (v8 & 8) {
                    v8 |= 0xFFFFFFF0;
                }
                float scale = filtertable[v10] * m_scale;
                float v6 = (double)v8 * scale;
                v12 = v12 + v6;
                if (v11 == frame_idx) {
                    done = 1;
                    break;
                }
                ++v11;
                ++v13;
            }
            v13 = 0;
            if (done) {
                break;
            }
            v14 = (float *)((char *)v15 + 9 * m_vectorLen - 1);
        }
        outdata[i] = v12;
    }
}

void AdaptiveDeltaMotionChannelClass::Decompress(unsigned int frame_idx, float *outdata)
{
    char v5[4];

    float *srcdata = (float *)m_data;
    bool done = 0;

    for (int i = 0; i < m_vectorLen; ++i) {
        float *v12 = (float *)((char *)m_data + 9 * i + 4 * m_vectorLen);
        float v11 = srcdata[i];
        unsigned int v10 = 1;
        while (v10 <= frame_idx) {
            int v9 = *(char *)v12;
            float *v13 = (float *)((char *)v12 + 1);
            for (int j = 0; j < 16; ++j) {
                signed int v7 = j >> 1;
                if (j & 1) {
                    *(int *)v5 = (signed int)*((char *)v13 + v7) >> 4;
                } else {
                    v5[0] = *((char *)v13 + v7);
                }
                int v6 = v5[0] & 0xF;
                if (v6 & 8) {
                    v6 |= 0xFFFFFFF0;
                }
                float scale = filtertable[v9] * m_scale;
                float v4 = (double)v6 * scale;
                v11 = v11 + v4;
                if (v10 == frame_idx) {
                    done = 1;
                    break;
                }
                ++v10;
            }
            if (done) {
                break;
            }
            v12 = (float *)((char *)v13 + 9 * m_vectorLen - 1);
        }
        outdata[i] = v11;
    }
}

MotionChannelClassBase::MotionChannelClassBase() : m_channel(-1), m_pivotIdx(-1) {}

MotionChannelClassBase *MotionChannelClassBase::Read_Motion_Channel(ChunkLoadClass &cload)
{
    W3dCompressedMotionChannelStruct chan;

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return nullptr;
    } else if (chan.m_zero != 0) {
        return nullptr;
    }

    MotionChannelClassBase *motchan;
    switch (chan.m_type) {
        case 0:
            motchan = new MotionChannelTimeCoded;
            break;
        case 1:
            motchan = new MotionChannelAdaptiveDelta4;
            break;
        case 2:
            motchan = new MotionChannelAdaptiveDelta8;
            break;
        default:
            return nullptr;
    }

    if (motchan) {
        motchan->m_channel = chan.m_channel;
        motchan->m_pivotIdx = chan.m_pivot;
        motchan->m_numTimeCodes = chan.m_numTimeCodes;
        motchan->m_vectorLen = chan.m_vectorLen;
        if (!motchan->Load_W3D(cload)) {
            delete motchan;
        }
        return motchan;
    }

    return nullptr;
}

MotionChannelTimeCoded::MotionChannelTimeCoded() : MotionChannelClassBase(), m_data1(nullptr), m_data2(nullptr) {}

bool MotionChannelTimeCoded::Load_W3D(ChunkLoadClass &cload)
{
    unsigned int size1 = 2 * m_numTimeCodes;
    unsigned int size2 = 4 * m_vectorLen * m_numTimeCodes;
    m_data1 = new short[size1];
    m_data2 = new unsigned int[size2];

    if (cload.Read(m_data1, size1) != size1) {
        return false;
    }

    if (m_numTimeCodes & 1) {
        cload.Seek(sizeof(short));
    }

    if (cload.Read(m_data1, size2) != size2) {
        return false;
    }

    return true;
}

unsigned int MotionChannelTimeCoded::Estimate_Size()
{
    return m_numTimeCodes * (4 * m_vectorLen + 2) + sizeof(MotionChannelTimeCoded);
}

MotionChannelAdaptiveDelta::MotionChannelAdaptiveDelta() : MotionChannelClassBase(), m_data(nullptr) {}

bool MotionChannelAdaptiveDelta::Load_W3D(ChunkLoadClass &cload)
{
    DEBUG_ASSERT(m_vectorLen <= 4);

    if (cload.Read(&m_scale, sizeof(m_scale)) != sizeof(m_scale)) {
        return false;
    }

    if (cload.Read(m_floats, 4 * m_vectorLen) != 4 * m_vectorLen) {
        return false;
    }

    unsigned int size = cload.Cur_Chunk_Length() - 12 - 4 * m_vectorLen;
    m_data = new unsigned int[size];

    if (cload.Read(m_data, size) != size) {
        return false;
    }

    return true;
}

MotionChannelAdaptiveDelta4::MotionChannelAdaptiveDelta4() : MotionChannelAdaptiveDelta() {}

unsigned int MotionChannelAdaptiveDelta4::Estimate_Size()
{
    return 9 * m_vectorLen * ((m_numTimeCodes + 15) / 16) + 4;
}

MotionChannelAdaptiveDelta8::MotionChannelAdaptiveDelta8() : MotionChannelAdaptiveDelta() {}

unsigned int MotionChannelAdaptiveDelta8::Estimate_Size()
{
    return 17 * m_vectorLen * ((m_numTimeCodes + 15) / 16) + 4;
}
