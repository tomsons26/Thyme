#pragma once
#include "always.h"
#include "chunkio.h"
#include "w3dmpo.h"

// to move
struct W3dAnimChannelStruct
{
    unsigned short FirstFrame;
    unsigned short LastFrame;
    unsigned short VectorLen;
    unsigned short Flags;
    unsigned short Pivot;
    unsigned short pad;
    float Data[1];
};

struct W3dBitChannelStruct
{
    unsigned short FirstFrame;
    unsigned short LastFrame;
    unsigned short Flags;
    unsigned short Pivot;
    unsigned int DefaultVal;
    unsigned int Data[1];
};

struct W3dTimeCodedAnimChannelStruct {

	unsigned int NumTimeCodes;
	unsigned short Pivot;
	unsigned char VectorLen;
	unsigned char Flags;
	unsigned int Data[1];
};

struct W3dTimeCodedBitChannelStruct {

	unsigned int NumTimeCodes;
	unsigned short Pivot;
	unsigned char Flags;
	unsigned char DefaultVal;
	unsigned int Data[1];
};


struct W3dAdaptiveDeltaAnimChannelStruct {
	unsigned int NumFrames;
	unsigned short Pivot;
	unsigned char VectorLen;
	unsigned char Flags;
	float Scale;
	unsigned int Data[1];
};

class MotionChannelClass : W3DMPO
{
public:
    MotionChannelClass();
    ~MotionChannelClass();
    void Free();
    int A4E450();
    bool Load_W3D(ChunkLoadClass &cload);
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }
	void Get_Vector(int, float *) {};
	void Get_Vector_As_Quat(int, class Quaternion &) {};
	void set_identity(float *) {};
	void Do_Data_Compression(int) {};

private:
    unsigned int PivotIdx;
    unsigned int Type;
    int VectorLen;
    int UnusedInt1;
    int UnusedInt2;
    float *UnusedBuffer;
    float *Data;
    int FirstFrame;
    int LastFrame;
};

class BitChannelClass : W3DMPO
{
public:
    BitChannelClass();
    ~BitChannelClass();
    void Free();
    bool Load_W3D(ChunkLoadClass &cload);
    int Get_Type() { return Type; }
    int Get_Pivot() { return PivotIdx; }

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
	~TimeCodedMotionChannelClass();
	void Free();
	bool Load_W3D(ChunkLoadClass& cload);
	int Get_Type() { return Type; }
	int Get_Pivot() { return PivotIdx; }
	void Get_Vector(float frame, float* setvec);
	Quaternion Get_QuatVector(float frame_idx);
	void set_identity(float* setvec);
	unsigned int get_index(unsigned int timecode);
	unsigned int binary_search_index(unsigned int timecode);

private:
	unsigned int PivotIdx;
	unsigned int Type;
	int VectorLen;
	unsigned int PacketSize;
	unsigned int NumTimeCodes;
	unsigned int LastTimeCodeIdx;
	unsigned int CachedIdx;
	unsigned int* Data;
};

class TimeCodedBitChannelClass : W3DMPO
{
public:
	TimeCodedBitChannelClass();
	~TimeCodedBitChannelClass();
	void Free();
	bool Load_W3D(ChunkLoadClass& cload);
	int Get_Type() { return Type; }
	int Get_Pivot() { return PivotIdx; }
	int Get_Bit(int frame);
	

private:
	unsigned int PivotIdx;
	unsigned int Type;
	int DefaultVal;
	unsigned int NumTimeCodes;
	unsigned int CachedIdx;
	unsigned int* Bits;
};

class AdaptiveDeltaMotionChannelClass : W3DMPO
{
public:
	AdaptiveDeltaMotionChannelClass();
	~AdaptiveDeltaMotionChannelClass();
	void Free();
	bool Load_W3D(ChunkLoadClass& cload);
	int Get_Type() { return Type; }
	int Get_Pivot() { return PivotIdx; }
	void Get_Vector(float frame, float* setvec);
	Quaternion Get_QuatVector(float frame_idx);
	float getframe(unsigned int frame_idx, unsigned int vector_idx);
	void decompress(unsigned int src_idx, float* srcdata, unsigned int frame_idx, float* outdata);
	void decompress(unsigned int frame_idx, float* outdata);

private:
	unsigned int PivotIdx;
	unsigned int Type;
	int VectorLen;
	unsigned int NumFrames;
	float Scale;
	unsigned int* Data;
	unsigned int CacheFrame;
	float* CacheData;
};