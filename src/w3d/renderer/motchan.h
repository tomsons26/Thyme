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
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }
    void Get_Vector(int, float *);
    //not sure if these exist anywhere as code but noting for future
    //void Get_Vector_As_Quat(int, class Quaternion &){};
    void Set_Identity(float *);
    void Do_Data_Compression(int){};//code exists but i don't get how to do it

private:
    unsigned int PivotIdx;
    unsigned int Type;
    int VectorLen;
    int UnusedFloat1;
    int UnusedFloat2;
    short *UnusedBuffer;
    float *Data;
    int FirstFrame;
    int LastFrame;
};

class BitChannelClass : W3DMPO
{
public:
    BitChannelClass();
    virtual ~BitChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }
    //not sure if this exists anywhere as code but noting for future
    int Get_Bit(int frame);

private:
    unsigned int PivotIdx;
    unsigned int Type;
    int DefaultVal;
    int FirstFrame;
    int LastFrame;
    unsigned char *Bits;
};

class TimeCodedMotionChannelClass : W3DMPO
{
public:
    TimeCodedMotionChannelClass();
    virtual ~TimeCodedMotionChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    void Set_Identity(float *setvec);
    unsigned int Get_Index(unsigned int timecode);
    unsigned int Binary_Search_Index(unsigned int timecode);

private:
    unsigned int PivotIdx;
    unsigned int Type;
    int VectorLen;
    unsigned int PacketSize;
    unsigned int NumTimeCodes;
    unsigned int LastTimeCodeIdx;
    unsigned int CachedIdx;
    unsigned int *Data;
};

class TimeCodedBitChannelClass : W3DMPO
{
public:
    TimeCodedBitChannelClass();
    virtual ~TimeCodedBitChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }
    int Get_Bit(int frame);

private:
    unsigned int PivotIdx;
    unsigned int Type;
    int DefaultVal;
    unsigned int NumTimeCodes;
    unsigned int CachedIdx;
    unsigned int *Bits;
};

class AdaptiveDeltaMotionChannelClass : W3DMPO
{
public:
    AdaptiveDeltaMotionChannelClass();
    virtual ~AdaptiveDeltaMotionChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    unsigned int Estimate_Size();
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }
    void Get_Vector(float frame, float *setvec);
    Quaternion Get_Quat_Vector(float frame_idx);
    float Get_Frame(unsigned int frame_idx, unsigned int vector_idx);
    void Decompress(unsigned int src_idx, float *srcdata, unsigned int frame_idx, float *outdata);
    void Decompress(unsigned int frame_idx, float *outdata);

private:
    unsigned int PivotIdx;
    unsigned int Type;
    int VectorLen;
    unsigned int NumFrames;
    float Scale;
    unsigned int *Data;
    unsigned int CacheFrame;
    float *CacheData;
};

// Following is from BFME2 WB
struct W3dCompressedMotionChannelStruct
{
    char zero; // must be 0 or bails reading
    char Type;
    char VectorLen;
    char Channel;
    short NumTimeCodes; // union? NumFrames for AD?
    short Pivot;
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
    unsigned int Get_Channel() { return Channel; }
    unsigned int Get_Pivot() { return PivotIdx; }
    static MotionChannelClassBase *Read_Motion_Channel(ChunkLoadClass &cload);

protected:
    unsigned int Channel;
    unsigned int PivotIdx;
    unsigned int NumTimeCodes;
    unsigned int VectorLen;
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
    short *Data1;
    unsigned int *Data2;
};

class MotionChannelAdaptiveDelta : public MotionChannelClassBase
{
public:
    MotionChannelAdaptiveDelta();
    virtual bool Load_W3D(ChunkLoadClass &cload) override;
    virtual ~MotionChannelAdaptiveDelta() override{};
    virtual unsigned int Size() override { return 8 * VectorLen + 4; };
    virtual void Get_Scalar() override{};
    virtual void Get_Vector() override{};
    virtual void Get_Quaternion() override{};

protected:
    float Scale;
    float floats[4]; // dunno
    unsigned int *Data;
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
