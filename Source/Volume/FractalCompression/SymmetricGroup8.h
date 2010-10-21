#ifndef _SYMMETRIC_GROUP8_H_
#define _SYMMETRIC_GROUP8_H_

#include <wtypes.h>
#include <map>

#define MASK(x) (0xf<<((x)<<2))
#define OFFSET(x) ((x)<< 2)

/*	
	Description: Permutation class on Symmetric Group 8.
*/
class CSymmetricGroup8
{
public:
	CSymmetricGroup8()
	{
		myValue = 0x87654321;
	}

	static CSymmetricGroup8& GetStaticInstance()
	{
		static CSymmetricGroup8 instance;
		return instance;
	}

	enum eSymmetricRotation
	{
		ESR_X, ESR_Y, ESR_Z
	};

	int ClassifySymmetricGroup(eSymmetricRotation rotation);
	int GetClassForPermutation(DWORD permutation) const;
	
private:
	bool next_permutation();

	void rotate(eSymmetricRotation rotation);

	inline BYTE ElementAt(int i)
	{
		return (myValue >> (i << 2)) & 0x0f;
	}

	void swap(int i, int j);

private:
	DWORD myValue;
	std::map<DWORD, int> myClasification;
};


#endif //_SYMMETRIC_GROUP8_H_