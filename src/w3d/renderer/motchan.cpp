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
    PivotIdx(0),
    Type(0),
    VectorLen(0),
    UnusedFloat1(0),
    UnusedFloat2(0),
    UnusedBuffer(nullptr),
    Data(nullptr),
    FirstFrame(-1),
    LastFrame(-1)
{
}

MotionChannelClass::~MotionChannelClass()
{
    Free();
}

void MotionChannelClass::Free()
{
    if (UnusedBuffer) {
        delete[] UnusedBuffer;
        UnusedBuffer = nullptr;
    }

    if (Data) {
        delete[] Data;
        Data = nullptr;
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

    FirstFrame = chan.first_frame;
    LastFrame = chan.last_frame;
    VectorLen = chan.vector_len;
    Type = chan.flags;
    PivotIdx = chan.pivot;
    unsigned int bytes = 4 * VectorLen * (LastFrame - FirstFrame + 1);
    unsigned int bytesleft = bytes - 4;
    Data = new float[bytes];
    Data[0] = chan.data[0];

    if (cload.Read(&Data[1], bytesleft) != bytesleft) {
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
    return 4 * VectorLen * (LastFrame - FirstFrame + 1) + sizeof(MotionChannelClass);
}

// From Ren
void MotionChannelClass::Get_Vector(int a2, float *a3)
{
    if (a2 < FirstFrame || a2 > LastFrame) {
        Set_Identity(a3);
    } else {
        int v3 = a2 - FirstFrame;

        if (Data) {
            for (int i = 0; i < VectorLen; ++i) {
                a3[i] = Data[i + VectorLen * v3];
            }
        } else {
            long double v5 = UnusedFloat2 / 65535.0;
            for (int j = 0; j < VectorLen; ++j) {
                a3[j] = v5 * UnusedBuffer[j + VectorLen * v3] + UnusedFloat1;
            }
        }
    }
}

void MotionChannelClass::Set_Identity(float *setvec)
{
    setvec = 0;
    if (Type == 6) {
        setvec[0] = 0;
        setvec[1] = 0;
        setvec[2] = 0;
        setvec[3] = 1.0;
    }
}

BitChannelClass::BitChannelClass() : PivotIdx(0), Type(0), DefaultVal(0), FirstFrame(-1), LastFrame(-1), Bits(nullptr) {}

BitChannelClass::~BitChannelClass()
{
    Free();
}

void BitChannelClass::Free()
{
    if (Bits) {
        delete[] Bits;
        Bits = nullptr;
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
    FirstFrame = chan.first_frame;
    LastFrame = chan.last_frame;
    Type = chan.flags;
    PivotIdx = chan.pivot;
    DefaultVal = chan.default_val;
    // need to clean this up
    unsigned int bytes = ((LastFrame - FirstFrame + 1) + 7) / 8;
    unsigned int bytesleft = bytes - 1;
    DEBUG_ASSERT((sizeof(W3dBitChannelStruct) + bytesleft) == chunk_size);
    Bits = new unsigned char[bytes];
    //
    DEBUG_ASSERT(Bits);
    Bits[0] = chan.data[0];

    if (bytesleft && cload.Read(&Bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int BitChannelClass::Estimate_Size()
{
    return ((LastFrame - FirstFrame + 1) + 7) / 8 + sizeof(BitChannelClass);
}

int BitChannelClass::Get_Bit(int frame)
{
    if (frame < FirstFrame || frame > LastFrame) {
        return DefaultVal;
    }

    return ((1 * ((frame - LOBYTE(FirstFrame)) & 7)) & Bits[(frame - FirstFrame) / 8]) != 0;
}

TimeCodedMotionChannelClass::TimeCodedMotionChannelClass() :
    PivotIdx(0),
    Type(0),
    VectorLen(0),
    PacketSize(0),
    NumTimeCodes(0),
    LastTimeCodeIdx(0),
    CachedIdx(0),
    Data(nullptr)
{
}

TimeCodedMotionChannelClass::~TimeCodedMotionChannelClass() {}

void TimeCodedMotionChannelClass::Free()
{
    if (Data) {
        delete[] Data;
        Data = nullptr;
    }
}

bool TimeCodedMotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dTimeCodedAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);
    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    NumTimeCodes = chan.num_timecodes;
    VectorLen = chan.vector_len;
    Type = chan.flags;
    PivotIdx = chan.pivot;
    PacketSize = VectorLen + 1;
    CachedIdx = 0;
    LastTimeCodeIdx = PacketSize * (NumTimeCodes - 1);
    Data = new unsigned int[((chunk_size / 4) + 1)];
    Data[0] = chan.data[0];

    if (cload.Read(&Data[1], chunk_size) != chunk_size) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int TimeCodedMotionChannelClass::Estimate_Size()
{
    return 4 * PacketSize * NumTimeCodes + sizeof(TimeCodedMotionChannelClass);
}

void TimeCodedMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    int index = Get_Index(frame);
    if (index == PacketSize * (NumTimeCodes - 1)) {
        float *data = (float *)&Data[index + 1];
        for (int i = 0; i < VectorLen; ++i) {
            setvec[i] = data[i];
        }
    } else {
        int index2 = PacketSize + index;
        unsigned int val = Data[index2];

        if (GameMath::Fast_Is_Float_Positive(val)) {
            float v8 = (Data[index] & 0x7FFFFFFF);
            float *data1 = (float *)&Data[index + 1];
            float *data2 = (float *)&Data[index2 + 1];
            for (int k = 0; k < VectorLen; ++k) {
                float f1 = (val & 0x7FFFFFFF);
                float f2 = (frame - v8) / (f1 - v8);
                setvec[k] = GameMath::Lerp(data1[k], data2[k], f2);
            }
        } else {
            float *data3 = (float *)&Data[index + 1];

            for (int j = 0; j < VectorLen; ++j) {
                setvec[j] = data3[j];
            }

        }
    }
}

Quaternion TimeCodedMotionChannelClass::Get_Quat_Vector(float frame_idx)
{
    DEBUG_ASSERT(VectorLen == 4);
    Quaternion q1(true);
    unsigned int a2a = frame_idx;
    int index = Get_Index(a2a);
    if (index == PacketSize * (NumTimeCodes - 1)) {
        Quaternion *dq1 = (Quaternion *)&Data[index + 1];
        q1.Set(dq1->X, dq1->Y, dq1->Z, dq1->W);
        return q1;
    } else {
        int index2 = PacketSize + index;
        unsigned int val = Data[index2];
        if (GameMath::Fast_Is_Float_Positive(val)) {
            float v10 = (Data[index] & 0x7FFFFFFF);
            float v9 = (val & 0x7FFFFFFF);
            float alpha = (frame_idx - v10) / (v9 - v10);
            Quaternion *dq3 = (Quaternion *)&Data[index + 1];
            Quaternion *dq4 = (Quaternion *)&Data[index2 + 1];
            Quaternion q2(true);
            Quaternion q3(true);
            q2.Set(dq3->X, dq3->Y, dq3->Z, dq3->W);
            q3.Set(dq4->X, dq4->Y, dq4->Z, dq4->W);
            Fast_Slerp(q1, q2, q3, alpha);
            return q1;
        } else {
            Quaternion *dq2 = (Quaternion *)&Data[index + 1];
            q1.Set(dq2->X, dq2->Y, dq2->Z, dq2->W);
            return q1;
        }
    }
}

void TimeCodedMotionChannelClass::Set_Identity(float *setvec)
{
    setvec = 0;
    if (Type == 6) {
        setvec[0] = 0;
        setvec[1] = 0;
        setvec[2] = 0;
        setvec[3] = 1.0;
    }
}

unsigned int TimeCodedMotionChannelClass::Get_Index(unsigned int timecode)
{
    unsigned int result;

    DEBUG_ASSERT(CachedIdx <= LastTimeCodeIdx);
    if (timecode < (Data[CachedIdx] & 0x7FFFFFFF)) {
        goto LABEL_15;
    }

    if (CachedIdx == LastTimeCodeIdx) {
        return CachedIdx;
    }

    if (timecode < (Data[PacketSize + CachedIdx] & 0x7FFFFFFF)) {
        return CachedIdx;
    }

    CachedIdx += PacketSize;
    if (CachedIdx == LastTimeCodeIdx) {
        return CachedIdx;
    }

    if (timecode >= (Data[PacketSize + CachedIdx] & 0x7FFFFFFF)) {
    LABEL_15:
        CachedIdx = Binary_Search_Index(timecode);
        result = CachedIdx;
    } else {
        result = CachedIdx;
    }

    return result;
}

unsigned int TimeCodedMotionChannelClass::Binary_Search_Index(unsigned int timecode)
{
    int result;
    int count2;

    int count = 0;
    int rightIdx = NumTimeCodes - 2;
    if (timecode >= (Data[LastTimeCodeIdx] & 0x7FFFFFFF)) {
        result = LastTimeCodeIdx;
    } else {
        while (1) {
            while (1) {
                count2 = PacketSize * (count + ((rightIdx - count) >> 1));
                if (timecode >= (Data[count2] & 0x7FFFFFFF)) {
                    break;
                }
                rightIdx = count + ((rightIdx - count) >> 1);
            }
            if (timecode < (Data[PacketSize + count2] & 0x7FFFFFFF)) {
                break;
            }
            if (count + ((rightIdx - count) >> 1) == count) {
                ++count;
            } else {
                count += (rightIdx - count) >> 1;
            }
        }
        result = PacketSize * (count + ((rightIdx - count) >> 1));
    }
    return result;
}

TimeCodedBitChannelClass::TimeCodedBitChannelClass() :
    PivotIdx(0),
    Type(0),
    DefaultVal(0),
    NumTimeCodes(0),
    CachedIdx(0),
    Bits(nullptr)
{
}

TimeCodedBitChannelClass::~TimeCodedBitChannelClass()
{
    Free();
}

void TimeCodedBitChannelClass::Free()
{
    if (Bits) {
        delete[] Bits;
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

    NumTimeCodes = chan.num_timecodes;
    Type = chan.flags;
    PivotIdx = chan.pivot;
    DefaultVal = chan.default_val;
    CachedIdx = 0;
    int bytesleft = 4 * NumTimeCodes - 4;
    DEBUG_ASSERT((sizeof(chan) + bytesleft) == (unsigned)chunk_size);
    Bits = new unsigned int[NumTimeCodes];
    DEBUG_ASSERT(Bits);
    Bits[0] = chan.data[0];

    if (bytesleft && cload.Read(&Bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int TimeCodedBitChannelClass::Estimate_Size()
{
    return 4 * NumTimeCodes + sizeof(TimeCodedBitChannelClass);
}

int TimeCodedBitChannelClass::Get_Bit(int frame)
{
    DEBUG_ASSERT(frame >= 0);
    DEBUG_ASSERT(CachedIdx < NumTimeCodes);
    unsigned int count = 0;

    if (frame >= (Bits[CachedIdx] & 0x7FFFFFFF)) {
        count = CachedIdx + 1;
    }

    while (count < NumTimeCodes && frame >= (Bits[count] & 0x7FFFFFFF)) {
        ++count;
    }

    int index = count - 1;
    if (index < 0) {
        index = 0;
    }

    CachedIdx = index;
    return (Bits[index] & 0x80000000) == 0x80000000;
}

AdaptiveDeltaMotionChannelClass::AdaptiveDeltaMotionChannelClass() :
    PivotIdx(0),
    Type(0),
    VectorLen(0),
    NumFrames(0),
    Scale(0),
    Data(nullptr),
    CacheFrame(0),
    CacheData(nullptr)
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
    if (Data) {
        delete[] Data;
        Data = nullptr;
    }
    if (CacheData) {
        delete CacheData;
        CacheData = nullptr;
    }
}

bool AdaptiveDeltaMotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    W3dAdaptiveDeltaAnimChannelStruct chan;
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(chan);

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return false;
    }

    VectorLen = chan.vector_len;
    Type = chan.flags;
    PivotIdx = chan.pivot;
    NumFrames = chan.num_frames;
    Scale = chan.scale;
    CacheFrame = 0x7FFFFFFF; // todo find out what this is
    CacheData = new float[2*VectorLen];
    Data = new unsigned int[((chunk_size / 4) + 1)]; // fix this
    Data[0] = chan.data[0];

    if (cload.Read(&Data[1], chunk_size) != chunk_size) {
        Free();
        return false;
    }

    return true;
}

// from BFME2 WB, used by NodeMotionStruct's
unsigned int AdaptiveDeltaMotionChannelClass::Estimate_Size()
{
    // needs to be verified it uses CacheFrame, think it could be chunksize..
    return CacheFrame + 8 * VectorLen + sizeof(AdaptiveDeltaMotionChannelClass);
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

    if (frame_idx >= NumFrames) {
        frame_idx = NumFrames - 1;
    }
    if (CacheFrame == frame_idx) {
        return CacheData[vector_idx];
    } else if (CacheFrame + 1 == frame_idx) {
        return CacheData[VectorLen + vector_idx];
    } else if (frame_idx < CacheFrame) {
        Decompress(frame_idx, CacheData);

        if (frame_idx != NumFrames - 1) {
            Decompress(frame_idx, CacheData, frame_idx + 1, &CacheData[VectorLen]);
        }

        CacheFrame = frame_idx;
        return CacheData[vector_idx];
    } else if (frame_idx == CacheFrame + 2) {
        memcpy(CacheData, &CacheData[VectorLen], 4 * VectorLen);
        Decompress(++CacheFrame, CacheData, frame_idx, &CacheData[VectorLen]);
        return CacheData[vector_idx + VectorLen];
    } else {
        DEBUG_ASSERT(VectorLen <= 4);
        memcpy(Dst, &CacheData[VectorLen], 4 * VectorLen);
        Decompress(CacheFrame, Dst, frame_idx, CacheData);
        CacheFrame = frame_idx;

        if (frame_idx != NumFrames - 1) {
            Decompress(CacheFrame, CacheData, frame_idx + 1, &CacheData[VectorLen]);
        }

        return CacheData[vector_idx];
    }
}

void AdaptiveDeltaMotionChannelClass::Decompress(
    unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata)
{
    char v7[4];

    DEBUG_ASSERT(src_idx < frame_idx);
    unsigned int src_idxa = src_idx + 1;
    float *base = (float *)&Data[VectorLen];
    bool done = 0;

    for (int i = 0; i < VectorLen; ++i) {
        float *v14 = (float *)((char *)base + 9 * i + ((src_idxa - 1) >> 4) * 9 * VectorLen);
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
                float scale = filtertable[v10] * Scale;
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
            v14 = (float *)((char *)v15 + 9 * VectorLen - 1);
        }
        outdata[i] = v12;
    }
}

void AdaptiveDeltaMotionChannelClass::Decompress(unsigned int frame_idx, float *outdata)
{
    char v5[4];

    float *srcdata = (float *)Data;
    bool done = 0;

    for (int i = 0; i < VectorLen; ++i) {
        float *v12 = (float *)((char *)Data + 9 * i + 4 * VectorLen);
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
                float scale = filtertable[v9] * Scale;
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
            v12 = (float *)((char *)v13 + 9 * VectorLen - 1);
        }
        outdata[i] = v11;
    }
}

MotionChannelClassBase::MotionChannelClassBase() : Channel(-1), PivotIdx(-1) {}

MotionChannelClassBase *MotionChannelClassBase::Read_Motion_Channel(ChunkLoadClass &cload)
{
    W3dCompressedMotionChannelStruct chan;

    if (cload.Read(&chan, sizeof(chan)) != sizeof(chan)) {
        return nullptr;
    } else if (chan.zero != 0) {
        return nullptr;
    }

    MotionChannelClassBase *motchan;
    switch (chan.Type) {
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
        motchan->Channel = chan.Channel;
        motchan->PivotIdx = chan.Pivot;
        motchan->NumTimeCodes = chan.NumTimeCodes;
        motchan->VectorLen = chan.VectorLen;
        if (!motchan->Load_W3D(cload)) {
            delete motchan;
        }
        return motchan;
    }

    return nullptr;
}

MotionChannelTimeCoded::MotionChannelTimeCoded() : MotionChannelClassBase(), Data1(nullptr), Data2(nullptr) {}

bool MotionChannelTimeCoded::Load_W3D(ChunkLoadClass &cload)
{
    unsigned int size1 = 2 * NumTimeCodes;
    unsigned int size2 = 4 * VectorLen * NumTimeCodes;
    Data1 = new short[size1];
    Data2 = new unsigned int[size2];

    if (cload.Read(Data1, size1) != size1) {
        return false;
    }

    if (NumTimeCodes & 1) {
        cload.Seek(sizeof(short));
    }

    if (cload.Read(Data1, size2) != size2) {
        return false;
    }

    return true;
}

unsigned int MotionChannelTimeCoded::Estimate_Size()
{
    return NumTimeCodes * (4 * VectorLen + 2) + sizeof(MotionChannelTimeCoded);
}

MotionChannelAdaptiveDelta::MotionChannelAdaptiveDelta() : MotionChannelClassBase(), Data(nullptr) {}

bool MotionChannelAdaptiveDelta::Load_W3D(ChunkLoadClass &cload)
{
    DEBUG_ASSERT(VectorLen <= 4);

    if (cload.Read(&Scale, sizeof(Scale)) != sizeof(Scale)) {
        return false;
    }

    if (cload.Read(floats, 4 * VectorLen) != 4 * VectorLen) {
        return false;
    }

    unsigned int size = cload.Cur_Chunk_Length() - 12 - 4 * VectorLen;
    Data = new unsigned int[size];

    if (cload.Read(Data, size) != size) {
        return false;
    }

    return true;
}

MotionChannelAdaptiveDelta4::MotionChannelAdaptiveDelta4() : MotionChannelAdaptiveDelta() {}

unsigned int MotionChannelAdaptiveDelta4::Estimate_Size()
{
    return 9 * VectorLen * ((NumTimeCodes + 15) / 16) + 4;
}

MotionChannelAdaptiveDelta8::MotionChannelAdaptiveDelta8() : MotionChannelAdaptiveDelta() {}

unsigned int MotionChannelAdaptiveDelta8::Estimate_Size()
{
    return 17 * VectorLen * ((NumTimeCodes + 15) / 16) + 4;
}
