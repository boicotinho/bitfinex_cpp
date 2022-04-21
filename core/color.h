#pragma once

#include <iostream>
#include <cstdio>

struct term_attr { enum _ {
    NORMAL    = 0,
    BRIGHT    = 1,
    DIM       = 2,
    UNDERLINE = 4,
    BLINK     = 5,
    REVERSE   = 7,
    CROSSOUT  = 9
}; };

struct term_color { enum _ {
    BLACK    = 0,
    RED      = 1,
    GREEN    = 2,
    YELLOW   = 3,
    BLUE     = 4,
    MAGENTA  = 5,
    CYAN     = 6,
    WHITE    = 7,
    ORIGINAL = 9
}; };


class setcolor {
public:
    typedef int state;

    static bool& enabled() { static bool ee = true; return ee; }

    explicit setcolor( term_attr::_  attr = term_attr::NORMAL,
                       term_color::_ fg   = term_color::ORIGINAL,
                       term_color::_ bg   = term_color::ORIGINAL,
                       state*             = nullptr)
    {
        m_command_size = std::sprintf( m_control_command, "%c[%c;3%c;4%cm",
          0x1B,
          static_cast<char>(attr + '0'),
          static_cast<char>(fg + '0'),
          static_cast<char>(bg + '0'));
    }

    explicit setcolor(state* )
    {
        m_command_size = std::sprintf(m_control_command, "%c[%c;3%c;4%cm",
          0x1B,
          static_cast<char>(term_attr::NORMAL + '0'),
          static_cast<char>(term_color::ORIGINAL + '0'),
          static_cast<char>(term_color::ORIGINAL + '0'));
    }

    friend std::ostream& operator<<( std::ostream& os, setcolor const& sc )
    {
       if (sc.enabled())
          return os.write( sc.m_control_command, sc.m_command_size );
       return os;
    }

    const char* c_str() const {return m_control_command;}

private:
    char m_control_command[13];
    int  m_command_size;
};

// SGR: Select Graphic Rendition escape codes. \e is GNU-specific. Use \033

#define COL_RESET  "\033[0;39;49m"
#define COL_RED    "\033[1;31;40m"
#define COL_GREEN  "\033[1;32;40m"
#define COL_YELLOW "\033[1;33;40m"
#define COL_BLUE   "\033[1;34;40m"
#define COL_MAG    "\033[1;35;40m"
#define COL_CYAN   "\033[1;36;40m"
#define COL_WHITE  "\033[1;37;40m"


struct scope_setcolor {
  scope_setcolor()
  : m_os( 0 )
  , m_state()
  {}

  explicit    scope_setcolor(
    bool is_color_output,
    std::ostream& os,
    term_attr::_  attr = term_attr::NORMAL,
    term_color::_ fg   = term_color::ORIGINAL,
    term_color::_ bg   = term_color::ORIGINAL )
  : m_os( &os )
  {
    os << setcolor(attr, fg, bg, &m_state);
  }

  ~scope_setcolor()
  {
    if (m_os) {
      *m_os << setcolor(&m_state);
    }
  }
private:
  scope_setcolor(const scope_setcolor& r);
  scope_setcolor& operator=(const scope_setcolor& r);
private:
  std::ostream* m_os;
  setcolor::state m_state;
};
