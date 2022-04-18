#include "time_util.h"
#include "string_utils.h"
#include <ostream>

namespace std
{
    std::ostream& operator << (std::ostream& out, const std::chrono::nanoseconds      & tt) { return out << CommaNum(tt.count()) << " ns"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::microseconds     & tt) { return out << CommaNum(tt.count()) << " us"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::milliseconds     & tt) { return out << CommaNum(tt.count()) << " ms"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::seconds          & tt) { return out << CommaNum(tt.count()) << " s"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::minutes          & tt) { return out << CommaNum(tt.count()) << " minutes"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::hours            & tt) { return out << CommaNum(tt.count()) << " hours"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::duration<double> & tt) { return out << tt.count() << " s"; }
}
