#include <iostream>
#include <fstream>
#include <regex>

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <zipkin/opentracing.h>

#include "text_map_carrier.h"

using namespace zipkin;
using namespace opentracing;

typedef std::unique_ptr<DIR, std::function<decltype(closedir)>> Dir;

int main (int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "No directory given!" << std::endl;

    return 1;
  }

  if (nullptr == getenv("ZIPKIN_HOST")) {
    std::cerr << "Please set ZIPKIN_HOST!" << std::endl;

    return 1;
  }

  std::string watch_directory {argv[1]};

  ZipkinOtTracerOptions options;
  options.collector_host = std::string {getenv("ZIPKIN_HOST")};
  options.service_name = "demo_arch";

  auto tracer = makeZipkinOtTracer(options);
  assert(tracer);

  std::regex file_format {"^([0-9]+)_([0-9a-f]+)_([0-9a-f]+)_([0-9a-f]+)\\.in$"};
  std::regex extension_change_format {"(\\.in)$"};

  while (true) {
    if (auto dir = Dir(opendir(watch_directory.c_str()), closedir)) {
      dirent *entry;

      while (nullptr != (entry = readdir(dir.get()))) {
        /* Type validation: files only. */
        if (DT_REG != entry->d_type) {
          continue;
        }

        /* Name validation: <FILE-ID>_<TRACE-ID>_<SPAN-ID>_<PARENT-ID>.in */
        std::string filename {entry->d_name};

        std::smatch matches;
        if (!std::regex_match(filename, matches, file_format)) {
          continue;
        }

        /* Zipkin trace data. */
        std::unordered_map<std::string, std::string> data;
        data.emplace("x-b3-traceid", matches[2].str());
        data.emplace("x-b3-spanid", matches[3].str());
        data.emplace("x-b3-parentspanid", matches[4].str());

        TextMapCarrier carrier {data};
        auto context = tracer->Extract(carrier);
        assert(context);

        /* "Task" */
        {
          auto task_span = tracer->StartSpan("spooler", {ChildOf(context->get())});
          assert(task_span);
          task_span->SetTag("span.kind", "server");
          task_span->SetTag("component", "calculation_spooler");
          task_span->SetTag("file.id", matches[1].str());

          auto new_filename = std::regex_replace(filename, extension_change_format, ".out");
          std::string new_filepath {watch_directory};
          new_filepath.append("/");
          new_filepath.append(new_filename);

          std::ofstream new_file {new_filepath};

          task_span->Log({{"event", "done"}});
          new_file.close();

          std::string old_path {watch_directory};
          old_path.append("/");
          old_path.append(filename);
          unlink(old_path.c_str());

          task_span->Finish();
        }
      }
    }
    usleep(1000);
  }

  tracer->Close();

  return 0;
}
