/*
 * Copyright (C) 2019 TU Dresden
 * All rights reserved.
 *
 * Authors:
 *   Christian Menard
 */

#include "reactor-cpp/reaction.hh"

#include "reactor-cpp/action.hh"
#include "reactor-cpp/assert.hh"
#include "reactor-cpp/environment.hh"
#include "reactor-cpp/port.hh"

#include <cassert>

namespace reactor {

Reaction::Reaction(const std::string& name,
                   int priority,
                   Reactor* container,
                   std::function<void(void)> body)
    : ReactorElement(name, ReactorElement::Type::Reaction, container)
    , _priority(priority)
    , body(body) {
  assert(priority != 0);
}

void Reaction::declare_trigger(BaseAction* action) {
  assert(action != nullptr);
  assert(this->environment() == action->environment());
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
           "Triggers may only be declared during assembly phase!");
  reactor::validate(this->container() == action->container(),
           "Action triggers must belong to the same reactor as the triggered "
           "reaction");

  assert(_action_triggers.insert(action).second);
  action->register_trigger(this);
}

void Reaction::declare_schedulable_action(BaseAction* action) {
  assert(action != nullptr);
  assert(this->environment() == action->environment());
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
           "Scheduable actions may only be declared during assembly phase!");
  reactor::validate(this->container() == action->container(),
           "Scheduable actions must belong to the same reactor as the "
           "triggered reaction");

  assert(_scheduable_actions.insert(action).second);
  action->register_scheduler(this);
}

void Reaction::declare_trigger(BasePort* port) {
  assert(port != nullptr);
  assert(this->environment() == port->environment());
  assert(this->environment()->phase() == Environment::Phase::Assembly);
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
        "Triggers may only be declared during assembly phase!");

  if (port->is_input()) {
    reactor::validate(
        this->container() == port->container(),
        "Input port triggers must belong to the same reactor as the triggered "
        "reaction");
  } else {
    reactor::validate(this->container() == port->container()->container(),
        "Output port triggers must belong to a contained reactor");
  }

  assert(_port_triggers.insert(port).second);
  assert(_dependencies.insert(port).second);
  port->register_dependency(this, true);
}

void Reaction::declare_dependency(BasePort* port) {
  assert(port != nullptr);
  assert(this->environment() == port->environment());
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
           "Dependencies may only be declared during assembly phase!");

  if (port->is_input()) {
    reactor::validate(this->container() == port->container(),
             "Dependent input ports must belong to the same reactor as the "
             "reaction");
  } else {
    reactor::validate(this->container() == port->container()->container(),
             "Dependent output ports must belong to a contained reactor");
  }

  assert(_dependencies.insert(port).second);
  port->register_dependency(this, false);
}

void Reaction::declare_antidependency(BasePort* port) {
  assert(port != nullptr);
  assert(this->environment() == port->environment());
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
           "Antidependencies may only be declared during assembly phase!");

  if (port->is_output()) {
    reactor::validate(this->container() == port->container(),
             "Antidependent output ports must belong to the same reactor as "
             "the reaction");
  } else {
    reactor::validate(this->container() == port->container()->container(),
             "Antidependent input ports must belong to a contained reactor");
  }

  assert(_antidependencies.insert(port).second);
  port->register_antidependency(this);
}

void Reaction::trigger() {
  if (has_deadline()) {
    assert(deadline_handler != nullptr);
    auto lag =
        container()->get_physical_time() - container()->get_logical_time();
    if (lag > deadline) {
      deadline_handler();
      return;
    }
  }

  body();
}

void Reaction::set_deadline_impl(Duration dl,
                                 std::function<void(void)> handler) {
  assert(!has_deadline());
  assert(handler != nullptr);
  this->deadline = dl;
  this->deadline_handler = handler;
}

void Reaction::set_index(unsigned index) {
  reactor::validate(this->environment()->phase() == Environment::Phase::Assembly,
           "Reaction indexes may only be set during assembly phase!");
  this->_index = index;
}

}  // namespace reactor
