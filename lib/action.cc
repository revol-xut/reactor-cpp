/*
 * Copyright (C) 2019 TU Dresden
 * All rights reserved.
 *
 * Authors:
 *   Christian Menard
 */

#include "reactor-cpp/action.hh"

#include "reactor-cpp/assert.hh"
#include "reactor-cpp/environment.hh"
#include "reactor-cpp/reaction.hh"

#include <assert.h>

namespace reactor {

void BaseAction::register_trigger(Reaction* reaction) {
  assert(reaction != nullptr);
  assert(this->environment() == reaction->environment());
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
           "Triggers may only be registered during assembly phase!");
  reactor::validate(this->container() == reaction->container(),
           "Action triggers must belong to the same reactor as the triggered "
           "reaction");
  assert(_triggers.insert(reaction).second);
}

void BaseAction::register_scheduler(Reaction* reaction) {
  assert(reaction != nullptr);
  assert(this->environment() == reaction->environment());
  reactor::validate(is_logical(), "only logical action can be scheduled by a reaction!");
  reactor::validate(
      this->environment()->phase() == Environment::Phase::Assembly,
      "Schedulers for actions may only be registered during assembly phase!");
  // the reaction must belong to the same reactor as this action
  reactor::validate(this->container() == reaction->container(),
           "Scheduable actions must belong to the same reactor as the "
           "triggered reaction");
  //auto r = _schedulers.insert(reaction);
  assert(_schedulers.insert(reaction).second);
}

void Timer::startup() {
  Tag t0 = Tag::from_physical_time(environment()->start_time());
  if (_offset != Duration::zero()) {
    environment()->scheduler()->schedule_sync(t0.delay(_offset), this, nullptr);
  } else {
    environment()->scheduler()->schedule_sync(t0, this, nullptr);
  }
}

void Timer::cleanup() {
  // schedule the timer again
  if (_period != Duration::zero()) {
    Tag now = Tag::from_logical_time(environment()->logical_time());
    Tag next = now.delay(_period);
    environment()->scheduler()->schedule_sync(next, this, nullptr);
  }
}

void ShutdownAction::shutdown() {
  Tag t = Tag::from_logical_time(environment()->logical_time()).delay();
  environment()->scheduler()->schedule_sync(t, this, nullptr);
}

}  // namespace reactor
