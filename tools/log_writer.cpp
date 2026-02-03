// testlogger.cpp
#define NOMINMAX
#include <windows.h>
#include <conio.h>
#include <cstdio>
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main() {
  const char* path = "C:\\temp\\test.log";

  HANDLE h = CreateFileA(
      path,
      FILE_APPEND_DATA,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

  if (h == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open " << path << " (err=" << GetLastError() << ")\n";
    return 1;
  }

  std::mt19937_64 rng{std::random_device{}()};
  std::uniform_int_distribution<int> space_dist(1, 3);
  std::uniform_int_distribution<int> id_dist(1000, 9999);
  std::uniform_int_distribution<int> pid_dist(2000, 9000);

  const std::vector<std::string> levels = {"INFO", "WARN", "ERROR", "DEBUG"};
  const std::vector<std::string> services = {"auth", "billing", "search", "api", "worker"};
  const std::vector<std::string> actions = {
      "request completed", "cache hit", "cache miss", "db query", "retry scheduled",
      "rate limit", "timeout", "job enqueued", "job started", "job finished"
  };

  auto pick = [&](const std::vector<std::string>& v) -> const std::string& {
    std::uniform_int_distribution<std::size_t> d(0, v.size() - 1);
    return v[d(rng)];
  };
  auto spaces = [&](int n) { return std::string(n, ' '); };

  int min_ms = 1;
  int max_ms = 50;
  int burst = 1;

  std::cout << "Writing to " << path << "\n";
  std::cout << "Press '+' to speed up, '-' to slow down. Ctrl+C to stop.\n";

  while (true) {
    while (_kbhit()) {
      int ch = _getch();
      if (ch == '+') {
        if (max_ms > min_ms) max_ms /= 2;
        if (burst < 1000) burst *= 2;
        std::cout << "Faster: delay [" << min_ms << "," << max_ms
                  << "] ms, burst=" << burst << "\n";
      } else if (ch == '-') {
        if (max_ms < 5000) max_ms *= 2;
        if (burst > 1) burst /= 2;
        std::cout << "Slower: delay [" << min_ms << "," << max_ms
                  << "] ms, burst=" << burst << "\n";
      }
    }

    std::uniform_int_distribution<int> sleep_ms(min_ms, max_ms);

    for (int b = 0; b < burst; ++b) {
      SYSTEMTIME st;
      GetLocalTime(&st);

      char ts[64];
      std::snprintf(
          ts, sizeof(ts),
          "%04d-%02d-%02d %02d:%02d:%02d.%03d",
          st.wYear, st.wMonth, st.wDay,
          st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

      int id = id_dist(rng);
      int pid = pid_dist(rng);

      std::string line =
          std::string(ts) + spaces(space_dist(rng)) +
          "[" + pick(levels) + "]" + spaces(space_dist(rng)) +
          "svc=" + pick(services) + spaces(space_dist(rng)) +
          "pid=" + std::to_string(pid) + spaces(space_dist(rng)) +
          "req=" + std::to_string(id) + spaces(space_dist(rng)) +
          pick(actions) + "\r\n";

      DWORD written = 0;
      BOOL ok = WriteFile(h, line.data(),
                          static_cast<DWORD>(line.size()),
                          &written, nullptr);
      if (!ok || written != line.size()) {
        std::cerr << "WriteFile failed (err=" << GetLastError() << ")\n";
        CloseHandle(h);
        return 1;
      }
    }

    Sleep(static_cast<DWORD>(sleep_ms(rng)));
  }
}
