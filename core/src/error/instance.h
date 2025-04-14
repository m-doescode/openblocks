#pragma once

#include "error.h"

class NoSuchInstance : public Error {
    public:
    inline NoSuchInstance(std::string className) : Error("NoSuchInstance", "Cannot create instance of unknown type " + className) {}
};

class NoSuchService : public Error {
    public:
    inline NoSuchService(std::string className) : Error("NoSuchService", "Cannot insert service of unknown type " + className) {}
};

class ServiceAlreadyExists : public Error {
    public:
    inline ServiceAlreadyExists(std::string className) : Error("ServiceAlreadyExists", "Service " + className + " is already inserted") {}
};