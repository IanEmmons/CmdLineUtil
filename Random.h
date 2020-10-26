
#if !defined(RANDOM_H_INCLUDED)
#define RANDOM_H_INCLUDED

#include <gsl/gsl>
#include <iosfwd>

class Random
{
public:
	static int usage(::std::ostream& strm, const ::std::string& progName,
		const char* pMsg);

	Random(::gsl::span<const char*const> args);
	int run() const;

	Random(const Random&) = delete;
	Random& operator=(const Random&) = delete;
	Random(Random&&) = delete;
	Random& operator=(Random&&) = delete;

private:
	static long getRandomInteger(long low, long high);

	long	m_lowerBound;
	long	m_upperBound;
	long	m_count;
};

#endif // RANDOM_H_INCLUDED
