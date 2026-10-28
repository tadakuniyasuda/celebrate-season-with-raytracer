#include <iostream>
#include <vector>
#include <cstdlib>	// std::abs


int main()
{
	const int radius = 3;
	std::vector<float> w(radius + 1);
	for (int d = 0; d <= radius; d++)
	{
		w[d] = 1.0f / (d + 1);
	}

	for (int d = -radius; d <= radius; d++)
	{
		std::cout << "d= " << d << " w=" << w[std::abs(d)] << "\n";

	}

	return 0;
}
