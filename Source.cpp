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
	// Global Statistic Variables

	// Initialize System State and staistical counters
	int t = 0; // set internal clock for simulation [t is in minutes, so at t = 60 we have reached one hour]
	int carsServiced = 0;
	int trucksServiced = 0;
	int vehiclesTurnedAway = 0;
	int totalCars = 0;
	int totalTrucks = 0;
	int arrivals = 0;
	std::queue<Vehicle*> IVRQueue; // Store the IVR Vehicles So We Know their Times to Avoid Conflicts
	std::queue<Vehicle*> carQueue; // Store the Cars Being Services So We Know their Times to Avoid Conflicts
	std::queue<Vehicle*> truckQueue; // Store the Trucks Being Serviced SO We Know Their Times to Avoid Conflicts
	Vehicle* currCustomer;
	
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
		Event* currEvent= NULL;
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

		// Set Event Current Customer
		Vehicle* customer = currEvent->getCustomer();

		// Process Event
		switch (currEvent->getEventType()) {
			// Upon Arrival
			case EventType::ARRIVAL:
			{
				std::cout << t << ": Car " << customer->getID() << " is Arriving" << std::endl;

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

				// Update IVR Queue
				IVRQueue.push(customer);				

				// Add to IVR and Generate Event
				std::tuple<Event*, Event*> upcomingEvents = customer->arrive(t, IVRQueue);

				// Update Event Queue
				eventsQueue.push(std::get<0>(upcomingEvents)); // IVR Serve Start

				Event* nextArrival = std::get<1>(upcomingEvents);
				// Do Not Schedule the Next Arrival if We Are Past 5pm
				if (nextArrival->getEventTime() < 480) eventsQueue.push(nextArrival);
				
				break;
			}
			// After the IVR Delay
			case EventType::IVR_SERVE:
			{
				// std::cout << t << ": Car " << customer->getID() << " Is Being Done Being Served By the IVR" << std::endl;
				IVRQueue.pop(); // Remove the Vehicle From IVR

				std::queue<Vehicle*>* vehicleQueue = customer->isCar() ? &carQueue : &truckQueue;

				vehicleQueue->push(customer);

				Event* queueArriveEvent = customer->arriveAtQueue(t, *vehicleQueue);

				eventsQueue.push(queueArriveEvent);

				break;
			}
			// After the Queue Delay
			case EventType::VEHICLE_WASH:
			{
				// std::cout << t << ": Car " << customer->getID() << " Is Being Washed" << std::endl;
				// Begin Washing the Vehicle and Generate Departure Event
				if (customer->isCar()) {
					carQueue.pop();
				}
				else {
					truckQueue.pop();
				}

				Event* departureEvent = customer->arriveAtWasher(t);

				eventsQueue.push(departureEvent);

				break;
			}

			case EventType::DEPARTURE: {
				std::cout << t << ": Car " << customer->getID() << " Is Done and Leaving the System." << std::endl;
				// Finish Washing the Vehicle and They Leave
				customer->isCar() ? carsServiced++ : trucksServiced++;
				customer->depart(t);

				break;
			}
		}

		// Go to Next Event
		if (!eventsQueue.empty()) t = eventsQueue.top()->getEventTime();
	}

	std::cout << "Total Arrivals: " << arrivals << std::endl;
	std::cout << "Total Cars: " << totalCars << std::endl;
	std::cout << "Total Trucks: " << totalTrucks << std::endl;
	std::cout << "Cars Serviced: " << carsServiced << std::endl;
	std::cout << "Trucks Serviced: " << trucksServiced << std::endl;
	std::cout << "Vehicles Turned Away: " << vehiclesTurnedAway << std::endl;

	std::system("pause");
	return 0;
}