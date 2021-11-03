#include<string>
#include<cstring>
#include <stdio.h>     
#include <stdlib.h>     
#include <time.h>
#include "DICE.h"
#pragma once
class Vehicle
{

private:
	int id;
	std::string vType;
	int arrivalTime;
	int enterQueueTime;
	int serviceTime;


public:


	// Default Constructor
	Vehicle()
	{
		srand((unsigned int)time(NULL));
		id = -1;
		vType = getRandomVehicleType();
		arrivalTime = -1;
		enterQueueTime = -1;
		serviceTime = -1;
	}

	//Other Constructors
	Vehicle(int newId)
	{
		srand((unsigned int)time(NULL));
		id = newId;
		vType = getRandomVehicleType();
		arrivalTime = -1;
		enterQueueTime = -1;
		serviceTime = -1;
	}

	Vehicle(int newId, std::string newVType)
	{
		srand((unsigned int)time(NULL));
		id = newId;
		vType = newVType;
		arrivalTime = -1;
		enterQueueTime = -1;
		serviceTime = -1;
	}

	Vehicle(int newId, std::string newVType, int newArrivalTime)
	{
		srand((unsigned int)time(NULL));
		id = newId;
		vType = newVType;
		arrivalTime = newArrivalTime;
		enterQueueTime = -1;
		serviceTime = -1;
	}

	Vehicle(int newId, std::string newVType, int newArrivalTime, int newEnterQueueTime)
	{
		srand((unsigned int)time(NULL));
		id = newId;
		vType = newVType;
		arrivalTime = newArrivalTime;
		enterQueueTime = newEnterQueueTime;
		serviceTime = -1;
	}



	// Accessor Functions
	int getID()
	{
		return id;
	}

	std::string getVType()
	{
		return vType;
	}

	int getArrivalTime()
	{
		return arrivalTime;
	}

	int getEnterQueueTime()
	{
		return enterQueueTime;
	}

	int getServiceTime()
	{
		return serviceTime;
	}

	// Mutator Functions
	void setID(int newId)
	{
		id = newId;
	}

	void setVType(std::string newVType)
	{
		vType = newVType;
	}

	void setArrivalTime(int newArrivalTime)
	{
		arrivalTime = newArrivalTime;
	}

	void setEnterQueueTime(int newEnterQueueTime)
	{
		enterQueueTime = newEnterQueueTime;
	}

	void setServiceTime(int newServiceTime)
	{
		serviceTime = newServiceTime;
	}

	// Other Functions
	void randomArrivalTime()
	{

	}

	void randomVehicleType()
	{
		float chance;
		chance = (rand() / (float)RAND_MAX) * 100; // Range [0-100]
		if (chance <= 16.7)
		{
			vType = "Truck";
		}
		else
		{
			vType = "Car";
		}
	}

	std::string getRandomVehicleType()
	{
		float chance;
		chance = (rand() / (float)RAND_MAX) * 100; // Range [0-100]
		if (chance <= 16.7)
		{
			return "Truck";
		}
		else
		{
			return "Car";
		}
	}


};

