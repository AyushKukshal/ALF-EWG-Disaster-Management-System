// CIC-211 OOP Using C++ | Project: ALF-EWG (Autonomous Landslide & Flash-Flood Early Warning Grid)
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <exception>
#include <stdexcept>
#include <cstdio>

namespace DisasterCore {

    // ======================= UNIT I / II / III / IV : SENSORS =======================
    namespace Sensors {

        // ---------- Unit I: call by reference / return by reference ----------
        class GeoLocation {
        public:
            double latitude, longitude, elevation;
            GeoLocation(double lat = 0.0, double lon = 0.0, double elev = 0.0)
                : latitude(lat), longitude(lon), elevation(elev) {}
        };

        GeoLocation& updateCoordinates(GeoLocation& loc, double latOffset, double lonOffset) {
            loc.latitude += latOffset;
            loc.longitude += lonOffset;
            return loc;
        }

        double& accumulateTelemetry(double& runningTotal, double reading) {
            runningTotal += reading;
            return runningTotal;          // return by reference
        }

        // ---------- Unit III: diamond problem with virtual base classes ----------
        class DisasterTelemetryDevice {
        protected:
            int deviceID;
        public:
            DisasterTelemetryDevice(int id) : deviceID(id) {
                std::cout << "-> Base Device Powered ON. ID: " << deviceID << "\n";
            }
            virtual ~DisasterTelemetryDevice() {}
        };

        class SolarPoweredNode : virtual public DisasterTelemetryDevice {
        public:
            SolarPoweredNode(int id) : DisasterTelemetryDevice(id) {
                std::cout << "-> Solar Panels Deployed.\n";
            }
            void chargeBattery() { std::cout << "Battery charging from sun...\n"; }
        };

        class WirelessLoRaTransceiver : virtual public DisasterTelemetryDevice {
        public:
            WirelessLoRaTransceiver(int id) : DisasterTelemetryDevice(id) {
                std::cout << "-> LoRa Network Connected.\n";
            }
            void sendSignal() { std::cout << "Signal transmitted to Headquarter.\n"; }
        };

        class AutonomousHybridProbe : public SolarPoweredNode, public WirelessLoRaTransceiver {
        public:
            AutonomousHybridProbe(int id)
                : DisasterTelemetryDevice(id), SolarPoweredNode(id), WirelessLoRaTransceiver(id) {
                std::cout << "=> Autonomous Hybrid Probe is fully Operational!\n";
            }
        };

        // ---------- Unit III: containership + abstract base ----------
        class PowerSubsystem {
        public:
            int batteryLevel;
            PowerSubsystem() : batteryLevel(100) {}
            void displayPower() const { std::cout << "[Power: " << batteryLevel << "%] "; }
        };

        class BaseSensorNode {
        protected:
            PowerSubsystem powerUnit;     // containership (has-a)
            std::string nodeName;
        public:
            BaseSensorNode(std::string name) : nodeName(name) {}
            virtual void readTelemetry() = 0;
            virtual void evaluateThreshold() = 0;
            virtual ~BaseSensorNode() {}
        };

        // ---------- Unit IV: custom exceptions (defined early: sensors below throw them) ----------
        class SensorFailureException : public std::exception {
        private:
            std::string errorMessage;
        public:
            SensorFailureException(std::string msg) : errorMessage(msg) {}
            const char* what() const noexcept override { return errorMessage.c_str(); }
        };

        class TelemetryChecksumMismatch : public std::exception {
        private:
            std::string errorMessage;
        public:
            TelemetryChecksumMismatch(int expected, int actual)
                : errorMessage("CHECKSUM MISMATCH: expected " + std::to_string(expected) +
                               " but disk returned " + std::to_string(actual)) {}
            const char* what() const noexcept override { return errorMessage.c_str(); }
        };

        class ThresholdOverflowException : public std::exception {
        private:
            std::string errorMessage;
        public:
            ThresholdOverflowException(const std::string& sensor, double value, double limit)
                : errorMessage("OVERFLOW at " + sensor + ": reading " + std::to_string(value) +
                               " exceeds hard limit " + std::to_string(limit)) {}
            const char* what() const noexcept override { return errorMessage.c_str(); }
        };

