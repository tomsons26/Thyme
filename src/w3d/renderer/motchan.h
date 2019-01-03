#pragma once
#include "always.h"
#include "chunkio.h"
#include "quaternion.h"
#include "w3dmpo.h"

// to move
struct W3dAnimChannelStruct
{
    unsigned short first_frame;
    unsigned short last_frame;
    unsigned short vector_len;
    unsigned short flags;
    unsigned short pivot;
    unsigned short pad;
    float data[1];
};

struct W3dBitChannelStruct
{
    unsigned short first_frame;
    unsigned short last_frame;
    unsigned short flags;
    unsigned short pivot;
    unsigned int default_val;
    unsigned int data[1];
};

struct W3dTimeCodedAnimChannelStruct
{
    unsigned int num_timecodes;
    unsigned short pivot;
    unsigned char vector_len;
    unsigned char flags;
    unsigned int data[1];
};

struct W3dTimeCodedBitChannelStruct
{
    unsigned int num_timecodes;
    unsigned short pivot;
    unsigned char flags;
    unsigned char default_val;
    unsigned int data[1];
};

struct W3dAdaptiveDeltaAnimChannelStruct
{
    unsigned int num_frames;
    unsigned short pivot;
    unsigned char vector_len;
    unsigned char flags;
    float scale;
    unsigned int data[1];
};

class MotionChannelClass : W3DMPO
{
public:
    MotionChannelClass();
    virtual ~MotionChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    void Get_Vector(int, float *);
    //not sure if these exist anywhere as code but noting for future
    //void Get_Vector_As_Quat(int, class Quaternion &){};
    void Set_Identity(float *);
    void Do_Data_Compression(int){};//code exists but i don't get how to do it

private:
    unsigned int m_pivotIdx;
    unsigned int m_type;
    int m_vectorLen;
    int m_unusedFloat1;
    int m_unusedFloat2;
    short *m_unusedBuffer;
    float *m_data;
    int m_firstFrame;
    int m_lastFrame;
};

class BitChannelClass : W3DMPO
{
public:
    BitChannelClass();
    virtual ~BitChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    //not sure if this exists anywhere as code but noting for future
    int Get_Bit(int frame);

private:
    unsigned int m_pivotIdx;
    unsigned int m_type;
    int m_defaultVal;
    int m_firstFrame;
    int m_lastFrame;
    unsigned char *m_bits;
};

class TimeCodedMotionChannelClass : W3DMPO
{
public:
    TimeCodedMotionChannelClass();
    virtual ~TimeCodedMotionChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    void Set_Identity(float *setvec);
    unsigned int Get_Index(unsigned int timecode);
    unsigned int Binary_Search_Index(unsigned int timecode);

private:
    unsigned int m_pivotIdx;
    unsigned int m_type;
    int m_vectorLen;
    unsigned int m_packetSize;
    unsigned int m_numTimeCodes;
    unsigned int m_lastTimeCodeIdx;
    unsigned int m_cachedIdx;
    unsigned int *m_data;
};

class TimeCodedBitChannelClass : W3DMPO
{
public:
    TimeCodedBitChannelClass();
    virtual ~TimeCodedBitChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    int Get_Bit(int frame);

private:
    unsigned int m_pivotIdx;
    unsigned int m_type;
    int m_defaultVal;
    unsigned int m_numTimeCodes;
    unsigned int m_cachedIdx;
    unsigned int *m_bits;
};

class AdaptiveDeltaMotionChannelClass : W3DMPO
{
public:
    AdaptiveDeltaMotionChannelClass();
    virtual ~AdaptiveDeltaMotionChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return m_type; }
    int Get_Pivot() { return m_pivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    float Get_Frame(unsigned int frame_idx, unsigned int vector_idx);
    void Decompress(unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata);
    void Decompress(unsigned int frame_idx, float *outdata);

private:
    unsigned int m_pivotIdx;
    unsigned int m_type;
    int m_vectorLen;
    unsigned int m_numFrames;
    float m_scale;
    unsigned int *m_data;
    unsigned int m_cacheFrame;
    float *m_cacheData;
};

// Following is from BFME2 WB
struct W3dCompressedMotionChannelStruct
{
    char m_zero; // must be 0 or bails reading
    char m_type;
    char m_vectorLen;
    char m_channel;
    short m_numTimeCodes; // union? NumFrames for AD?
    short m_pivot;
};

class MotionChannelClassBase
{
public:
    MotionChannelClassBase();
    virtual bool Load_W3D(ChunkLoadClass &cload) = 0;
    virtual ~MotionChannelClassBase(){};
    virtual unsigned int Size() = 0; // dunno what this size is of
    virtual void Get_Scalar() = 0;
    virtual void Get_Vector() = 0;
    virtual void Get_Quaternion() = 0;
    virtual unsigned int Estimate_Size() = 0;
    unsigned int Get_Channel() { return m_channel; }
    unsigned int Get_Pivot() { return m_pivotIdx; }
    static MotionChannelClassBase *Read_Motion_Channel(ChunkLoadClass &cload);

protected:
    unsigned int m_channel;
    unsigned int m_pivotIdx;
    unsigned int m_numTimeCodes;
    unsigned int m_vectorLen;
};

class MotionChannelTimeCoded : public MotionChannelClassBase
{
public:
    MotionChannelTimeCoded();
    virtual bool Load_W3D(ChunkLoadClass &cload) override;
    virtual ~MotionChannelTimeCoded() override{};
    virtual unsigned int Size() override { return 4; };
    virtual void Get_Scalar() override{};
    virtual void Get_Vector() override{};
    virtual void Get_Quaternion() override{};
    virtual unsigned int Estimate_Size() override;

private:
    short *m_data1;
    unsigned int *m_data2;
};

class MotionChannelAdaptiveDelta : public MotionChannelClassBase
{
public:
    MotionChannelAdaptiveDelta();
    virtual bool Load_W3D(ChunkLoadClass &cload) override;
    virtual ~MotionChannelAdaptiveDelta() override{};
    virtual unsigned int Size() override { return 8 * m_vectorLen + 4; };
    virtual void Get_Scalar() override{};
    virtual void Get_Vector() override{};
    virtual void Get_Quaternion() override{};

protected:
    float m_scale;
    float m_floats[4]; // dunno
    unsigned int *m_data;
};

class MotionChannelAdaptiveDelta4 : public MotionChannelAdaptiveDelta
{
public:
    MotionChannelAdaptiveDelta4();
    virtual ~MotionChannelAdaptiveDelta4() override{};
    virtual void Get_Scalar() override{};
    virtual void Get_Vector() override{};
    virtual void Get_Quaternion() override{};
    virtual unsigned int Estimate_Size() override;
};

class MotionChannelAdaptiveDelta8 : public MotionChannelAdaptiveDelta
{
public:
    MotionChannelAdaptiveDelta8();
    virtual ~MotionChannelAdaptiveDelta8() override{};
    virtual void Get_Scalar() override{};
    virtual void Get_Vector() override{};
    virtual void Get_Quaternion() override{};
    virtual unsigned int Estimate_Size() override;
};
