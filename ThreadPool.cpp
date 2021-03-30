#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int main()
{
	std::cout << "ThreadPool Start!\n";
	ThreadPool threadPool(4);

	std::vector<std::future<int> > results;

	for (int i = 0; i < 8; i++) {
		//初始化子线程，关联输出
		results.emplace_back(
			threadPool.equeue([i] {
				
				//std::cout << "hello " << i << std::endl;
				//if( i==3 )
					std::this_thread::sleep_for(std::chrono::seconds(5));
				//std::cout << "world " << i << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));

				return i * i;
			})
		);
	}
	
	for (auto&& result : results) {
		std::cout << result.get() <<  ' ';
	}
	

	//std::cout << "\nThreadPool End!\n";

	return 0;
}

