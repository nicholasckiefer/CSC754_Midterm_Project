#include <iostream>
#include <queue>
#include <math.h>
#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <tuple>
#include "DICE.h"
#include "Event.h"
#include "Vehicle.h"

float standardDeviation(std::vector<float> vals, float mean) {
	float sum = 0;

	for (float& val : vals) {
		sum += pow((val - mean), 2);
	}

	return float(sqrt((float(1) / float(vals.size())) * float(sum)));
}

float calcMean(std::vector<float> vals) {
	float sum = 0;

	for (float& val : vals) {
		sum += val;
	}

	return float(sum / vals.size());
}

std::tuple<float, float> confidenceInterval(std::vector<float> vals, float z, float mean, float sd) {
	float n = z * (sd / sqrt(vals.size()));

	return std::make_tuple(mean - n, mean + n);
}

int main()
{
	// Simulation Parameters
	int numRuns = 30;
	float waitTimeThreshold = 1;
	int rejectionThreshold = 2;

	// Cumulative Statistical Counters
	float totalTime = 0;
	int totalArrivals = 0;
	int totalVehiclesOverMinWait = 0;
	int totalCarsSeen = 0;
	int totalTrucksSeen = 0;
	int totalCarsServiced = 0;
	int totalTrucksServiced = 0;
	int totalVehiclesTurnedAway = 0;
	float totalCarWashBusyTime = 0;
	float totalTruckWashBusyTime = 0;
	float totalMaxCarWaitTime = 0;
	std::vector<float> totalAvgCarWashUtilizations;
	std::vector<float> totalAvgTruckWashUtilizations;
	std::vector<float> totalAvgCarWaitTimes;
	std::vector<float> totalAvgTruckWaitTimes;
	std::vector<float> totalMinCarWaitTimes;
	std::vector<float> totalMaxCarWaitTimes;
	std::vector<float> totalMinTruckWaitTimes;
	std::vector<float> totalMaxTruckWaitTimes;
	float totalMinTruckWaitTime = 0;
	float totalMaxTruckWaitTime = 0;

	for (int k = 0; k < numRuns; k++) {
		// Local Staistical counters
		float t = 0; // set internal clock for simulation [t is in minutes, so at t = 60 we have reached one hour]
		float carWashBusyTime = 0;
		float truckWashBusyTime = 0;
		int carsServiced = 0;
		int trucksServiced = 0;
		int vehiclesTurnedAway = 0;
		int totalCars = 0;
		int totalTrucks = 0;
		int arrivals = 0;
		int timeSpentWaiting = 0; // Track time customer spent waiting
		int vehiclesOverMinWait = 0;
		float minCarWaitTime = std::numeric_limits<float>::max();
		float maxCarWaitTime = std::numeric_limits<float>::min();
		std::vector<float> carWaitTimes;
		std::vector<float> truckWaitTimes;
		int carsWaitingOverThreshold = 0;
		int truckWaitingOverThreshold = 0;
		float minTruckWaitTime = std::numeric_limits<float>::max();
		float maxTruckWaitTime = std::numeric_limits<float>::min();
		std::vector<Vehicle*> carCustomers;
		std::vector<Vehicle*> truckCustomers;
		int currentSize = 0; // track current largest car queue
		int currQueue = 0; // track what queue add car into
		int counter = 0;

		// State
		std::queue<Vehicle*> IVRQueue; // Store the IVR Vehicles So We Know their Times to Avoid Conflicts
		std::queue<Vehicle*> carQueue; // Store the Cars Being Services So We Know their Times to Avoid Conflicts
		std::queue<Vehicle*> carQueue2; // Store the Cars Being Services So We Know their Times to Avoid Conflicts
		std::queue<Vehicle*> carQueue3; // Store the Cars Being Services So We Know their Times to Avoid Conflicts
		std::list<std::queue<Vehicle*>> carQueueList; // List of all car queues
		std::queue<Vehicle*> truckQueue; // Store the Trucks Being Serviced So We Know Their Times to Avoid Conflicts
		Vehicle* currCustomer;
		float carWashLastBusy = 0;
		float truckWashLastBusy = 0;

		struct Compare {
			bool operator()(Event* lhs, Event* rhs) {
				return lhs->getEventTime() > rhs->getEventTime();
			}
		};

		std::priority_queue<Event*, std::vector<Event*>, Compare> eventsQueue; // Keep Our Event List Sorted For Us

		// Initialization
		float firstArrival = d.rollDice() * 0.333;
		Event* firstEvent = new Event(EventType::ARRIVAL, new Vehicle(firstArrival), firstArrival);

		t = firstEvent->getEventTime();
		eventsQueue.push(firstEvent);

		// Do Not Strand Customers Left in System
		while (t < 480 || !eventsQueue.empty()) // 480 minutes is 8 hours, or 9am to 5pm
		{
			Event* currEvent = NULL;
			// Get Next Event and Remove From Queue
			if (!eventsQueue.empty()) {
				currEvent = eventsQueue.top();
				eventsQueue.pop();
				t = currEvent->getEventTime();
			}
			else {
				throw std::exception("Empty Event Queue");
			}

			if (t > 480) std::cout << "Time over 480: " << IVRQueue.size() + carQueue.size() + carQueue2.size() + carQueue3.size() + truckQueue.size() << std::endl;

			// Set Event Current Customer
			Vehicle* customer = currEvent->getCustomer();

			// Process Event
			switch (currEvent->getEventType()) {
				// Upon Arrival
			case EventType::ARRIVAL:
			{
				arrivals++;
				customer->isCar() ? totalCars++ : totalTrucks++;

				// std::cout << "There are " << IVRQueue.size() << " Vehicles in Line for the IVR, " << truckQueue.size() << " Trucks in Line for a Wash and " << carQueue.size() << " Cars in Line for a Wash" << std::endl;
				// Check Queue Lengths
				if (IVRQueue.size() + carQueue.size() + truckQueue.size() >= rejectionThreshold) {
					std::cout << "Car " << customer->getID() << " is Turning Away" << std::endl;

					// Depart if Over 10 Vehicles Waiting
					Event* nextArrival = customer->depart(t, true);

					vehiclesTurnedAway++;

					// Do Not Schedule the Next Arrival if We Are Past 5pm
					if (nextArrival->getEventTime() < 480) eventsQueue.push(nextArrival);

					break;
				}

				std::cout << t << ": " << customer->getVehicleTypeString() << " " << customer->getID() << " is Arriving" << std::endl;

				// Add to IVR and Generate Event
				std::tuple<Event*, Event*> upcomingEvents = customer->arrive(t, IVRQueue);

				// Update Event Queue
				IVRQueue.push(customer);
				eventsQueue.push(std::get<0>(upcomingEvents)); // IVR Serve Start

				Event* nextArrival = std::get<1>(upcomingEvents);
				std::cout << t << ": Next Arrival is in " << nextArrival->getEventTime() - t << " Minutes" << std::endl;

				// Do Not Schedule the Next Arrival if We Are Past 5pm
				if (nextArrival->getEventTime() < 480) eventsQueue.push(nextArrival);

				break;
			}
			// After the IVR Delay
			case EventType::IVR_SERVE:
			{
				std::cout << t << ": " << customer->getVehicleTypeString() << " " << customer->getID() << " Is Being Done Being Served By the IVR" << std::endl;
				IVRQueue.pop(); // Remove the Vehicle From IVR

				std::queue<Vehicle*>* vehicleQueue;
				if (customer->isCar()) 
				{
					//Find shortest car queue
					// Ensure list is clear
					carQueueList.clear();
					// Add car queues to queue list
					carQueueList.push_front(carQueue3);
					carQueueList.push_front(carQueue2);
					carQueueList.push_front(carQueue);
					//Ensure Trackers are correct
					currentSize = carQueue.size();
					counter = 1;
					currQueue = 1;

					std::list<std::queue<Vehicle*>>::iterator it;
					for (it = carQueueList.begin(); it != carQueueList.end(); it++) {
						std::cout << "Current Queue Size: " << it->size() << std::endl;
						if (it->size() < currentSize)
						{
							currQueue = counter;
							currentSize = it->size();
						}
						counter++;
						std::cout << "Counter: " <<counter << std::endl;
						std::cout << "Current Queue: "<< currQueue << std::endl;

					}

					if (currQueue == 1)
					{
						customer->setQueueNum(1);
						vehicleQueue = &carQueue;
					}
					else if (currQueue == 2)
					{
						customer->setQueueNum(2);
						vehicleQueue = &carQueue2;
					}
					else
					{
						customer->setQueueNum(3);
						vehicleQueue = &carQueue3;
					}
	
				}
				else
				{
					vehicleQueue = &truckQueue;
				}


				std::tuple<Event*, Event*> queueWashEvents = customer->arriveAtQueue(t, *vehicleQueue);

				vehicleQueue->push(customer);

				Event* washStart = std::get<0>(queueWashEvents);
				Event* washEnd = std::get<1>(queueWashEvents);

				eventsQueue.push(washStart);
				eventsQueue.push(washEnd);

				break;
			}
			// After the Queue Delay
			case EventType::VEHICLE_WASH:
			{
				std::cout << t << ": " << customer->getVehicleTypeString() << " " << customer->getID() << " Is Being Washed" << std::endl;

				timeSpentWaiting = t - customer->getID();

				if (customer->isCar()) {
					carWashLastBusy = t;
					if (timeSpentWaiting < minCarWaitTime) minCarWaitTime = timeSpentWaiting;
					if (timeSpentWaiting > maxCarWaitTime) maxCarWaitTime = timeSpentWaiting;
					carWaitTimes.push_back(timeSpentWaiting);
				}
				else {
					if (timeSpentWaiting < minTruckWaitTime) minTruckWaitTime = timeSpentWaiting;
					if (timeSpentWaiting > maxTruckWaitTime) maxTruckWaitTime = timeSpentWaiting;

					truckWashLastBusy = t;
					truckWaitTimes.push_back(timeSpentWaiting);
				}

				customer->arriveAtWasher(t);

				if (timeSpentWaiting > 1)
				{
					vehiclesOverMinWait++;
				}

				break;
			}

			case EventType::DEPARTURE: {
				std::cout << t << ": " << customer->getVehicleTypeString() << " " << customer->getID() << " Is Done and Leaving the System." << std::endl;
				// Finish Washing the Vehicle and They Leave
				customer->isCar() ? carsServiced++ : trucksServiced++;
				customer->depart(t);

				if (customer->isCar()) {
					carWashBusyTime += t - carWashLastBusy;

					if (customer->getQueueNum() == 1)
					{
						carQueue.pop();
					}
					else if (customer->getQueueNum() == 2)
					{
						carQueue2.pop();
					}
					else
					{
						carQueue3.pop();
					}

				}
				else {
					truckWashBusyTime += t - truckWashLastBusy;
					truckQueue.pop();
				}

				break;
			}
			}

			// Go to Next Event
			if (!eventsQueue.empty()) t = eventsQueue.top()->getEventTime();
		}

		std::cout << std::endl;

		float avgCarWait = calcMean(carWaitTimes);
		totalAvgCarWaitTimes.push_back(avgCarWait);

		float avgTruckWait = calcMean(truckWaitTimes);
		totalAvgTruckWaitTimes.push_back(avgTruckWait);

		totalMinCarWaitTimes.push_back(minCarWaitTime);
		totalMaxCarWaitTimes.push_back(maxCarWaitTime);
		totalMinTruckWaitTimes.push_back(minCarWaitTime);
		totalMaxTruckWaitTimes.push_back(maxTruckWaitTime);

		totalTime += t;
		totalArrivals += arrivals;
		totalCarsSeen += totalCars;
		totalTrucksSeen += totalTrucks;
		totalCarsServiced += carsServiced;
		totalTrucksServiced += trucksServiced;
		totalVehiclesTurnedAway += vehiclesTurnedAway;
		totalAvgCarWashUtilizations.push_back(carWashBusyTime / t);
		totalCarWashBusyTime += carWashBusyTime;
		totalTruckWashBusyTime += truckWashBusyTime;
		totalAvgTruckWashUtilizations.push_back(truckWashBusyTime / t);
		totalVehiclesOverMinWait += vehiclesOverMinWait;

		std::cout << "Total Arrivals: " << arrivals << std::endl;
		std::cout << "Total Cars: " << totalCars << std::endl;
		std::cout << "Total Trucks: " << totalTrucks << std::endl;
		std::cout << "Cars Serviced: " << carsServiced << std::endl;
		std::cout << "Trucks Serviced: " << trucksServiced << std::endl;
		std::cout << "Vehicles Turned Away: " << vehiclesTurnedAway << std::endl;
		std::cout << "Average Car Wait Time: " << avgCarWait << " minutes" << std::endl;
		std::cout << "Average Truck Wait Time: " << avgTruckWait << " minutes" << std::endl;
		std::cout << "Percentage of Vehicles with Wait Time > " << waitTimeThreshold << " Minutes: " << ((float)(vehiclesOverMinWait) / (float)(totalCars + totalTrucks)) * 100 << " % " << std::endl;
	}

	std::cout << std::endl;
	std::cout << "RESULTS" << std::endl;
	std::cout << "######################################" << std::endl;

	float totalAvgCarWait = calcMean(totalAvgCarWaitTimes);
	float totalAvgTruckWait = calcMean(totalAvgTruckWaitTimes);

	float totalAvgMinCarWait = calcMean(totalMinCarWaitTimes);
	float totalAvgMaxCarWait = calcMean(totalMaxCarWaitTimes);
	float totalAvgMinTruckWait = calcMean(totalMinTruckWaitTimes);
	float totalAvgMaxTruckWait = calcMean(totalMaxTruckWaitTimes);

	float totalAvgCarWashUtilization = calcMean(totalAvgCarWashUtilizations);
	float totalAvgTruckWashUtilization = calcMean(totalAvgTruckWashUtilizations);

	float carWaitDeviation = standardDeviation(totalAvgCarWaitTimes, totalAvgCarWait);
	float truckWaitDeviation = standardDeviation(totalAvgTruckWaitTimes, totalAvgTruckWait);

	float carMinWaitDeviation = standardDeviation(totalMinCarWaitTimes, totalAvgMinCarWait);
	float carMaxWaitDeviation = standardDeviation(totalMaxCarWaitTimes, totalAvgMaxCarWait);
	float truckMinWaitDeviation = standardDeviation(totalMinTruckWaitTimes, totalAvgMinTruckWait);
	float truckMaxWaitDeviation = standardDeviation(totalMaxTruckWaitTimes, totalAvgMaxTruckWait);

	float carUtilizationDeviation = standardDeviation(totalAvgCarWashUtilizations, totalAvgCarWashUtilization);
	float truckUtilizationDeviation = standardDeviation(totalAvgTruckWashUtilizations, totalAvgTruckWashUtilization);

	std::tuple<float, float> carWaitConfidence = confidenceInterval(totalAvgCarWaitTimes, 1.960, totalAvgCarWait, carWaitDeviation);
	std::tuple<float, float> truckWaitConfidence = confidenceInterval(totalAvgTruckWaitTimes, 1.960, totalAvgTruckWait, truckWaitDeviation);

	std::tuple<float, float> carMinWaitConfidence = confidenceInterval(totalMinCarWaitTimes, 1.960, totalAvgMinCarWait, carMinWaitDeviation);
	std::tuple<float, float> carMaxWaitConfidence = confidenceInterval(totalMaxCarWaitTimes, 1.960, totalAvgMaxCarWait, carMaxWaitDeviation);
	std::tuple<float, float> truckMinWaitConfidence = confidenceInterval(totalMinTruckWaitTimes, 1.960, totalAvgMinTruckWait, truckMinWaitDeviation);
	std::tuple<float, float> truckMaxWaitConfidence = confidenceInterval(totalMaxTruckWaitTimes, 1.960, totalAvgMaxTruckWait, truckMaxWaitDeviation);

	std::tuple<float, float> carUtilizationConfidence = confidenceInterval(totalAvgCarWashUtilizations, 1.960, totalAvgCarWashUtilization, carUtilizationDeviation);
	std::tuple<float, float> truckUtilizationConfidence = confidenceInterval(totalAvgTruckWashUtilizations, 1.960, totalAvgTruckWashUtilization, truckUtilizationDeviation);

	std::cout << "Overall Time: " << totalTime << " minutes" << std::endl;
	std::cout << "Average Time: " << float(totalTime / numRuns) << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "Overall Total Vehicles: " << totalArrivals << " vehicles" << std::endl;
	std::cout << "Overall Total Cars: " << totalCarsSeen << " cars" << std::endl;
	std::cout << "Overall Total Trucks: " << totalTrucksSeen << " trucks" << std::endl;
	std::cout << std::endl;

	std::cout << "Overall Cars Serviced: " << totalCarsServiced << " cars" << std::endl;
	std::cout << "Overall Trucks Serviced: " << totalTrucksServiced << " trucks" << std::endl;
	std::cout << std::endl;

	std::cout << "Overall Vehicles Turned Away: " << totalVehiclesTurnedAway << " vehicles" << std::endl;
	std::cout << "Percentage Vehicles Turned Away: " << float(totalVehiclesTurnedAway / float(totalCarsSeen + totalTrucksSeen)) * 100 << "%" << std::endl;
	std::cout << std::endl;

	std::cout << "Overall CarWash Busy Time: " << totalCarWashBusyTime << " minutes" << std::endl;
	std::cout << "Average CarWash Busy Time: " << float(totalCarWashBusyTime / numRuns) << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "Average CarWash Utilization: " << float(totalCarWashBusyTime / totalTime) * 100 << "%" << std::endl;
	std::cout << std::endl;

	std::cout << "Overall TruckWash Busy Time: " << totalTruckWashBusyTime << " minutes" << std::endl;
	std::cout << "Average TruckWash Busy Time: " << float(totalTruckWashBusyTime / numRuns) << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "Average TruckWash Utilization : " << float(totalTruckWashBusyTime / totalTime) * 100 << "%" << std::endl;
	std::cout << std::endl;

	std::cout << "Average Car Wait Time: " << totalAvgCarWait << " minutes" << std::endl;
	std::cout << "Average Truck Wait Time: " << totalAvgTruckWait << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "Percentage of Vehicles with Wait Time > " << waitTimeThreshold << " Minutes: " << ((float)(totalVehiclesOverMinWait) / (float)(totalCarsSeen + totalTrucksSeen)) * 100 << "%" << std::endl;
	std::cout << std::endl;

	std::cout << "CarWash Utilization Standard Deviation: " << carUtilizationDeviation * 100 << "%" << std::endl;
	std::cout << "TruckWash Utilization Standard Deviation: " << truckUtilizationDeviation * 100 << "%" << std::endl;
	std::cout << std::endl;

	std::cout << "Car Wait Time Standard Deviation: " << carWaitDeviation << " minutes" << std::endl;
	std::cout << "Car Min Wait Time Standard Deviation: " << carMinWaitDeviation << " minutes" << std::endl;
	std::cout << "Car Max Wait Time Standard Deviation: " << carMaxWaitDeviation << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "Truck Wait Time Standard Deviation: " << truckWaitDeviation << " minutes" << std::endl;
	std::cout << "Truck Min Wait Time Standard Deviation: " << truckMinWaitDeviation << " minutes" << std::endl;
	std::cout << "Truck Max Wait Time Standard Deviation: " << truckMaxWaitDeviation << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "CarWash Utilization Confidence Interval: " << std::get<0>(carUtilizationConfidence) * 100 << " - " << std::get<1>(carUtilizationConfidence) * 100 << "%" << std::endl;
	std::cout << "TruckWash Utilization Confidence Interval: " << std::get<0>(truckUtilizationConfidence) * 100 << " - " << std::get<1>(truckUtilizationConfidence) * 100 << "%" << std::endl;
	std::cout << std::endl;

	std::cout << "Car Wait Time Confidence Interval: " << std::get<0>(carWaitConfidence) << " - " << std::get<1>(carWaitConfidence) << " minutes" << std::endl;
	std::cout << "Car Min Wait Time Confidence Interval: " << std::get<0>(carMinWaitConfidence) << " - " << std::get<1>(carMinWaitConfidence) << " minutes" << std::endl;
	std::cout << "Car Max Wait Time Confidence Interval: " << std::get<0>(carMaxWaitConfidence) << " - " << std::get<1>(carMaxWaitConfidence) << " minutes" << std::endl;
	std::cout << std::endl;

	std::cout << "Truck Wait Time Confidence Interval: " << std::get<0>(truckWaitConfidence) << " - " << std::get<1>(truckWaitConfidence) << " minutes" << std::endl;
	std::cout << "Truck Min Wait Time Confidence Interval: " << std::get<0>(truckMinWaitConfidence) << " - " << std::get<1>(truckMinWaitConfidence) << " minutes" << std::endl;
	std::cout << "Truck Max Wait Time Confidence Interval: " << std::get<0>(truckMaxWaitConfidence) << " - " << std::get<1>(truckMaxWaitConfidence) << " minutes" << std::endl;

	std::system("pause");
	return 0;
}
