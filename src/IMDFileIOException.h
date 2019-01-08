#ifndef IMD_IMDFILEIOEXCEPTION_H
#define IMD_IMDFILEIOEXCEPTION_H


#include <stdexcept>

namespace imd {

    class IMDFileIOException : public std::runtime_error {

        using std::runtime_error::runtime_error;

    };

}


#endif //IMD_IMDFILEIOEXCEPTION_H
