#pragma once
class Event;
enum EventType;
class Vehicle;
enum VehicleType;
enum VechicleStatus;

enum EventType {
    ARRIVAL, IVR_ARRIVE, IVR_SERVE, QUEUE_ARRIVE, VEHICLE_WASH, DEPARTURE
};

class Event {

private:
    Vehicle* customer;
    float eventTime;
    EventType eventType;

public:
    Event(EventType eventType, Vehicle* customer, float eventTime) {
        this->customer = customer;
        this->eventType = eventType;
        this->eventTime = eventTime;
    }

    EventType getEventType() {
        return this->eventType;
    }

    Vehicle* getCustomer() {
        return this->customer;
    }

    float getEventTime() {
        return this->eventTime;
    }
};