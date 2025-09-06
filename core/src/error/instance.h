#pragma once

#include "error.h"

class NoSuchInstance : public Error {
    public:
    inline NoSuchInstance(std::string className) : Error("NoSuchInstance", "Cannot create instance of unknown type " + className) {}
};

class NoSuchService : public Error {
    public:
    inline NoSuchService(std::string className) : Error("NoSuchService", "Unknown service type " + className) {}
};

class ServiceAlreadyExists : public Error {
    public:
    inline ServiceAlreadyExists(std::string className) : Error("ServiceAlreadyExists", "Service " + className + " is already inserted") {}
};

class MemberNotFound : public Error {
    public:
    inline MemberNotFound(std::string className, std::string memberName) : Error("MemberNotFound", "Could not find member '" + memberName + "' in class " + className) {}
};

class AssignToReadOnlyMember : public Error {
    public:
    inline AssignToReadOnlyMember(std::string className, std::string memberName) : Error("AssignToReadOnlyMember", "Attempt to assign value to read-only member '" + memberName + "' in class " + className) {}
};

class InstanceCastError : public Error {
    public:
    inline InstanceCastError(std::string sourceClass, std::string targetClass) : Error("InstanceCastError", "Attempt to cast object of type " + sourceClass + " to incompatible type " + targetClass) {}
};