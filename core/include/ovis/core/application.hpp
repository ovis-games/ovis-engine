#pragma once

#include "ovis/core/scheduler.hpp"
namespace ovis {

struct Nothing {};
using ApplicationSchedular = Scheduler<Nothing, double>;
using ApplicationJob = Job<Nothing, double>;

extern ApplicationSchedular application_scheduler;

void RunApplicationLoop();
void QuitApplicationLoop();

}  // namespace ovis
