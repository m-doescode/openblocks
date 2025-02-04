#include "service.h"
#include <memory>

Service::Service(std::weak_ptr<DataModel> root) : dataModel(root) {}