        // ---------- Unit III: concrete sensors ----------
        class RiverSonarGauge : public BaseSensorNode {
        private:
            double waterLevel;
        public:
            RiverSonarGauge(std::string name) : BaseSensorNode(name), waterLevel(0.0) {}
            void readTelemetry() override {
                powerUnit.displayPower();
                waterLevel = 15.5;
                std::cout << nodeName << " -> Reading River Level: " << waterLevel << " meters\n";
            }
            void evaluateThreshold() override {
                if (waterLevel > 10.0)
                    std::cout << "  => ALERT! Flash Flood Warning at " << nodeName << "\n";
            }
        };

        class SoilMoistureProbe : public BaseSensorNode {
            double saturation;
        public:
            SoilMoistureProbe(std::string n) : BaseSensorNode(n), saturation(0.0) {}
            void readTelemetry() override {
                powerUnit.displayPower();
                saturation = 82.0;
                std::cout << nodeName << " -> Soil Saturation: " << saturation << "%\n";
            }
            void evaluateThreshold() override {
                if (saturation > 75.0)
                    std::cout << "  => ALERT! Landslide risk at " << nodeName << "\n";
            }
        };

        class SeismicGeophone : public BaseSensorNode {
            double vibration;
        public:
            SeismicGeophone(std::string n) : BaseSensorNode(n), vibration(0.0) {}
            void readTelemetry() override {
                powerUnit.displayPower();
                vibration = 4.2;
                std::cout << nodeName << " -> Ground Vibration: " << vibration << " Hz\n";
            }
            void evaluateThreshold() override {
                if (vibration > 3.0)
                    std::cout << "  => ALERT! Seismic activity at " << nodeName << "\n";
            }
        };

        class AcousticTiltmeter : public BaseSensorNode {
            double tilt;
        public:
            AcousticTiltmeter(std::string n) : BaseSensorNode(n), tilt(0.0) {}
            void readTelemetry() override {
                powerUnit.displayPower();
                tilt = 95.0;
                std::cout << nodeName << " -> Slope Tilt: " << tilt << " deg\n";
            }
            void evaluateThreshold() override {
                if (tilt > 90.0) throw ThresholdOverflowException(nodeName, tilt, 90.0);
            }
        };

        // ---------- Unit II: TelemetryBuffer (dynamic memory + operators) ----------
        class TelemetryBuffer {
        private:
            int* dataArray;
            int size;
            int hazardScore;
        public:
            TelemetryBuffer() : dataArray(nullptr), size(0), hazardScore(0) {}

            // parameterized / dynamic constructor
            TelemetryBuffer(int s, int score) : size(s), hazardScore(score) {
                dataArray = new int[size];
                for (int i = 0; i < size; i++) dataArray[i] = 1;
            }

            // deep copy constructor
            TelemetryBuffer(const TelemetryBuffer& source) {
                size = source.size;
                hazardScore = source.hazardScore;
                dataArray = new int[size];
                for (int i = 0; i < size; i++) dataArray[i] = source.dataArray[i];
            }

            // copy assignment (Rule of Three)
            TelemetryBuffer& operator=(const TelemetryBuffer& rhs) {
                if (this != &rhs) {
                    int* tmp = new int[rhs.size];
                    for (int i = 0; i < rhs.size; i++) tmp[i] = rhs.dataArray[i];
                    delete[] dataArray;
                    dataArray = tmp;
                    size = rhs.size;
                    hazardScore = rhs.hazardScore;
                }
                return *this;
            }

            ~TelemetryBuffer() { delete[] dataArray; }

            TelemetryBuffer operator+(const TelemetryBuffer& other) const {
                TelemetryBuffer merged(size + other.size, hazardScore + other.hazardScore);
                for (int i = 0; i < size; i++)       merged.dataArray[i] = dataArray[i];
                for (int i = 0; i < other.size; i++) merged.dataArray[size + i] = other.dataArray[i];
                return merged;
            }

            bool operator>(const TelemetryBuffer& other) const {
                return hazardScore > other.hazardScore;
            }

            friend std::ostream& operator<<(std::ostream& os, const TelemetryBuffer& buffer) {
                os << "[Buffer Capacity: " << buffer.size << " units | Hazard Score: "
                   << buffer.hazardScore << "]";
                return os;
            }

            friend std::istream& operator>>(std::istream& is, TelemetryBuffer& b) {
                int s, h;
                if (is >> s >> h && s > 0) {
                    int* tmp = new int[s];
                    for (int i = 0; i < s; i++) tmp[i] = 1;
                    delete[] b.dataArray;
                    b.dataArray = tmp;
                    b.size = s;
                    b.hazardScore = h;
                } else {
                    is.setstate(std::ios::failbit);
                }
                return is;
            }
        };

