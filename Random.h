
#if !defined(RANDOM_H_INCLUDED)
#define RANDOM_H_INCLUDED

#include <iosfwd>

class Random
{
public:
	static int usage(::std::ostream& strm, const char* pMsg);

	Random(size_t argCount, const char*const*const ppArgList);
	int run() const;

	Random(const Random&) = delete;
	Random& operator=(const Random&) = delete;
	Random(Random&&) = delete;
	Random& operator=(Random&&) = delete;

private:
	static int getRandomInteger(int low, int high);

	int			m_lowerBound;
	int			m_upperBound;
	unsigned	m_count;
};

#endif // RANDOM_H_INCLUDED
