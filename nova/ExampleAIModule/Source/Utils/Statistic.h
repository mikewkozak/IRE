#pragma once

#include <cassert>

class Statistic
{
public:

	Statistic();
	Statistic(double val, int count);

	void add(double val);
	void clear();
	int getCount() const;
	void initialise(double val, int count);
	double getTotal() const;
	double getMean() const;
	double getVariance() const;
	double getStdDev() const;
	double getStdErr() const;
	double getMax() const;
	double getMin() const;
	void print(const std::string& name, std::ostream& ostr) const;
	friend std::ostream& operator<< (std::ostream& out, const Statistic& state);

private:

	int count;
	double mean;
	double variance;
	double min, max;
};

inline Statistic::Statistic()
{
	clear();
}

inline Statistic::Statistic(double val, int count)
{
	initialise(val, count);
}

inline void Statistic::add(double val)
{
	double meanOld = mean;
	int countOld = count;
	++count;
	assert(count > 0); // overflow
	mean += (val - mean) / count;
	variance = (countOld * (variance + meanOld * meanOld)
		+ val * val) / count - mean * mean;
	if (val > max)
		max = val;
	if (val < min)
		min = val;
}

inline void Statistic::clear()
{
	count = 0;
	mean = 0;
	variance = 0;
	min = DBL_MAX;
	max = DBL_MIN;
}

inline int Statistic::getCount() const
{
	return count;
}

inline void Statistic::initialise(double val, int count)
{
	count = count;
	mean = val;
}

inline double Statistic::getTotal() const
{
	return mean * count;
}

inline double Statistic::getMean() const
{
	return mean;
}

inline double Statistic::getStdDev() const
{
	return sqrt(variance);
}

inline double Statistic::getStdErr() const
{
	return sqrt(variance / count);
}

inline double Statistic::getMax() const
{
	return max;
}

inline double Statistic::getMin() const
{
	return min;
}

inline void Statistic::print(const std::string& name, std::ostream& ostr) const
{
	ostr << name << ": " << mean << " [" << min << ", " << max << "]" << std::endl;
}

inline std::ostream& operator<< (std::ostream& out, const Statistic& stat)
{
	return out << stat.mean << " [" << stat.min << ", " << stat.max << "]";
};