        // ---------- Unit II: EarlyWarningSiren + SensorStation (static members) ----------
        class EarlyWarningSiren {
        private:
            static int globalAlertLevel;
            std::string zone;
            bool active;
        public:
            EarlyWarningSiren(const std::string& z) : zone(z), active(false) {}
            void trigger() {
                active = true;
                if (globalAlertLevel < 5) ++globalAlertLevel;
                std::cout << "SIREN ACTIVE in " << zone << " | Global Alert Level: "
                          << globalAlertLevel << "\n";
            }
            bool isActive() const { return active; }
            const std::string& getZone() const { return zone; }
            static int getGlobalAlertLevel() { return globalAlertLevel; }
            static void resetGlobalAlertLevel() { globalAlertLevel = 0; }
        };
        int EarlyWarningSiren::globalAlertLevel = 0;

        class SensorStation {
        private:
            static int activeStations;
            std::string stationName;
            TelemetryBuffer buffer;
        public:
            SensorStation() : stationName("UNKNOWN"), buffer() { ++activeStations; }
            SensorStation(const std::string& name, int bufSize, int score)
                : stationName(name), buffer(bufSize, score) { ++activeStations; }
            SensorStation(const SensorStation& o)
                : stationName(o.stationName), buffer(o.buffer) { ++activeStations; }
            ~SensorStation() { --activeStations; }

            const std::string& getName() const { return stationName; }
            bool operator>(const SensorStation& o) const { return buffer > o.buffer; }
            static int getActiveStations() { return activeStations; }

            friend std::ostream& operator<<(std::ostream& os, const SensorStation& s) {
                return os << "Station[" << s.stationName << "] " << s.buffer;
            }
            friend std::istream& operator>>(std::istream& is, SensorStation& s) {
                return is >> s.stationName >> s.buffer;     // format: NAME SIZE SCORE
            }
        };
        int SensorStation::activeStations = 0;

        // ---------- Unit I: friend class ----------
        class SecureSensorData {
        private:
            double rawSeismicVoltage;
            bool isCalibrated;
        public:
            SecureSensorData(double voltage, bool calibrated)
                : rawSeismicVoltage(voltage), isCalibrated(calibrated) {}

            void runDiagnostic() {
                if (rawSeismicVoltage < 2.0)
                    throw SensorFailureException("CRITICAL: Seismic sensor voltage dropped below minimum operational threshold!");
                std::cout << "Diagnostic Passed. Sensor operating normally.\n";
            }
            friend class TelemetryAuditor;
        };

        class TelemetryAuditor {
        public:
            void inspectSensor(const SecureSensorData& sensor) {
                std::cout << "Auditor Access Bypassed... Raw Voltage is: " << sensor.rawSeismicVoltage << "V\n";
                if (!sensor.isCalibrated)
                    std::cout << "Auditor Alert: Sensor requires urgent calibration!\n";
            }
        };

        // ---------- Unit IV: persistence, functor, transactional logger ----------
        struct IncidentRecord {
            int hazardScore;
            char regionCode[20];
        };

        struct PriorityEvacuationRouter {
            bool operator()(const IncidentRecord& a, const IncidentRecord& b) const {
                return a.hazardScore > b.hazardScore;
            }
        };

