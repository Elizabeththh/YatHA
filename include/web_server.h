#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "../third_party/cpp_httplib/httplib.h"
#include "../third_party/json.hpp"
#include "../include/ha_engine.h"
#include "../include/ha_engine_sse.h"
#include <string>
#include <memory>
#include <chrono>

using json = nlohmann::json;

class WebServer
{
    private:
        httplib::Server server;

        std::string generateTempFileName(const std::string &prefix);
        void saveTempFile(const std::string &filename, const std::string &content);
        std::string readTempFile(const std::string &filename);
        void cleanupTempFile(const std::string &filename);

        void handleAnalyze(const httplib::Request &req, httplib::Response &res);
        void handleAnalyzeFilter(const httplib::Request &req, httplib::Response &res);
        void handleAnalyzeChooser(const httplib::Request &req, httplib::Response &res);
        void handleStreamAnalyze(const httplib::Request &req, httplib::Response &res);

    public:
        WebServer();

        void setupRoutes();
        void start(const std::string &host = "localhost", int port = 8080);
};

#endif // WEB_SERVER_H
