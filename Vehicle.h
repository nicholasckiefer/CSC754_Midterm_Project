#include <stdio.h>     
#include <stdlib.h>     
#include <time.h>
#include <queue>
#include <vector>
#include <tuple>
#include "Event.h"
#include "DICE.h"
#pragma once

class Event;
class Vehicle;
enum EventType;
enum VehicleType;

DICE d;

enum VehicleStatus { IVR_WAITING, QUEUED_WAITING, SERVING, DEPARTED };

enum VehicleType { CAR, TRUCK };

class Vehicle
{

private:
	float id;
	VehicleType vehicleType;
	std::vector<Event*> events;
	VehicleStatus status;

public:
	// Constructor
	Vehicle(float id)
	{
		// srand((unsigned int)time(NULL));
		this->id = id;
		this->vehicleType = this->getRandomVehicleType();
	}

	// Accessor Functions
	float getID()
	{
		return id;
	}

	float getNextEvent() {
		return this->events.back()->getEventTime();
	}

	// Vehicle Arrives in System, Goes to IVR
	std::tuple<Event*, Event*> arrive(float arrivalTime, std::queue<Vehicle*>& IVRQueue) {
		this->events.push_back(new Event(EventType::ARRIVAL, this, arrivalTime));
		this->status = VehicleStatus::IVR_WAITING;

		// Return Time for Next Event
		return std::make_tuple(generateIVRFinishTime(arrivalTime, IVRQueue), generateNextArrival(arrivalTime));
	}

	// Vehicle Departs IVR, Waits in Respective Queue for Wash
	std::tuple<Event*, Event*> arriveAtQueue(float arrivalTime, std::queue<Vehicle*>& vehicleQueue) {
		this->status = VehicleStatus::QUEUED_WAITING;

		return generateWashTimes(arrivalTime, vehicleQueue);
	}

	// Vehicle Arrives at Washing Station
	void arriveAtWasher(float arrivalTime) {
		this->status = VehicleStatus::SERVING;
	}

	// Vehicle Finishes Washing, Departs System
	Event* depart(float departureTime, bool premature = false) {
		if (premature) return generateNextArrival(departureTime);

		this->status = VehicleStatus::DEPARTED;

		return NULL;
	}

	// Upon Vehicle A's Arrival, We Generate Vehicle B's Arrival Time
	Event* generateNextArrival(float t) {
		// Do Not Push this Event Since it is for the Next Arriving Vehicle and Will
		// Be Pushed to that Vehicle's Event vector in arrive()
		float roll = d.rollDice();
		float arrivalTime = t + (roll * 0.333);
		Event* nextEvent = new Event(EventType::ARRIVAL, new Vehicle(arrivalTime), arrivalTime);

		return nextEvent;
	}

	// Check the IVR Queue and Schedule the Current Vehicle to Be Served Next
	Event* generateIVRFinishTime(float t, std::queue<Vehicle*>& IVRQueue) {
		int roll = d.rollDice();
		Event* nextEvent;

		if (IVRQueue.empty()) {
			nextEvent = new Event(EventType::IVR_SERVE, this, t + roll * 0.3);
			this->events.push_back(nextEvent);

			return nextEvent;
		}

		Vehicle* nextVehicle = IVRQueue.back();

		// Use this to Enforce Queue Order
		nextEvent = new Event(EventType::IVR_SERVE, this, nextVehicle->getNextEvent() + (roll * 0.3));

		this->events.push_back(nextEvent);

		return nextEvent;
	}

	// Once the Vehicle is in the Wash Queue, Schedule the Time it Should Get Serviced Based on Queue
	std::tuple<Event*, Event*> generateWashTimes(float t, std::queue<Vehicle*>& vehicleQueue) {
		Event* nextEvent;
		Event* departureEvent;

		float roll = d.rollDice();
		float tt = this->isCar() ? roll : (roll * 2);

		if (vehicleQueue.empty()) {
			nextEvent = new Event(EventType::VEHICLE_WASH, this, t); // If No One in Front, Get Served Immediately
			departureEvent = new Event(EventType::DEPARTURE, this, t + tt);

			this->events.push_back(nextEvent);
			this->events.push_back(departureEvent);

			return std::make_tuple(nextEvent, departureEvent);
		}

		Vehicle* nextVehicle = vehicleQueue.back();
		if (nextVehicle->events.back()->getEventType() != EventType::DEPARTURE) throw new std::exception("WE FUCKED");

		//std::cout << "Next Car Departing? " << yn << std::endl;

		// Use this to Enforce Queue Order
		// Wait for Person in Front to Leave and Be Ready to Wash
		nextEvent = new Event(EventType::VEHICLE_WASH, this, nextVehicle->getNextEvent() + 0.01);
		departureEvent = new Event(EventType::DEPARTURE, this, nextVehicle->getNextEvent() + 0.01 + tt);

		this->events.push_back(nextEvent);
		this->events.push_back(departureEvent);

		return std::make_tuple(nextEvent, departureEvent);
	}

	VehicleType getVehicleType() {
		return vehicleType;
	}

	bool isCar() {
		return vehicleType == VehicleType::CAR;
	}

	bool isTruck() {
		return vehicleType == VehicleType::TRUCK;
	}

	std::string getVehicleTypeString() {
		return isCar() ? "Car" : "Truck";
	}

	VehicleType getRandomVehicleType() {
		float chance;
		chance = (rand() / (float)RAND_MAX) * 100; // Range [0-100]

		if (chance <= 16.7)
		{
			return VehicleType::TRUCK;
		}
		else
		{
			return VehicleType::CAR;
		}
	}
};