        class DisasterLogger {
        public:
            void logIncidentsToDisk(std::vector<IncidentRecord>& incidents, bool injectFault = false) {
                const std::string filename = "disaster_logs.dat";
                { std::ofstream touch(filename, std::ios::app | std::ios::binary); }

                std::fstream logFile(filename, std::ios::in | std::ios::out | std::ios::binary);
                if (!logFile) throw std::runtime_error("Disk Storage Failure!");

                // BEGIN TRANSACTION: snapshot existing contents
                logFile.seekg(0, std::ios::end);
                std::streamoff oldSize = logFile.tellg();
                std::vector<char> backup(static_cast<size_t>(oldSize));
                logFile.seekg(0, std::ios::beg);
                if (oldSize > 0) logFile.read(backup.data(), oldSize);
                logFile.clear();

                std::sort(incidents.begin(), incidents.end(), PriorityEvacuationRouter());

                logFile.seekp(0, std::ios::end);
                std::streampos txnStart = logFile.tellp();
                int expectedChecksum = 0;

                std::for_each(incidents.begin(), incidents.end(), [&](const IncidentRecord& r) {
                    logFile.write(reinterpret_cast<const char*>(&r), sizeof(IncidentRecord));
                    expectedChecksum += r.hazardScore;
                });
                logFile.flush();

                // VERIFY: read back with seekg
                logFile.seekg(txnStart);
                int actualChecksum = 0;
                IncidentRecord tmp = {0, ""};
                for (size_t i = 0; i < incidents.size(); ++i) {
                    if (!logFile.read(reinterpret_cast<char*>(&tmp), sizeof(IncidentRecord))) break;
                    actualChecksum += tmp.hazardScore;
                }
                logFile.clear();
                if (injectFault) expectedChecksum += 1;   // simulate corruption for demo

                if (actualChecksum != expectedChecksum) {
                    // ROLLBACK: restore pre-transaction file
                    logFile.close();
                    std::fstream restore(filename, std::ios::out | std::ios::binary | std::ios::trunc);
                    if (oldSize > 0) restore.write(backup.data(), oldSize);
                    restore.close();
                    throw TelemetryChecksumMismatch(expectedChecksum, actualChecksum);
                }

                logFile.close();   // COMMIT
                std::cout << "Transaction committed: " << incidents.size()
                          << " incidents saved to " << filename << ".\n";
            }
        };
    } // namespace Sensors

    // ======================= ALERTS =======================
    namespace Alerts {

        inline double calculateBaseRisk(double baseFactor) { return baseFactor * 1.5; }

        inline double computeHazardIndex(double rainfallRate, double slopeAngle) {
            return calculateBaseRisk(rainfallRate) + (slopeAngle * 0.5);
        }

        inline double computeHazardIndex(double soilSaturation, int vibrationFreq) {
            return calculateBaseRisk(soilSaturation) + (vibrationFreq * 0.8);
        }

        // Unit IV: custom stream manipulators
        inline std::ostream& alertHeader(std::ostream& os) {          // no-arg manipulator
            return os << std::left << std::setw(18) << "NODE" << std::setw(12) << "LEVEL"
                      << std::setw(10) << "VALUE" << "\n" << std::string(40, '-') << "\n";
        }

        struct AlertRow {                                             // parameterized manipulator
            std::string node, level;
            double value;
            AlertRow(const std::string& n, const std::string& l, double v)
                : node(n), level(l), value(v) {}
        };
        inline std::ostream& operator<<(std::ostream& os, const AlertRow& r) {
            os << std::left << std::setw(18) << r.node << std::setw(12) << r.level
               << std::fixed << std::setprecision(2) << std::setw(10) << r.value << "\n";
            os.unsetf(std::ios::floatfield);
            return os;
        }
    }
}

