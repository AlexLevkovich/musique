#ifndef LIBMPVDEFS_H
#define LIBMPVDEFS_H

namespace Libmpv {

enum ErrorType {
    NoError = 0,
    NormalError = 1,
    FatalError = 2
};

enum State
{
    LoadingState,
    StoppedState,
    PlayingState,
    PausedState,
    ErrorState
};

}

#endif
