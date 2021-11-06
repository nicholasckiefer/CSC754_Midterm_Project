#include <iostream>
#include <queue>
#include <math.h>
#include <string>
#include <cstring>
#include <vector>
#include <tuple>
#include "DICE.h"
#include "Event.h"
#include "Vehicle.h"

int main()
{
	int numRuns = 30;
	float totalTime = 0;
	int totalCarsSeen = 0;
	int totalTrucksSeen = 0;
	int totalCarsServiced = 0;
	int totalTrucksServiced = 0;
	int totalVehiclesTurnedAway = 0;
	float totalCarWashBusyTime = 0;
	float totalTruckWashBusyTime = 0;

	for (int k = 0; k < numRuns; k++) {
		// Staistical counters
		float t = 0; // set internal clock for simulation [t is in minutes, so at t = 60 we have reached one hour]
		float carWashBusyTime = 0;
		float truckWashBusyTime = 0;
		int carsServiced = 0;
		int trucksServiced = 0;
		int vehiclesTurnedAway = 0;
		int totalCars = 0;
		int totalTrucks = 0;
		int arrivals = 0;

		std::vector<Vehicle*> carsOrder;
		std::vector<Vehicle*> trucksOrder;

		// State
		std::queue<Vehicle*> IVRQueue; // Store the IVR Vehicles So We Know their Times to Avoid Conflicts
		std::queue<Vehicle*> carQueue; // Store the Cars Being Services So We Know their Times to Avoid Conflicts
		std::queue<Vehicle*> truckQueue; // Store the Trucks Being Serviced So We Know Their Times to Avoid Conflicts
		Vehicle* currCustomer;
		float carWashLastBusy = 0;
		float truckWashLastBusy = 0;

		DICE d6;

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

			if (currEvent->getEventType() == EventType::ARRIVAL) {
				arrivals++;
			}

			if (t > 480) std::cout << "Time over 480: " << IVRQueue.size() + carQueue.size() + truckQueue.size() << std::endl;

			// Set Event Current Customer
			Vehicle* customer = currEvent->getCustomer();

			// Process Event
			switch (currEvent->getEventType()) {
				// Upon Arrival
			case EventType::ARRIVAL:
			{
				customer->isCar() ? totalCars++ : totalTrucks++;

				// std::cout << "There are " << IVRQueue.size() << " Vehicles in Line for the IVR, " << truckQueue.size() << " Trucks in Line for a Wash and " << carQueue.size() << " Cars in Line for a Wash" << std::endl;
				// Check Queue Lengths
				if (IVRQueue.size() + carQueue.size() + truckQueue.size() >= 10) {
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

				std::queue<Vehicle*>* vehicleQueue = customer->isCar() ? &carQueue : &truckQueue;

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
				
				if (customer->isCar()) {
					carWashLastBusy = t;
				}
				else {
					truckWashLastBusy = t;
				}

				customer->arriveAtWasher(t);

				break;
			}

			case EventType::DEPARTURE: {
				std::cout << t << ": " << customer->getVehicleTypeString() << " " << customer->getID() << " Is Done and Leaving the System." << std::endl;
				// Finish Washing the Vehicle and They Leave
				customer->isCar() ? carsServiced++ : trucksServiced++;
				customer->depart(t);

				if (customer->isCar()) {
					carWashBusyTime += t - carWashLastBusy;
				}
				else {
					truckWashBusyTime += t - truckWashLastBusy;
				}

				if (customer->isCar()) {
					carsOrder.push_back(carQueue.front());
					carQueue.pop();
				}
				else {
					trucksOrder.push_back(truckQueue.front());
					truckQueue.pop();
				}

				break;
			}
			}

			// Go to Next Event
			if (!eventsQueue.empty()) t = eventsQueue.top()->getEventTime();
		}

		std::cout << std::endl;

		totalTime += t;
		totalCarsSeen += totalCars;
		totalTrucksSeen += totalTrucks;
		totalCarsServiced += carsServiced;
		totalTrucksServiced += trucksServiced;
		totalVehiclesTurnedAway += vehiclesTurnedAway;
		totalCarWashBusyTime += carWashBusyTime;
		totalTruckWashBusyTime += truckWashBusyTime;

		std::cout << "Total Arrivals: " << arrivals << std::endl;
		std::cout << "Total Cars: " << totalCars << std::endl;
		std::cout << "Total Trucks: " << totalTrucks << std::endl;
		std::cout << "Cars Serviced: " << carsServiced << std::endl;
		std::cout << "Trucks Serviced: " << trucksServiced << std::endl;
		std::cout << "Vehicles Turned Away: " << vehiclesTurnedAway << std::endl;
	}

	std::cout << std::endl;

	std::cout << "Overall Time: " << totalTime << std::endl;
	std::cout << "Overall Total Cars: " << totalCarsSeen << std::endl;
	std::cout << "Overall Total Trucks: " << totalTrucksSeen << std::endl;
	std::cout << "Overall Cars Serviced: " << totalCarsServiced << std::endl;
	std::cout << "Overall Trucks Serviced: " << totalTrucksServiced << std::endl;
	std::cout << "Overall Vehicles Turned Away: " << totalVehiclesTurnedAway << std::endl;
	std::cout << "Overall CarWash Busy Time: " << totalCarWashBusyTime << std::endl;
	std::cout << "Overall CarWash Utilization: " << float(totalCarWashBusyTime / totalTime) << std::endl;
	std::cout << "Overall TruckWash Busy Time: " << totalTruckWashBusyTime << std::endl;
	std::cout << "Overall TruckWash Utilization: " << float(totalTruckWashBusyTime / totalTime) << std::endl;

	std::system("pause");
	return 0;
}