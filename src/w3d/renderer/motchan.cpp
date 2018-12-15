#include "motchan.h"
#include "gamedebug.h"
#include "gamemath.h"
#include "quaternion.h"

MotionChannelClass::MotionChannelClass() :
    PivotIdx(0),
    Type(0),
    VectorLen(0),
    UnusedInt1(0),
    UnusedInt2(0),
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

// not sure if this belongs here....
int MotionChannelClass::A4E450()
{
    //?
    return 4 * VectorLen * (LastFrame - FirstFrame + 1) + 0x20;
}

// ZH version
bool MotionChannelClass::Load_W3D(ChunkLoadClass &cload)
{
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(W3dAnimChannelStruct);
    W3dAnimChannelStruct chan;
    if (cload.Read(&chan, sizeof(W3dAnimChannelStruct)) != sizeof(W3dAnimChannelStruct)) {
        return false;
    }
    FirstFrame = chan.FirstFrame;
    LastFrame = chan.LastFrame;
    VectorLen = chan.VectorLen;
    Type = chan.Flags;
    PivotIdx = chan.Pivot;
    unsigned int bytes = 4 * VectorLen * (LastFrame - FirstFrame + 1);
    unsigned int bytesleft = bytes - 4;
    Data = new float[bytes];
    Data[0] = chan.Data[0];
    if (cload.Read(&Data[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }
    if (chunk_size != bytesleft) {
        cload.Seek(chunk_size - bytesleft);
    }
    return 1;
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
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(W3dBitChannelStruct);
    W3dBitChannelStruct chan;
    if (cload.Read(&chan, sizeof(W3dBitChannelStruct)) != sizeof(W3dBitChannelStruct)) {
        return false;
    }
    FirstFrame = chan.FirstFrame;
    LastFrame = chan.LastFrame;
    Type = chan.Flags;
    PivotIdx = chan.Pivot;
    DefaultVal = chan.DefaultVal;
    // need to clean this up
    unsigned int bytes = ((LastFrame - FirstFrame + 1) + 7) >> 3;
    unsigned int bytesleft = bytes - 1;
    DEBUG_ASSERT((sizeof(W3dBitChannelStruct) + bytesleft) == cload.Cur_Chunk_Length());
    Bits = new unsigned char[bytes];
    //
    DEBUG_ASSERT(Bits);
    Bits[0] = chan.Data[0];
    if (bytesleft && cload.Read(&Bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }
    return 1;
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
    return false;
}

void TimeCodedMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    int index = get_index(frame);
    if (index == PacketSize * (NumTimeCodes - 1)) {
        float *data = (float *)&Data[index + 1];
        for (int i = 0; i < VectorLen; ++i) {
            setvec[i] = data[i];
        }
    } else {
        int index2 = PacketSize + index;
        unsigned int val = Data[index2];
        if (!(val & 0x80000000)) {
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

Quaternion TimeCodedMotionChannelClass::Get_QuatVector(float frame_idx)
{
    DEBUG_ASSERT(VectorLen==4);
    Quaternion q1(true);
    unsigned int a2a = frame_idx;
    int index = get_index(a2a);
    if (index == PacketSize * (NumTimeCodes - 1)) {
        Quaternion *dq1 = (Quaternion *)&Data[index + 1];
        q1.Set(dq1->X, dq1->Y, dq1->Z, dq1->W);
        return q1;
    } else {
        int index2 = PacketSize + index;
        unsigned int val = Data[index2];
        if (val & 0x80000000) {
            Quaternion *dq2 = (Quaternion *)&Data[index + 1];
            q1.Set(dq2->X, dq2->Y, dq2->Z, dq2->W);
            return q1;
        } else {
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
        }
    }
}

void TimeCodedMotionChannelClass::set_identity(float *setvec) {}

unsigned int TimeCodedMotionChannelClass::get_index(unsigned int timecode)
{
    unsigned int result;

    DEBUG_ASSERT(CachedIdx<=LastTimeCodeIdx);
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
        CachedIdx = binary_search_index(timecode);
        result = CachedIdx;
    } else {
        result = CachedIdx;
    }
    return result;
}

unsigned int TimeCodedMotionChannelClass::binary_search_index(unsigned int timecode)
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
    Free();
    unsigned int chunk_size = cload.Cur_Chunk_Length();
    W3dTimeCodedBitChannelStruct chan;
    if (cload.Read(&chan, sizeof(W3dTimeCodedBitChannelStruct)) != sizeof(W3dTimeCodedBitChannelStruct)) {
        return false;
    }
    NumTimeCodes = chan.NumTimeCodes;
    Type = chan.Flags;
    PivotIdx = chan.Pivot;
    DefaultVal = chan.DefaultVal;
    CachedIdx = 0;
    int bytesleft = 4 * NumTimeCodes - 4;
    DEBUG_ASSERT((sizeof(W3dTimeCodedBitChannelStruct) + bytesleft) == (unsigned)chunk_size);
    Bits = new unsigned int[4 * NumTimeCodes];
    DEBUG_ASSERT(Bits);
    Bits[0] = chan.Data[0];
    if (bytesleft && cload.Read(&Bits[1], bytesleft) != bytesleft) {
        Free();
        return false;
    }

    return true;
}

int TimeCodedBitChannelClass::Get_Bit(int frame)
{
    DEBUG_ASSERT(frame>=0);
    DEBUG_ASSERT(CachedIdx < NumTimeCodes);
    int v3 = 0;
    if (frame >= (Bits[CachedIdx] & 0x7FFFFFFF)) {
        v3 = CachedIdx + 1;
    }
    while (v3 < NumTimeCodes && frame >= (Bits[v3] & 0x7FFFFFFF)) {
        ++v3;
    }
    int v4 = v3 - 1;
    if (v4 < 0) {
        v4 = 0;
    }
    CachedIdx = v4;
    return (Bits[v4] & 0x80000000) == 0x80000000;
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
    /* fixpzl
    if (!table_valid) {
        for (i = 0; i < 240; ++i) {
            filtertable[i + 16] = 1.0 - GameMath::Sin(i / 240.0 * DEG_TO_RAD(90.0));
        }
        table_valid = 1;
    }
    */
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
    unsigned int chunk_size = cload.Cur_Chunk_Length() - sizeof(W3dAdaptiveDeltaAnimChannelStruct);
    W3dAdaptiveDeltaAnimChannelStruct chan;
    if (cload.Read(&chan, sizeof(W3dAdaptiveDeltaAnimChannelStruct)) != sizeof(W3dAdaptiveDeltaAnimChannelStruct)) {
        return false;
    }
    VectorLen = chan.VectorLen;
    Type = chan.Flags;
    PivotIdx = chan.Pivot;
    NumFrames = chan.NumFrames;
    Scale = chan.Scale;
    CacheFrame = 0x7FFFFFFF; // todo find out what this is
    CacheData = new float[8 * VectorLen];
    Data = new unsigned int[4 * ((chunk_size >> 2) + 1)]; // fix this
    Data[0] = chan.Data[0];
    if (cload.Read(&Data[1], chunk_size) != chunk_size) {
        Free();
        return 0;
    }
    return 1;
}

void AdaptiveDeltaMotionChannelClass::Get_Vector(float frame, float *setvec)
{
    unsigned int v3 = frame;
	float v4 = frame-frame;//wat
    float value1 = AdaptiveDeltaMotionChannelClass::getframe(v3, 0);
    float value2 = AdaptiveDeltaMotionChannelClass::getframe(v3 + 1, 0);
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

Quaternion AdaptiveDeltaMotionChannelClass::Get_QuatVector(float frame_idx)
{
    unsigned int frame = frame_idx;
    unsigned int next_frame = frame + 1;
    float alpha = frame_idx - frame; // wat
    Quaternion q2(true);
    float a = getframe(frame, 0);
    float b = getframe(frame, 1);
    float c = getframe(frame, 2);
    float d = getframe(frame, 3);
    q2.Set(a, b, c, d);
    Quaternion q3(true);
    float na = getframe(next_frame, 0);
    float nb = getframe(next_frame, 1);
    float nc = getframe(next_frame, 2);
    float nd = getframe(next_frame, 3);
    q3.Set(na, nb, nc, nd);
    Quaternion q(true);
    Fast_Slerp(q, q2, q3, alpha);
    return q;
}

float AdaptiveDeltaMotionChannelClass::getframe(unsigned int frame_idx, unsigned int vector_idx)
{
    double result;
    float Dst[4];

    if (frame_idx >= NumFrames) {
        frame_idx = NumFrames - 1;
    }
    if (CacheFrame == frame_idx) {
        result = CacheData[vector_idx];
    } else if (CacheFrame + 1 == frame_idx) {
        result = CacheData[VectorLen + vector_idx];
    } else if (frame_idx < CacheFrame) {
        decompress(frame_idx, CacheData);
        if (frame_idx != NumFrames - 1) {
            decompress(frame_idx, CacheData, frame_idx + 1, &CacheData[VectorLen]);
        }
        CacheFrame = frame_idx;
        result = CacheData[vector_idx];
    } else if (frame_idx == CacheFrame + 2) {
        memcpy(CacheData, &CacheData[VectorLen], 4 * VectorLen);
        decompress(++CacheFrame, CacheData, frame_idx, &CacheData[VectorLen]);
        result = CacheData[vector_idx + VectorLen];
    } else {
		DEBUG_ASSERT(VectorLen<=4);
        memcpy(Dst, &CacheData[VectorLen], 4 * VectorLen);
        decompress(CacheFrame, Dst, frame_idx, CacheData);
        CacheFrame = frame_idx;
        if (frame_idx != NumFrames - 1) {
            decompress(CacheFrame, CacheData, frame_idx + 1, &CacheData[VectorLen]);
        }
        result = CacheData[vector_idx];
    }
    return result;
}

void AdaptiveDeltaMotionChannelClass::decompress(
    unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata)
{
    // hyelp
}

void AdaptiveDeltaMotionChannelClass::decompress(unsigned int frame_idx, float *outdata)
{
    // hyelp
}
