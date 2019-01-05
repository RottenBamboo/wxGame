#pragma once
#include <memory>

class DataBuffer
{
public:
	DataBuffer():m_pData(nullptr), m_szSize(0), m_szAlignment(alignof(unsigned int)) {};

	DataBuffer(size_t size, size_t alignment = 4) :m_szSize(size), m_szAlignment(alignment)
	{
		m_pData = new unsigned char[m_szSize];
		memset(m_pData, 0, size);
	}

	DataBuffer(void* pData, size_t size, size_t alignment = 4) : m_szSize(size), m_szAlignment(alignment)
	{
		if (pData)
		{;
			m_pData = new unsigned char[m_szSize];
			memcpy(m_pData, reinterpret_cast<unsigned char*>(pData), size);
		}
		else
		{
			m_pData = new unsigned char[m_szSize];
			memset(m_pData, 0, size);
		}
	};

	DataBuffer(const DataBuffer& rhs)
	{
		m_pData = new unsigned char[rhs.m_szSize];
		memcpy(m_pData, rhs.m_pData, rhs.m_szSize);
		m_szSize = rhs.m_szSize;
		m_szAlignment = rhs.m_szAlignment;
	}

	~DataBuffer()
	{
		delete[] m_pData;
		m_pData = nullptr;
	}

	const DataBuffer& operator=(const DataBuffer& rhs)
	{
		if (m_pData)
		{
			if (rhs.m_pData)	// if rhs has the m_pData member, then do the assignment
			{
				if (m_szSize > rhs.m_szSize)// && m_szAlignment == rhs.m_szAlignment)
				{
					memcpy(m_pData, rhs.m_pData, rhs.m_szSize);
					memset(&m_pData[rhs.m_szSize], 0, m_szSize - rhs.m_szSize);
					m_szSize = rhs.m_szSize;
					m_szAlignment = rhs.m_szAlignment;
				}
				else
				{
					delete[] m_pData;
					m_pData = new unsigned char[rhs.m_szSize];
					memcpy(m_pData, rhs.m_pData, rhs.m_szSize);
					m_szSize = rhs.m_szSize;
					m_szAlignment = rhs.m_szAlignment;
				}
			}
		}
		else
		{
			m_pData = new unsigned char[rhs.m_szSize];
			memcpy(m_pData, rhs.m_pData, rhs.m_szSize);
			m_szSize = rhs.m_szSize;
			m_szAlignment = rhs.m_szAlignment;
		}
		return *this;
	}

	DataBuffer& operator=(DataBuffer& rhs)
	{	
		if (m_pData)
		{
			if (rhs.m_pData)	// if rhs has the m_pData member, then do the assignment
			{
				if (m_szSize > rhs.m_szSize)// && m_szAlignment == rhs.m_szAlignment)
				{
					memcpy(m_pData, rhs.m_pData, rhs.m_szSize);
					memset(&m_pData[m_szSize], 0, m_szSize - rhs.m_szSize);
					m_szSize = rhs.m_szSize;
					m_szAlignment = rhs.m_szAlignment;
				}
				else
				{
					delete[] m_pData;
					m_pData = new unsigned char[rhs.m_szSize];
					memcpy(m_pData, rhs.m_pData, rhs.m_szSize); 
					m_szSize = rhs.m_szSize;
					m_szAlignment = rhs.m_szAlignment;
				}
			}
		}
		else
		{
			if (rhs.m_pData)
			{
				m_pData = new unsigned char[rhs.m_szSize];
				memcpy(m_pData, rhs.m_pData, rhs.m_szSize);
				m_szSize = rhs.m_szSize;
				m_szAlignment = rhs.m_szAlignment;
			}
		}
		return *this;
	}

	unsigned char* GetData()
	{
		return m_pData;
	}

	size_t GetDataSize(void) const
	{
		return m_szSize;
	};

protected:
	unsigned char* m_pData;
	size_t m_szSize;
	size_t m_szAlignment;
};

