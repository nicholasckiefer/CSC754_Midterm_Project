#pragma once

struct QueueCompare {
	bool operator()(std::queue<Vehicle*>* lhs, std::queue<Vehicle*>* rhs) {
		return lhs->size() > rhs->size();
	}
};

class VehicleQueue {
private:
	int numQueues;
	int totalSize;
	std::vector<std::queue<Vehicle*>*> vehicleQueues;
	std::priority_queue<std::queue<Vehicle*>*, std::vector<std::queue<Vehicle*>*>, QueueCompare> vehicleQueueList;

public:
	VehicleQueue(int numQueues) {
		this->numQueues = numQueues;
		this->totalSize = 0;

		for (int i = 0; i < numQueues; i++) {
			std::queue<Vehicle*>* vehicleQueue = new std::queue<Vehicle*>;
			vehicleQueues.push_back(vehicleQueue);
		}

		for (std::queue<Vehicle*>* vehicleQueue : vehicleQueues) {
			vehicleQueueList.push(vehicleQueue);
		}
	}

	int size() {
		return this->totalSize;
	}

	void shrink() {
		this->totalSize = this->totalSize - 1;
	}

	std::queue<Vehicle*>* push(Vehicle* vehicle) {
		std::queue<Vehicle*>* topQueue = vehicleQueueList.top();
		topQueue->push(vehicle);

		this->sort();
		this->totalSize = this->totalSize + 1;

		return topQueue;
	}

	std::queue<Vehicle*>* top() {
		return vehicleQueueList.top();
	}

	void sort() {
		while (!vehicleQueueList.empty()) {
			vehicleQueueList.pop();
		}

		for (std::queue<Vehicle*>* vehicleQueue : vehicleQueues) {
			vehicleQueueList.push(vehicleQueue);
		}
	}
};