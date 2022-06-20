#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

/**
 * TODO
 * 1. Improve arg parser
 *    - regex to parse time (?)
 * 2. Loops
 *    - while(): use atomic bool to pause
 * 3. Mecanism
 *    - add a real pause() function
 *    - save times in vector
 * 4. Display
 *    - check terminal width
 */

template<typename Period>
using duration = std::chrono::duration<long int, Period>;

using namespace std::chrono_literals;


#ifdef __linux__
#include <termios.h>
int getch()
{
  termios oldattr, newattr;
  tcgetattr( /*STDIN_FILENO*/0, &oldattr );
  newattr = oldattr;
  newattr.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( /*STDIN_FILENO*/0, TCSANOW, &newattr );
  int ch = getchar();
  tcsetattr( /*STDIN_FILENO*/0, TCSANOW, &oldattr );
  return ch;
}
#elif defined _WIN32
#include <conio.h>
#else
#error Platform not supported
#endif

template<class Period>
void display_time(const duration<Period>& ms)
{
  const auto d = std::chrono::duration_cast<std::chrono::days>(ms);
  const auto h = std::chrono::duration_cast<std::chrono::hours>(ms);
  const auto min = std::chrono::duration_cast<std::chrono::minutes>(ms);
  const auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);

  auto display = [&](int time, const char *rep){
    if (time == 0)
      return std::string{};
    else
      return std::to_string(time).append(rep);
  };

  const auto d_mod = d.count();
  const auto h_mod = h.count()%24;
  const auto m_mod = min.count()%60;
  const auto s_mod = s.count()%60;

  std::cout << "\x1b[J" // ANSI erase line
	    << display(d_mod, " d ")
	    << display(h_mod, " h ")
	    << display(m_mod, " min ")
	    << display(s_mod, " s ")
	    << std::flush << '\r';
}

template<class Period>
void display_bar(const duration<Period>& ms)
{
  const auto d = std::chrono::duration_cast<std::chrono::days>(ms);
  const auto h = std::chrono::duration_cast<std::chrono::hours>(ms);
  const auto min = std::chrono::duration_cast<std::chrono::minutes>(ms);
  const auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);

  auto display = [&](const char* color, int time, const char *rep){
    if (time == 0)
      return std::string{};
    else
      return std::string(color).append(std::to_string(time)).append(rep).append(time, ' ');
  };

  const auto d_mod = d.count();
  const auto h_mod = h.count()%24;
  const auto m_mod = min.count()%60;
  const auto s_mod = s.count()%60;

  std::cout << "\x1b[J\x1b[1m"
	    << display("\x1b[45m ", d_mod, " d ")
	    << display("\x1b[43m ", h_mod, " h ")
	    << display("\x1b[42m ", m_mod, " min ")
	    << display("\x1b[44m ", s_mod, " s ")
	    << "\x1b[0m"
	    << std::flush << '\r';
}

template<typename Period>
void countdown(const duration<Period>& chrono_duration, const bool bar=false)
{
  auto and_now = [](){return std::chrono::high_resolution_clock::now();};
  const auto start_time = and_now();
  // To display the whole duration
  const auto end = start_time + chrono_duration + 1s;

  for (auto now = start_time; now < end-1s; now = and_now()) {
    const auto timer_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - now);
    if(bar == true)
      display_bar(timer_duration);
    else
      display_time(timer_duration);
    std::this_thread::sleep_until(now+100ms);
  }
  // Safe exit program if countdown over
  exit(0);
}


void timer(bool bar=false)
{
  auto and_now = [](){return std::chrono::high_resolution_clock::now();};
  const auto start_time = and_now();
  // TODO add atomic bool to pause
  while(true) {
    auto now = and_now();
    const auto timer_duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
    if(bar)
      display_bar(timer_duration);
    else
      display_time(timer_duration);
    std::this_thread::sleep_until(now+100ms);
  }
}

void keyboard_handler()
{
  // TODO
  while(true) {
    auto to_int = [](char c){return static_cast<int>(c);};
    int ch = getch();
    switch(ch){
    default:
      break; // do nothing: this key isn't used
    case to_int('\n'):
    case to_int('p'):
      std::puts("");
      break;
    case to_int('q'):
    case 27:// ESC KEY
      std::puts("");
      exit(0);
    }
  }
}

void help()
{
  std::cout << R"!(Usage: timer [OPTION]
To quit the timer press 'q' or ESC, press 'p' to save a time
with [OPTION]
-h show help
-c [DURATION] use countdown with duration
-t use timer (quit with Ctrl+C or 'q')
-b display bar (only work with ANSI)
)!";
}

// TODO make it work with format 1h19min3s
int main(int argc, char *argv[])
{
  // Each timing function is launched in a thread
  // Another thread is used to check inputs
  bool display_countdown = false;
  bool display_timer = false;
  bool use_bars = false;
  int read_duration = 0;
  std::vector<std::thread> vthread;
  for(int i = 0; i != argc; ++i) {
    auto arg_cmp = [](const char* arg, const char* o) {
      return (std::string(arg).compare(o) == 0);
    };
    if (arg_cmp(argv[i], "-c")) {
      read_duration = std::stoi(argv[i+1]);
      display_countdown = true;
    } else if (arg_cmp(argv[i], "-t")) {
      //      vthread.emplace_back(timer, true);
      display_timer = true;
    } else if(arg_cmp(argv[i], "-b")) {
      use_bars = true;
    }
  }

  if (display_timer) {
    vthread.emplace_back(timer, use_bars);
  } else if (display_countdown) {
    std::chrono::seconds sec{1};
    auto countdown_impl=[&read_duration, &use_bars](){
      countdown(std::chrono::seconds(read_duration), use_bars);};
    vthread.emplace_back(std::thread(countdown_impl));
  } else {
    help();
    return 0;
  }
  vthread.emplace_back(std::thread(keyboard_handler));
  for(auto &t:vthread) {
    t.join();
  }
  return 0;
}
