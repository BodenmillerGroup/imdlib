#ifndef IMD_IMDFILEMALFORMEDEXCEPTION_H
#define IMD_IMDFILEMALFORMEDEXCEPTION_H


#include <stdexcept>

namespace imd {

    class IMDFileMalformedException : std::runtime_error {

        using std::runtime_error::runtime_error;

    };

}


#endif //IMD_IMDFILEMALFORMEDEXCEPTION_H
