#include "torus.hpp"

int main()
{
  Torus torus(1.0, 0.2);
  Data_cloud cloud;
  size_t N = 10000;
  generate(torus, N, cloud);

  for (size_t i = 0; i < cloud.size(); ++i)
    {
      const uvector &uv = cloud[i];

      std::cout << uv(0) << " "
		<< uv(1) << " "
		<< uv(2) << "\n";
    }
}
