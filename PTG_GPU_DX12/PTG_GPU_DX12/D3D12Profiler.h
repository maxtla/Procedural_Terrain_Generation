#pragma once
class D3D12Profiler
{
public:
	static bool Init(UINT count, UINT countCompute);
	static void Release();

	static void Begin();
	static void End();

	static void BeginCompute();
	static void EndCompute();

	static void Timestamp(UINT index);
	static void MapData();
	static void UnmapData();
	static double GetDuration(UINT beginTS, UINT endTS);

	static void TimestampCompute(UINT index);
	static void MapDataCompute();
	static void UnmapDataCompute();
	static double GetDurationCompute(UINT beginTS, UINT endTS);

private:
	D3D12Profiler();
	~D3D12Profiler();

	static bool m_initialized;

	static bool m_hasBegun;
	static bool m_hasEnded;
	static bool m_hasAquiredData;

	static bool m_hasBegunCompute;
	static bool m_hasEndedCompute;
	static bool m_hasAquiredDataCompute;

	static ID3D12QueryHeap * m_queryHeap;
	static ID3D12Resource * m_queryBuffer;

	static UINT64 m_gpuFrequency;
	static UINT64 * m_queryData;
	static UINT m_count;

	static ID3D12QueryHeap * m_queryHeapCompute;
	static ID3D12Resource * m_queryBufferCompute;

	static UINT64 m_gpuFrequencyCompute;
	static UINT64 * m_queryDataCompute;
	static UINT m_countCompute;
};

