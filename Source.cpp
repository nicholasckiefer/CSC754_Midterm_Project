#include <iostream>
#include <queue>
#include <math.h> 
#include<string>
#include<cstring>
#include "DICE.h"
#include "Vehicle.h"
using namespace std;


int main()
{
	// Global Statistic Variables
	

	// Invoke Initialization Routine
	int t = 0; // set internal clock for simulation [t is in minutes, so at t = 60 we have reached one hour]

	// Initialize System State and staistical counters
	int carsServiced = 0;
	int trucksServiced = 0;
	int vehiclesTurnedAway = 0;
	int totalCars = 0;
	int totalTrucks = 0;

	// Other Variables
	DICE d6;
	queue<Vehicle> carQueue;
	queue<Vehicle> truckQueue;
	queue<Vehicle> IVR;

	bool incomingVehicle = false; // Will track if simulation currently has a vehicle coming, and if not will roll dice to see how long before next arriving vehicle
	double nextEvent; // Tracks when next vehicle tries to enter the queue

	Vehicle newVehicle; // Used to add new vehicles to queue
	Vehicle transferVehicle; // Used to add new vehicles to either car or truck queue


	// Simulation Start | Condition = Empty and Idle
	


	// Advance simulation clock
	for (t = 0; t < 480; t++) // 480 minutes is 8 hours, or 9am to 5pm
	{
		if (incomingVehicle == false) // No vehicle is currently in route to the queue
		{
			nextEvent = ceil(t + (d6.rollDice() * 0.333));
			incomingVehicle = true;
			newVehicle.setID(t); // Sets vehicle ID to time it was created
			newVehicle.randomVehicleType(); // Vehicle generates its vehicle type
			newVehicle.setArrivalTime(nextEvent); // Sets vehicles arrival time
			cout << newVehicle.getVType() << endl;
			if (newVehicle.getVType() == "Car")
			{
				totalCars++;
			}
			else
			{
				totalTrucks++;
			}
		}

		if (incomingVehicle == true) // A vehicle is on route to queue, car wash runs normally
		{
			if (t == nextEvent) // A car has arrived at the car wash
			{
				incomingVehicle = false;
				if (carQueue.size() + truckQueue.size() <= 10) // If queue in both lines is <= 10 we can add another vehicle to IVR 
				{
					IVR.push(Vehicle(newVehicle.getID(), newVehicle.getVType(), newVehicle.getArrivalTime())); // Add arriving vehicle to IVR queue to be processed and added to car or truck queue
				}
				else // queue size is > 10
				{
					vehiclesTurnedAway++;
				}
			}

			if (!IVR.empty()) // If cars are waiting in IVR queue
			{
				if (IVR.front().getEnterQueueTime() == -1) // If vehicle has just got to front of IVR queue we must determine how long it takes to process them at queue 
				{
					IVR.front().setEnterQueueTime(ceil(t + (d6.rollDice() * 0.3))); // Account for delay at IVR unit
				}

				else if (IVR.front().getEnterQueueTime() == t) // If vehicle has spent approprate amount of time at IVR station it moves to approporate queue
				{
					if (IVR.front().getVType() == "Car")
					{
						transferVehicle = IVR.front();
						carQueue.push(transferVehicle);
						IVR.pop();
					}
					else
					{
						transferVehicle = IVR.front();
						truckQueue.push(transferVehicle);
						IVR.pop();
					}
				}
			}


			if (!carQueue.empty()) // If there are cars in the car queue
			{
				if (carQueue.front().getServiceTime() == -1)
				{
					carQueue.front().setServiceTime(ceil(t + (d6.rollDice()))); // Set time when car will be serviced 
				}

				if (carQueue.front().getServiceTime() == t) // If car has been serviced
				{
					carQueue.pop();
					carsServiced++;
				}
			}


			if (!truckQueue.empty()) // If there are trucks in the truck queue
			{
				if (truckQueue.front().getServiceTime() == -1)
				{
					truckQueue.front().setServiceTime(ceil(t + (d6.rollDice() * 2))); // Set time when truck will be serviced 
				}

				if (truckQueue.front().getServiceTime() == t) // If truck has been serviced
				{
					truckQueue.pop();
					trucksServiced++;
				}
			}
			

		}

	}

	cout << "Total Cars: " << totalCars << endl;
	cout << "Total Trucks: " << totalTrucks << endl;
	cout << "Cars Serviced: " << carsServiced << endl;
	cout << "Trucks Serviced: " << trucksServiced << endl;
	cout << "Vehicles Turned Away: " << vehiclesTurnedAway << endl;


	system("pause");
	return 0;
}