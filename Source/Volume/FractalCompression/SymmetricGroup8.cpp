#include "StdAfx.h"
#include "SymmetricGroup8.h"


/* 	The next permutation in Lexicographic order 
	-------------------------------------------
	Permutation f precedes a permutation g in the lexicographic (alphabetic) order 
	iff for the minimum value of k such that f(k) g(k), we have f(k) < g(k). 
	The algorithm is described in [Dijkstra, p. 71]. */
bool CSymmetricGroup8::next_permutation()
{
	/* Find the largest i */
	int i=6;
	while( i>=0 && ElementAt(i) > ElementAt(i+1) ) --i;

	/* If i is smaller than 0, then there are no more permutations. */
	if(i<0) return false;

	/* Find the largest element after vi but not larger than vi */
	int j=7;
	while( ElementAt(j) < ElementAt(i) ) --j;

	/* swap values at positions i and j */
	swap(i, j);

	i++; j=7;

	/* Swap the last n - i elements. */
	while (i < j)
	{
		swap(i, j);
		i++;
		j--;
	}

	return true;
}

/*	
	Rotate Symmetric Group about an axis.
*/
void CSymmetricGroup8::rotate(eSymmetricRotation rotation)
{
	switch(rotation)
	{
	case ESR_X: 	// rotateX = (1 5 7 3)(2 6 8 4)	eg. (1 2 3 4 5 6 7 8) -> (5 6 1 2 7 8 3 4)
		myValue = (myValue << 16) & 0xFF000000 | (myValue >> 8)&0x00FF0000 | (myValue << 8)&0x0000FF00 | (myValue >> 16)&0x000000FF;
		break;
	case ESR_Y:
		myValue = 0;
		break;
	case ESR_Z:
		myValue = 0;
	}
}

/*	
	Swap two elements using bit-masks
*/
void CSymmetricGroup8::swap(int i, int j)
{
	if( i==j ) return;
	else
		if(i > j)
			myValue = myValue & ~(MASK(i) | MASK(j)) | (myValue & MASK(i)) >> OFFSET(i-j) | (myValue & MASK(j)) << OFFSET(i-j);
		else
			myValue = myValue & ~(MASK(i) | MASK(j)) | (myValue & MASK(j)) >> OFFSET(j-i) | (myValue & MASK(i)) << OFFSET(j-i);
}

int CSymmetricGroup8::ClassifySymmetricGroup(eSymmetricRotation rotation)
{
	myClasification.clear();
	int nClassIndex = 0;

	myClasification.insert( std::pair<DWORD, int>( myValue, ++nClassIndex ) );
	while( next_permutation() )
	{
		bool bFound = false;

		DWORD dwClassToFind = myValue;

		// try to find a class index in the hash-map
		for(int i=0; i<4; i++)
		{
			if( !bFound )
			{
				std::map<DWORD, int>::iterator it = myClasification.find( myValue );
				if( it != myClasification.end() )
				{
					myClasification.insert( std::pair<DWORD, int>( dwClassToFind, it->second ) );
					bFound = true;
				}
			}
			rotate( rotation );
		}

		if( !bFound )
			myClasification.insert( std::pair<DWORD, int>( myValue, ++nClassIndex ) );
	}

	return myClasification.size();
}

int CSymmetricGroup8::GetClassForPermutation(DWORD permutation) const
{
	std::map<DWORD, int>::const_iterator it = myClasification.find( permutation );
	return it != myClasification.end() ? it->second : -1;			
}