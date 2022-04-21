#include "time_util.h"
#include "string_utils.h"
#include <ostream>

namespace std
{
    std::ostream& operator << (std::ostream& out, const std::chrono::nanoseconds      & tt) { return out << comma_num(tt.count()) << " ns"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::microseconds     & tt) { return out << comma_num(tt.count()) << " us"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::milliseconds     & tt) { return out << comma_num(tt.count()) << " ms"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::seconds          & tt) { return out << comma_num(tt.count()) << " s"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::minutes          & tt) { return out << comma_num(tt.count()) << " minutes"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::hours            & tt) { return out << comma_num(tt.count()) << " hours"; }
    std::ostream& operator << (std::ostream& out, const std::chrono::duration<double> & tt) { return out << tt.count() << " s"; }
}
