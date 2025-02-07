/*
 * Copyright (C) 2019 TU Dresden
 * All rights reserved.
 *
 * Authors:
 *   Christian Menard
 */

#include "reactor-cpp/reactor.hh"

#include "reactor-cpp/action.hh"
#include "reactor-cpp/assert.hh"
#include "reactor-cpp/environment.hh"
#include "reactor-cpp/logging.hh"
#include "reactor-cpp/port.hh"
#include "reactor-cpp/reaction.hh"

#include <cassert>

namespace reactor {

ReactorElement::ReactorElement(const std::string& name,
                               ReactorElement::Type type,
                               Reactor* container)
    : _name(name), _container(container) {
  assert(container != nullptr);
  this->_environment = container->environment();
  assert(this->_environment != nullptr);
  reactor::validate(this->_environment->phase() == Environment::Phase::Construction,
           "Reactor elements can only be created during construction phase!");
  // We need a reinterpret_cast here as the derived class is not yet created
  // when this constructor is executed. dynamic_cast only works for
  // completely constructed objects. Technically, the casts here return
  // invalid pointers as the objects they point to do not yet
  // exists. However, we are good as long as we only store the pointer and do
  // not dereference it before construction is completeted.
  // It works, but maybe there is some nicer way of doing this...
  switch (type) {
    case Type::Action:
      container->register_action(reinterpret_cast<BaseAction*>(this));
      break;
    case Type::Port:
      container->register_port(reinterpret_cast<BasePort*>(this));
      break;
    case Type::Reaction:
      container->register_reaction(reinterpret_cast<Reaction*>(this));
      break;
    case Type::Reactor:
      container->register_reactor(reinterpret_cast<Reactor*>(this));
      break;
    default:
      throw std::runtime_error("unexpected type");
  }

  std::stringstream ss;
  ss << _container->fqn() << '.' << name;
  _fqn = ss.str();
}

ReactorElement::ReactorElement(const std::string& name,
                               ReactorElement::Type type,
                               Environment* environment)
    : _name(name), _fqn(name), _container(nullptr), _environment(environment) {
  assert(environment != nullptr);
  reactor::validate(type == Type::Reactor,
           "Only reactors can be owned by the environment!");
  reactor::validate(this->_environment->phase() == Environment::Phase::Construction,
           "Reactor elements can only be created during construction phase!");
}

Reactor::Reactor(const std::string& name, Reactor* container)
    : ReactorElement(name, ReactorElement::Type::Reactor, container) {}
Reactor::Reactor(const std::string& name, Environment* environment)
    : ReactorElement(name, ReactorElement::Type::Reactor, environment) {
  environment->register_reactor(this);
}

void Reactor::register_action(BaseAction* action) {
  UNUSED(action);
  assert(action != nullptr);
  reactor::validate(this->environment()->phase() == Environment::Phase::Construction,
           "Actions can only be registered during construction phase!");
  assert(_actions.insert(action).second);
}
void Reactor::register_port(BasePort* port) {
  assert(port != nullptr);
  reactor::validate(this->environment()->phase() == Environment::Phase::Construction,
           "Ports can only be registered during construction phase!");
  if (port->is_input()) {
    assert(_inputs.insert(port).second);
  } else {
    assert(_outputs.insert(port).second);
  }
}
void Reactor::register_reaction(Reaction* reaction) {
  UNUSED(reaction);
  assert(reaction != nullptr);
  reactor::validate(this->environment()->phase() == Environment::Phase::Construction,
           "Reactions can only be registered during construction phase!");
  assert(_reaction.insert(reaction).second);
}
void Reactor::register_reactor(Reactor* reactor) {
  UNUSED(reactor);
  assert(reactor != nullptr);
  reactor::validate(this->environment()->phase() == Environment::Phase::Construction,
           "Reactions can only be registered during construction phase!");
  assert(_reactors.insert(reactor).second);
}

void Reactor::startup() {
  assert(environment()->phase() == Environment::Phase::Startup);
  log::Debug() << "Starting up reactor " << fqn();
  // call startup on all contained objects
  for (auto x : _actions)
    x->startup();
  for (auto x : _inputs)
    x->startup();
  for (auto x : _outputs)
    x->startup();
  for (auto x : _reactions)
    x->startup();
  for (auto x : _reactors)
    x->startup();
}

void Reactor::shutdown() {
  assert(environment()->phase() == Environment::Phase::Shutdown);
  log::Debug() << "Terminating reactor " << fqn();
  // call shutdown on all contained objects
  for (auto x : _actions)
    x->shutdown();
  for (auto x : _inputs)
    x->shutdown();
  for (auto x : _outputs)
    x->shutdown();
  for (auto x : _reactions)
    x->shutdown();
  for (auto x : _reactors)
    x->shutdown();
}

TimePoint Reactor::get_physical_time() const {
  return ::reactor::get_physical_time();
}

TimePoint Reactor::get_logical_time() const {
  return environment()->scheduler()->logical_time().time_point();
}

Duration Reactor::get_elapsed_logical_time() const {
  return get_logical_time() - environment()->start_time();
}

Duration Reactor::get_elapsed_physical_time() const {
  return get_physical_time() - environment()->start_time();
}

}  // namespace reactor
