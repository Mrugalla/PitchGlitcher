#include "Events.h"

evt::System::Evt::Evt(System& _sys) :
    sys(_sys),
    notifier(nullptr)
{
}

evt::System::Evt::Evt(System& _sys, const Notify& _notifier) :
    sys(_sys),
    notifier(_notifier)
{
    sys.add(this);
}

evt::System::Evt::Evt(System& _sys, Notify&& _notifier) :
    sys(_sys),
    notifier(_notifier)
{
    sys.add(this);
}

evt::System::Evt::Evt(const Evt& other) :
    notifier(other.notifier),
    sys(other.sys)
{
    sys.add(this);
}

evt::System::Evt::~Evt()
{
    sys.remove(this);
}

void evt::System::Evt::operator()(const Type type, const void* stuff) const
{
    sys.notify(type, stuff);
}

evt::System::System() :
    evts()
{}

void evt::System::notify(const Type type, const void* stuff)
{
    for (const auto e : evts)
        e->notifier(type, stuff);
}

void evt::System::add(Evt* e)
{
    evts.push_back(e);
}

void evt::System::remove(const Evt* e)
{
    for (auto i = 0; i < evts.size(); ++i)
        if (e == evts[i])
        {
            evts.erase(evts.begin() + i);
            return;
        }
}