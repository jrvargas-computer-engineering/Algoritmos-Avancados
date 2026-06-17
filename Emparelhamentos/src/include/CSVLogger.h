#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <ostream>
#include <string>

struct CSVLogger {
    // Enforces the universal 9-column layout header
    static void write_header(std::ostream& out) {
        out << "test_id,n,alpha,rep,algorithm,iteration,weight_regime,metric_name,metric_value\n";
    }

    // Stateless metric injection guaranteeing exact schema alignment
    static void log_metric(std::ostream& out,
                           const std::string& test_id,
                           int n,
                           double alpha,
                           int rep,
                           const std::string& algorithm,
                           int iteration,
                           const std::string& weight_regime,
                           const std::string& metric_name,
                           double metric_value) {
        out << test_id << ","
            << n << ","
            << alpha << ","
            << rep << ","
            << algorithm << ","
            << iteration << ","
            << weight_regime << ","
            << metric_name << ","
            << metric_value << "\n";
    }
};

#endif // CSV_LOGGER_H