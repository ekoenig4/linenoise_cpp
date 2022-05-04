#ifndef TERMINAL_OUTPUT_H
#define TERMINAL_OUTPUT_H

#include <iostream>
#include <sstream>

#include "config.h"

namespace print 
{
    struct Logger
    {
        virtual bool show() = 0;

        template <typename T>
        Logger &operator<<(const T &x)
        {
            if (!show())
                return *this;

            // format x as you please
            std::cout << x;
            return *this;
        }

        Logger &operator<<(std::ostream &(*f)(std::ostream &))
        {
            if (!show())
                return *this;

            f(std::cout);
            return *this;
        }

        Logger &operator<<(std::ostream &(*f)(std::ios &))
        {
            if (!show())
                return *this;

            f(std::cout);
            return *this;
        }

        Logger &operator<<(std::ostream &(*f)(std::ios_base &))
        {
            if (!show())
                return *this;

            f(std::cout);
            return *this;
        }
    };
    
    struct Output : Logger
    {
        int level = 1;
        bool show() { return config::verbose >= level; }
        Output& verbose(int level)
        {
            this->level = level;
            return *this;
        }
    };
    extern Output output;

    struct Debug : Logger
    {
        bool show() { return config::debug; }
    };
    extern Debug debug;
};

#endif // TERMINAL_OUTPUT_H