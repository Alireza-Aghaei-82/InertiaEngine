#pragma once


#include "game-state-maintainer.hpp"


class StateWrapper
{
public:

    inline static StateWrapper& instance()
    {
        static StateWrapper instance;
        return instance;
    }

    StateWrapper(const StateWrapper& other) = delete;
    StateWrapper(StateWrapper&& other) = delete;
    StateWrapper& operator=(const StateWrapper& rhs) = delete;
    StateWrapper& operator=(StateWrapper&& rhs) = delete;

    inline GameStateMaintainer* state()
    {
        return m_stateMaintainer;
    }

    inline void setState(GameStateMaintainer* stateMaintainer)
    {
        m_stateMaintainer = stateMaintainer;
    }

private:

    StateWrapper() = default;

    GameStateMaintainer* m_stateMaintainer{};
};
