#include <iostream>
#include <vector>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include "ThreadPool.h"

int main()
{
	std::cout << "ThreadPool Start!\n";
	std::vector<std::future<int> > results;

	//加上作用域后，可以保证主线程在子线程运行结束后，才继续执行
	//因为ThreadPool的析构函数中，会阻塞等待回收子线程资源
	{
		ThreadPool threadPool(4);

		for (int i = 0; i < 8; i++) {
			//初始化子线程，关联输出
			results.emplace_back(
				threadPool.equeue([i] {
					//由于是多线程，所以在输出上，先后次序不能得到保障，杂乱无章
					//用printf时，能在单行输出上保持一致性
					//而cout两部分时，会互相干扰，不能保证单行的一致性

					//std::cout << "hello " << i << std::endl;
					printf("hello %d \n", i);
					std::this_thread::sleep_for(std::chrono::seconds(2));
					//std::cout << "world  " << i << std::endl;
					printf("world %d \n", i);
					return i * i;
					})
			);
		}

	}

	for (auto&& result : results) {
		std::cout << result.get() <<  ' ';
	}
	

	//std::cout << "\nThreadPool End!\n";

	return 0;
}