int main() {
    using namespace DisasterCore::Sensors;

    std::cout << "--- ALF-EWG Phase 1: Core Foundations ---\n";
    GeoLocation nodeLocation(29.39, 78.18, 500.0);
    std::cout << "Original Location: Lat " << nodeLocation.latitude << ", Lon " << nodeLocation.longitude << "\n";
    updateCoordinates(nodeLocation, 0.05, -0.02);
    std::cout << "Updated Location: Lat " << nodeLocation.latitude << ", Lon " << nodeLocation.longitude << "\n";

    double riskFromRain = DisasterCore::Alerts::computeHazardIndex(12.5, 45.0);
    double riskFromEarthquake = DisasterCore::Alerts::computeHazardIndex(85.0, 120);
    std::cout << "\nHazard Index (Rain & Slope): " << riskFromRain << "\n";
    std::cout << "Hazard Index (Soil & Vibration): " << riskFromEarthquake << "\n";

    double total = 0.0;
    accumulateTelemetry(total, 4.2) += 1.0;      // modify through returned reference
    std::cout << "Accumulated telemetry: " << total << "\n";

    std::cout << "\n--- ALF-EWG Phase 2: Diamond Problem Resolution ---\n";
    AutonomousHybridProbe probe1(2725);
    probe1.chargeBattery();
    probe1.sendSignal();

    std::cout << "\n--- ALF-EWG Phase 3: Containership, Abstract Classes & Polymorphic Sweep ---\n";
    std::vector<BaseSensorNode*> grid;
    grid.push_back(new RiverSonarGauge("Ganga_Node_1"));
    grid.push_back(new SoilMoistureProbe("Soil_Node_2"));
    grid.push_back(new SeismicGeophone("Geo_Node_3"));
    grid.push_back(new AcousticTiltmeter("Tilt_Node_4"));

    for (int cycle = 1; cycle <= 2; ++cycle) {
        std::cout << "Sweep cycle " << cycle << ":\n";
        for (BaseSensorNode* node : grid) {          // runtime dispatch via vtable
            try {
                node->readTelemetry();
                node->evaluateThreshold();
            } catch (const std::exception& e) {
                std::cerr << "  EXCEPTION: " << e.what() << "\n";
            }
        }
    }
    for (BaseSensorNode* node : grid) delete node;   // virtual destructor
    grid.clear();

    std::cout << "\n--- ALF-EWG Phase 4: Memory & Operator Overloading ---\n";
    TelemetryBuffer zoneA(50, 45);
    TelemetryBuffer zoneB(30, 85);
    if (zoneB > zoneA)
        std::cout << "Zone B requires higher priority evacuation than Zone A!\n";

    TelemetryBuffer combinedRegion = zoneA + zoneB;
    std::cout << "Combined Regional Buffer State: " << combinedRegion << "\n";

    TelemetryBuffer inBuf;
    std::istringstream bufIn("20 60");
    bufIn >> inBuf;
    std::cout << "Extracted buffer: " << inBuf << "\n";

    SensorStation s1("Station_Alpha", 10, 40), s2("Station_Beta", 5, 70);
    {
        SensorStation copy(s1);
        std::cout << "Active stations (with copy): " << SensorStation::getActiveStations() << "\n";
    }
    std::cout << "Active stations (after scope): " << SensorStation::getActiveStations() << "\n";
    if (s2 > s1) std::cout << s2.getName() << " outranks " << s1.getName() << "\n";

    std::ofstream stationFile("stations.txt");
    stationFile << s1 << "\n" << s2 << "\n";          // file stream insertion
    stationFile.close();

    SensorStation loaded;
    std::istringstream stationIn("Station_Gamma 8 55");
    stationIn >> loaded;                               // stream extraction
    std::cout << loaded << "\n";

    EarlyWarningSiren siren("Haridwar_Zone");
    siren.trigger();
    std::cout << "Global alert level: " << EarlyWarningSiren::getGlobalAlertLevel() << "\n";

    std::cout << "\n--- ALF-EWG Phase 5: Friend Class & Exceptions ---\n";
    SecureSensorData nodeData(1.5, false);
    TelemetryAuditor auditor;
    auditor.inspectSensor(nodeData);

    try {
        std::cout << "Running automatic diagnostics...\n";
        nodeData.runDiagnostic();
    } catch (const SensorFailureException& e) {
        std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n";
    }

    try { throw ThresholdOverflowException("Demo_Node", 120.0, 100.0); }
    catch (const std::exception& e) { std::cerr << "EXCEPTION CAUGHT: " << e.what() << "\n"; }

    std::cout << "\n--- ALF-EWG Phase 6: STL & File Persistence ---\n";
    std::remove("disaster_logs.dat");                  // clean slate for each demo run

    std::vector<IncidentRecord> activeIncidents = {
        {45, "ZONE_ALPHA"},
        {120, "ZONE_BETA"},
        {80, "ZONE_GAMMA"}
    };

    DisasterLogger systemLogger;
    try {
        systemLogger.logIncidentsToDisk(activeIncidents);
    } catch (const std::exception& e) {
        std::cerr << "Logging Error: " << e.what() << "\n";
    }

    // std::map aggregation + std::for_each + custom manipulators
    std::map<std::string, int> zoneHazard;
    for (const auto& inc : activeIncidents) zoneHazard[inc.regionCode] += inc.hazardScore;

    std::cout << DisasterCore::Alerts::alertHeader;
    std::for_each(zoneHazard.begin(), zoneHazard.end(),
        [](const std::pair<const std::string, int>& kv) {
            std::cout << DisasterCore::Alerts::AlertRow(kv.first,
                           kv.second > 100 ? "CRITICAL" : "WARNING", kv.second);
        });

    // Demonstrate transactional rollback
    try { systemLogger.logIncidentsToDisk(activeIncidents, true); }
    catch (const TelemetryChecksumMismatch& e) {
        std::cerr << "ROLLED BACK: " << e.what() << "\n";
    }

    return 0;
}